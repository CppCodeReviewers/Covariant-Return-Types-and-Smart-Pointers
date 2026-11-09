// Design B: std::polymorphic<Figure>. The classes have no clone(), no hooks and no override to forget, and the
// wrapper copies the dynamic type of what it holds.
#include <gtest/gtest.h>

#include <clone26/figure_polymorphic.hpp>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

#include "support/test_values.hpp"

namespace {

using namespace clone26::testing_values;
using shapes_polymorphic::Circle;
using shapes_polymorphic::Figure;
using shapes_polymorphic::Square;
using shapes_polymorphic::Tile;

const Figure& held(const std::polymorphic<Figure>& wrapper) { return *wrapper; }

std::vector<std::polymorphic<Figure>> make_row() {
  std::vector<std::polymorphic<Figure>> row;
  row.emplace_back(std::in_place_type<Square>, kSide);
  row.emplace_back(std::in_place_type<Circle>, kRadius);
  row.emplace_back(std::in_place_type<Tile>, kSide, kGrout);
  return row;
}

TEST(PolymorphicFigure, copies_keep_the_dynamic_type) {
  const auto row = make_row();
  const auto copy = row;

  ASSERT_EQ(copy.size(), row.size());
  for (std::size_t i = 0; i < row.size(); ++i) {
    EXPECT_TRUE(typeid(held(copy[i])) == typeid(held(row[i]))) << typeid(held(copy[i])).name();
  }
}

TEST(PolymorphicFigure, a_copy_is_deep_and_the_objects_have_distinct_addresses) {
  const auto row = make_row();
  const auto copy = row;

  ASSERT_EQ(copy.size(), row.size());
  for (std::size_t i = 0; i < row.size(); ++i) {
    EXPECT_NE(&held(copy[i]), &held(row[i]));
    EXPECT_DOUBLE_EQ(held(copy[i]).area(), held(row[i]).area());
  }
}

TEST(PolymorphicFigure, a_copy_owns_its_state) {
  std::polymorphic<Figure> original{std::in_place_type<Square>, kSide};
  const auto copy = original;
  dynamic_cast<Square&>(*original).history.clear();

  EXPECT_EQ(dynamic_cast<const Square&>(*copy).history, std::vector<double>{kSide});
}

TEST(PolymorphicFigure, a_Tile_without_any_hook_copies_as_a_Tile) {
  const std::polymorphic<Figure> original{std::in_place_type<Tile>, kSide, kGrout};
  const auto copy = original;

  ASSERT_TRUE(typeid(held(copy)) == typeid(Tile)) << typeid(held(copy)).name();
  EXPECT_EQ(dynamic_cast<const Tile&>(*copy).grout, kGrout);
}

TEST(PolymorphicFigure, copy_assignment_takes_the_dynamic_type_of_the_source) {
  std::polymorphic<Figure> target{std::in_place_type<Square>, kOtherSide};
  const std::polymorphic<Figure> source{std::in_place_type<Circle>, kRadius};
  target = source;

  EXPECT_TRUE(typeid(held(target)) == typeid(Circle)) << typeid(held(target)).name();
}

TEST(PolymorphicFigure, the_source_is_valueless_after_a_move) {
  std::polymorphic<Figure> source{std::in_place_type<Square>, kSide};
  const auto taken = std::move(source);

  EXPECT_TRUE(source.valueless_after_move());
  EXPECT_FALSE(taken.valueless_after_move());
  EXPECT_DOUBLE_EQ(taken->area(), kSide * kSide);
}

TEST(PolymorphicFigure, there_is_no_conversion_from_polymorphic_Square_to_polymorphic_Figure) {
  static_assert(!std::is_convertible_v<std::polymorphic<Square>, std::polymorphic<Figure>>);
  static_assert(!std::is_constructible_v<std::polymorphic<Figure>, std::polymorphic<Square>>);
  static_assert(!std::is_assignable_v<std::polymorphic<Figure>&, std::polymorphic<Square>>);
  SUCCEED();
}

TEST(PolymorphicFigure, a_copy_works_in_constant_evaluation) {
  constexpr bool keeps_type = [] {
    std::polymorphic<Figure> original{std::in_place_type<Tile>, kSide, kGrout};
    auto copy = original;
    const Figure& inside = *copy;
    return typeid(inside) == typeid(Tile) && &*copy != &*original;
  }();

  EXPECT_TRUE(keeps_type);
}

}  // namespace
