// Design C: AutoCloneable, a clone() mixin with no hooks. Reflection lists the concrete classes of the namespace and
// copies with the one that matches the dynamic type of the source. The classes write nothing for cloning.
#include <gtest/gtest.h>

#include <clone26/figure_reflected.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "support/test_values.hpp"

namespace elsewhere {

// Derived from Figure, but not in the namespace that clone() searches.
struct Stray : shapes_reflected::Figure {
  double area() const override { return 0; }
};

}  // namespace elsewhere

namespace {

using namespace clone26::testing_values;
using shapes_reflected::Circle;
using shapes_reflected::Figure;
using shapes_reflected::Square;
using shapes_reflected::Tile;

TEST(ReflectedMixin, clone_through_a_Figure_reference_keeps_the_dynamic_type) {
  Tile tile{kSide, kGrout};
  Figure& through_base = tile;
  auto copy = through_base.clone();

  EXPECT_TRUE((std::is_same_v<std::unique_ptr<Figure>, decltype(copy)>));
  ASSERT_NE(copy, nullptr);
  const Figure& cloned = *copy;
  EXPECT_TRUE(typeid(cloned) == typeid(Tile)) << typeid(cloned).name();
  EXPECT_DOUBLE_EQ(copy->area(), tile.area());
}

TEST(ReflectedMixin, every_class_of_the_namespace_keeps_its_dynamic_type) {
  Square square{kSide};
  Circle circle{kRadius};
  Figure& square_base = square;
  Figure& circle_base = circle;
  auto square_copy = square_base.clone();
  auto circle_copy = circle_base.clone();

  ASSERT_NE(square_copy, nullptr);
  ASSERT_NE(circle_copy, nullptr);
  const Figure& square_cloned = *square_copy;
  const Figure& circle_cloned = *circle_copy;
  EXPECT_TRUE(typeid(square_cloned) == typeid(Square)) << typeid(square_cloned).name();
  EXPECT_TRUE(typeid(circle_cloned) == typeid(Circle)) << typeid(circle_cloned).name();
}

TEST(ReflectedMixin, tile_clone_called_directly_returns_unique_ptr_of_Tile) {
  Tile tile{kSide, kGrout};
  auto copy = tile.clone();

  EXPECT_TRUE((std::is_same_v<std::unique_ptr<Tile>, decltype(copy)>));
  ASSERT_NE(copy, nullptr);
  EXPECT_EQ(copy->grout, kGrout);
  EXPECT_NE(copy.get(), &tile);
}

TEST(ReflectedMixin, clone_of_a_const_object_copies) {
  const Tile frozen{kSide, kGrout};
  auto copy = frozen.clone();

  EXPECT_TRUE((std::is_same_v<std::unique_ptr<Tile>, decltype(copy)>));
  ASSERT_NE(copy, nullptr);
  EXPECT_EQ(copy->history, frozen.history);
  EXPECT_NE(copy->history.data(), frozen.history.data());
}

TEST(ReflectedMixin, clone_of_an_rvalue_moves_the_state_into_the_new_object) {
  Tile donor{kSide, kGrout};
  const double* donor_buffer = donor.history.data();
  auto moved = std::move(donor).clone();

  EXPECT_TRUE((std::is_same_v<std::unique_ptr<Tile>, decltype(moved)>));
  ASSERT_NE(moved, nullptr);
  EXPECT_EQ(moved->grout, kGrout);
  // The moved-to vector owns the buffer the donor had: a copy would have allocated a new one.
  EXPECT_EQ(moved->history.data(), donor_buffer);
}

TEST(ReflectedMixin, clone_of_a_const_rvalue_copies_and_keeps_the_source) {
  const Tile frozen{kSide, kGrout};
  auto copy = std::move(frozen).clone();

  ASSERT_NE(copy, nullptr);
  EXPECT_NE(copy->history.data(), frozen.history.data());
  EXPECT_EQ(frozen.history, std::vector<double>{kSide});
}

// A forward declaration (Pending) and an alias (TileAlias) are members of shapes_reflected, so every test above runs
// with them present. These tests state what they must not change.

TEST(ReflectedMixin, an_alias_names_the_same_class_and_adds_no_class) {
  static_assert(std::is_same_v<shapes_reflected::TileAlias, Tile>);
  shapes_reflected::TileAlias tile{kSide, kGrout};
  auto copy = tile.clone();

  EXPECT_TRUE((std::is_same_v<std::unique_ptr<Tile>, decltype(copy)>));
  EXPECT_EQ(copy->grout, kGrout);
}

TEST(ReflectedMixin, clone_of_an_rvalue_moves_once_although_an_alias_names_the_class_twice) {
  Tile donor{kSide, kGrout};
  const double* donor_buffer = donor.history.data();
  auto moved = std::move(donor).clone();

  ASSERT_NE(moved, nullptr);
  // A second match would move from the donor again, after the first move emptied it: the new object would end up
  // with an empty history.
  EXPECT_EQ(moved->history.size(), 1u);
  EXPECT_EQ(moved->history.data(), donor_buffer);
  EXPECT_EQ(moved->history, std::vector<double>{kSide});
}

TEST(ReflectedMixin, clone_of_an_rvalue_through_the_alias_moves_once_too) {
  shapes_reflected::TileAlias donor{kSide, kGrout};
  const double* donor_buffer = donor.history.data();
  auto moved = std::move(donor).clone();

  ASSERT_NE(moved, nullptr);
  EXPECT_EQ(moved->history, std::vector<double>{kSide});
  EXPECT_EQ(moved->history.data(), donor_buffer);
}

TEST(ReflectedMixin, a_forward_declared_class_in_the_namespace_is_ignored) {
  Tile tile{kSide, kGrout};
  Figure& tile_base = tile;

  EXPECT_NO_THROW(tile_base.clone());
}

// A class in another namespace is the supported way to extend the hierarchy: its own namespace is searched for it,
// so it is cloned through its own type. Through a base reference of shapes_reflected it is not found, and it throws.
TEST(ReflectedMixin, a_class_in_another_namespace_clones_through_its_own_type) {
  elsewhere::Stray stray;
  auto copy = stray.clone();

  EXPECT_TRUE((std::is_same_v<std::unique_ptr<elsewhere::Stray>, decltype(copy)>));
  ASSERT_NE(copy, nullptr);
  EXPECT_NE(copy.get(), &stray);
}

TEST(ReflectedMixin, a_class_in_another_namespace_throws_through_a_base_reference) {
  elsewhere::Stray stray;
  Figure& stray_base = stray;

  EXPECT_THROW(stray_base.clone(), clone26::clone_unavailable);
  EXPECT_THROW(stray_base.clone(), std::logic_error);
}

TEST(ReflectedMixin, the_message_of_clone_unavailable_names_the_dynamic_type) {
  elsewhere::Stray stray;
  Figure& stray_base = stray;

  try {
    [[maybe_unused]] auto copy = stray_base.clone();
    ADD_FAILURE() << "clone() of a class outside the searched namespace did not throw";
  } catch (const clone26::clone_unavailable& error) {
    EXPECT_NE(std::string{error.what()}.find("Stray"), std::string::npos) << error.what();
  }
}

TEST(ReflectedMixin, a_class_of_the_searched_namespace_does_not_throw_through_a_base_reference) {
  Tile tile{kSide, kGrout};
  Figure& tile_base = tile;

  EXPECT_NO_THROW(tile_base.clone());
}

}  // namespace
