// The guard for the requirement of design C: every translation unit of one program sees the same finished set of
// classes in the searched namespace. This test is in a third translation unit that does not include the hierarchy.
// It compares the lists that design_c_tu_a.cpp and design_c_tu_b.cpp export, and the three files are linked into
// one executable, as the units of a real program are.
//
// The first test would pass for two empty lists, so the second one names the classes that the lists must hold.
#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "support/odr_lists.hpp"

namespace {

using namespace clone26::testing_odr;

TEST(DesignCOdr, every_translation_unit_sees_the_same_searched_classes) { EXPECT_EQ(tu_a_classes(), tu_b_classes()); }

TEST(DesignCOdr, the_searched_classes_are_the_concrete_classes_of_the_hierarchy_once_each) {
  const std::vector<std::string> expected{"Square", "Circle", "Tile"};

  EXPECT_EQ(tu_a_classes(), expected);
}

}  // namespace
