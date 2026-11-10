// The 2014 tests (FigureTests-v5.cpp), ported to design A: clone() with deducing this. One test per old test,
// then the cases the 2014 helper did not have: const objects, moving from an rvalue, Circle and Tile, and the
// assignment rules.
#include <gtest/gtest.h>

#include <clone26/figure_clone.hpp>
#include <cstddef>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

#include "support/test_values.hpp"

namespace {

using namespace clone26::testing_values;
using clone26::Cloneable;
using shapes::Circle;
using shapes::Figure;
using shapes::Square;
using shapes::Tile;

// The old tests, one each.

TEST(DeducingThisClone, square_clone_called_directly_returns_unique_ptr_of_Square) {
  Square square{kSide};
  auto figure = square.clone();

  EXPECT_TRUE((std::is_same_v<std::unique_ptr<Square>, decltype(figure)>));
  EXPECT_NE(figure.get(), &square);
}

TEST(DeducingThisClone, square_clone_called_via_base_reference_returns_unique_ptr_of_Figure) {
  Square square{kSide};
  Figure& square_figure = square;
  auto figure = square_figure.clone();

  EXPECT_TRUE((std::is_same_v<std::unique_ptr<Figure>, decltype(figure)>));
  const Figure& cloned = *figure;
  EXPECT_TRUE(typeid(cloned) == typeid(Square)) << typeid(cloned).name();
}

TEST(DeducingThisClone, cloned_square_via_base_reference_returns_same_area) {
  Square square{kSide};
  Figure& square_figure = square;
  auto figure = square_figure.clone();

  EXPECT_DOUBLE_EQ(square.area(), kSide * kSide);
  EXPECT_DOUBLE_EQ(figure->area(), square.area());
}

TEST(DeducingThisClone, cloning_rvalue_returns_unique_ptr_of_Square) {
  auto figure = Square{kSide}.clone();

  EXPECT_TRUE((std::is_same_v<std::unique_ptr<Square>, decltype(figure)>));
  EXPECT_DOUBLE_EQ(figure->area(), kSide * kSide);
}

// Moving from an rvalue. The answer to the expert question at the end of the 2014 article is yes.

TEST(DeducingThisClone, cloning_an_rvalue_moves_the_state_into_the_new_object) {
  Square donor{kSide};
  const double* donor_buffer = donor.history.data();
  auto moved = std::move(donor).clone();

  EXPECT_TRUE((std::is_same_v<std::unique_ptr<Square>, decltype(moved)>));
  EXPECT_DOUBLE_EQ(moved->side, kSide);
  EXPECT_EQ(moved->history, std::vector<double>{kSide});
  // The moved-to vector owns the buffer the donor had: pointers into a vector stay valid across a move
  // construction, and a copy would have allocated a new buffer.
  EXPECT_EQ(moved->history.data(), donor_buffer);
}

TEST(DeducingThisClone, cloning_an_lvalue_leaves_the_source_unchanged) {
  Square source{kSide};
  auto copy = source.clone();

  EXPECT_EQ(copy->history, source.history);
  EXPECT_NE(copy->history.data(), source.history.data());
  EXPECT_EQ(source.history, std::vector<double>{kSide});
}

// Const objects are copied, never moved.

TEST(DeducingThisClone, clone_of_a_const_object_returns_unique_ptr_of_Square_and_copies) {
  const Square frozen{kSide};
  auto copy = frozen.clone();

  EXPECT_TRUE((std::is_same_v<std::unique_ptr<Square>, decltype(copy)>));
  EXPECT_EQ(copy->history, frozen.history);
  EXPECT_NE(copy.get(), &frozen);
}

TEST(DeducingThisClone, clone_of_a_const_rvalue_copies_and_keeps_the_source) {
  const Square frozen{kSide};
  auto copy = std::move(frozen).clone();

  EXPECT_TRUE((std::is_same_v<std::unique_ptr<Square>, decltype(copy)>));
  EXPECT_EQ(copy->history, std::vector<double>{kSide});
  EXPECT_NE(copy->history.data(), frozen.history.data());
  EXPECT_EQ(frozen.history, std::vector<double>{kSide});
}

// The other classes of the hierarchy.

TEST(DeducingThisClone, circle_clone_via_base_reference_keeps_the_dynamic_type_and_the_area) {
  Circle circle{kRadius};
  Figure& circle_figure = circle;
  auto figure = circle_figure.clone();

  const Figure& cloned = *figure;
  EXPECT_TRUE(typeid(cloned) == typeid(Circle)) << typeid(cloned).name();
  EXPECT_DOUBLE_EQ(figure->area(), circle.area());
}

TEST(DeducingThisClone, tile_clone_called_directly_returns_unique_ptr_of_Tile) {
  Tile tile{kSide, kGrout};
  auto copy = tile.clone();

  EXPECT_TRUE((std::is_same_v<std::unique_ptr<Tile>, decltype(copy)>));
  EXPECT_EQ(copy->grout, kGrout);
  EXPECT_DOUBLE_EQ(copy->side, kSide);
}

TEST(DeducingThisClone, tile_clone_via_Square_and_Figure_references_keeps_the_dynamic_type) {
  Tile tile{kSide, kGrout};
  Square& as_square = tile;
  Figure& as_figure = tile;
  auto from_square = as_square.clone();
  auto from_figure = as_figure.clone();

  EXPECT_TRUE((std::is_same_v<std::unique_ptr<Square>, decltype(from_square)>));
  EXPECT_TRUE((std::is_same_v<std::unique_ptr<Figure>, decltype(from_figure)>));
  const Square& square_clone = *from_square;
  const Figure& figure_clone = *from_figure;
  EXPECT_TRUE(typeid(square_clone) == typeid(Tile)) << typeid(square_clone).name();
  EXPECT_TRUE(typeid(figure_clone) == typeid(Tile)) << typeid(figure_clone).name();
}

TEST(DeducingThisClone, tile_clone_of_an_rvalue_moves_through_the_Tile_hook) {
  auto moved = Tile{kSide, kGrout}.clone();

  EXPECT_TRUE((std::is_same_v<std::unique_ptr<Tile>, decltype(moved)>));
  EXPECT_EQ(moved->grout, kGrout);
  EXPECT_EQ(moved->history, std::vector<double>{kSide});
}

// Assignment: the copy and move assignment of the bases are protected, a derived class stays assignable, and
// assignment through a Figure& does not compile. The type traits ask the question without a failing build, so a
// static_assert states each answer.

TEST(DeducingThisClone, a_derived_class_stays_assignable) {
  static_assert(std::is_copy_assignable_v<Square>);
  static_assert(std::is_move_assignable_v<Square>);

  Square left{kOtherSide};
  const Square right{kSide};
  left = right;
  EXPECT_DOUBLE_EQ(left.area(), kSide * kSide);

  std::vector<Square> row(kRowSize);
  row.erase(row.begin());  // erase assigns the elements down: it needs Square::operator=(Square&&)
  EXPECT_EQ(row.size(), kRowSize - 1);
}

TEST(DeducingThisClone, assignment_through_a_Figure_or_Cloneable_reference_does_not_compile) {
  static_assert(!std::is_assignable_v<Figure&, const Figure&>);
  static_assert(!std::is_assignable_v<Figure&, Figure&&>);
  static_assert(!std::is_assignable_v<Figure&, const Square&>);
  static_assert(!std::is_assignable_v<Cloneable&, const Cloneable&>);
  static_assert(!std::is_assignable_v<Cloneable&, const Square&>);
  SUCCEED();
}

TEST(DeducingThisClone, Figure_and_Cloneable_cannot_be_copied_or_moved_by_a_user_of_the_base) {
  static_assert(!std::is_copy_constructible_v<Figure>);
  static_assert(!std::is_move_constructible_v<Figure>);
  static_assert(!std::is_copy_constructible_v<Cloneable>);
  static_assert(std::is_copy_constructible_v<Square>);
  SUCCEED();
}

// Cloneable has no template parameters, so any class can use it, not only a Figure.

class Note : public Cloneable {
 public:
  Note() = default;
  explicit Note(std::string t) : text{std::move(t)} {}

  std::string text;

 private:
  std::unique_ptr<Cloneable> copy_() const override { return std::make_unique<Note>(*this); }
  std::unique_ptr<Cloneable> move_() override { return std::make_unique<Note>(std::move(*this)); }
};

TEST(DeducingThisClone, a_class_that_is_not_a_Figure_can_use_the_mixin) {
  const Note note{"grout"};
  auto copy = note.clone();

  EXPECT_TRUE((std::is_same_v<std::unique_ptr<Note>, decltype(copy)>));
  EXPECT_EQ(copy->text, note.text);
}

TEST(DeducingThisClone, clone_through_a_Cloneable_reference_returns_unique_ptr_of_Cloneable) {
  Square square{kSide};
  Cloneable& base = square;
  auto copy = base.clone();

  EXPECT_TRUE((std::is_same_v<std::unique_ptr<Cloneable>, decltype(copy)>));
  const Cloneable& cloned = *copy;
  EXPECT_TRUE(typeid(cloned) == typeid(Square)) << typeid(cloned).name();
}

}  // namespace
