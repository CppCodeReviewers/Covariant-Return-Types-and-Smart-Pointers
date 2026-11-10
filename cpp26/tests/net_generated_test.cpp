// Net 3b: generated tests. Reflection lists every concrete class derived from shapes::Figure in the namespace,
// and one test body runs a clone round trip on each of them with `template for`, so a class added to the
// namespace is tested without being added to a list. The round trip has to keep the dynamic type of every class
// except ForgetfulTile, and the test checks both: the classes that keep it and the one that does not. A Tile
// that lost its override fails here by name. A generated test needs a default constructor and compares dynamic
// types only.
#include <gtest/gtest.h>

#include <clone26/figure_clone.hpp>
#include <clone26/reflection_net.hpp>
#include <meta>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <vector>

namespace {

constexpr auto kFigures = std::define_static_array(clone26::concrete_figures(^^shapes, ^^shapes::Figure));

// Both hooks: a copy through a base reference and a move through a base rvalue keep the dynamic type.
template <class T>
bool clone_keeps_type() {
  static_assert(std::is_default_constructible_v<T>, "a generated test needs a default constructor");
  T copy_source{};
  const shapes::Figure& copy_base = copy_source;
  auto copy = copy_base.clone();
  const shapes::Figure& copied = *copy;

  T move_source{};
  const std::type_info& source_type = typeid(move_source);
  auto moved = static_cast<shapes::Figure&&>(move_source).clone();
  const shapes::Figure& moved_ref = *moved;

  return typeid(copied) == typeid(copy_source) && typeid(moved_ref) == source_type;
}

// The namespace has an alias of Tile and a forward declaration besides the four classes of the hierarchy. The list
// has each class once, in declaration order, and neither of the two.
TEST(NetGenerated, the_list_has_each_class_of_the_hierarchy_once) {
  std::vector<std::string_view> names;
  template for (constexpr auto cls : kFigures) { names.push_back(std::meta::identifier_of(cls)); }

  const std::vector<std::string_view> expected{"Square", "Circle", "Tile", "ForgetfulTile"};
  EXPECT_EQ(names, expected);
}

TEST(NetGenerated, every_concrete_figure_keeps_its_dynamic_type_except_ForgetfulTile) {
  ASSERT_FALSE(kFigures.empty());
  template for (constexpr auto cls : kFigures) {
    constexpr std::string_view name = std::meta::identifier_of(cls);
    constexpr bool forgot = cls == ^^shapes::ForgetfulTile;
    SCOPED_TRACE(name);

    EXPECT_EQ(clone_keeps_type<typename[:cls:]>(), !forgot) << name;
  }
}

}  // namespace
