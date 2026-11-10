#pragma once

#include <string>
#include <vector>

// The classes that design C searches in shapes_reflected, as the translation unit named in the function sees them.
// The two functions are declared under two names, because each one is defined in its own translation unit and a
// test can then compare what the two units see. The units are design_c_tu_a.cpp and design_c_tu_b.cpp.
namespace clone26::testing_odr {

std::vector<std::string> tu_a_classes();
std::vector<std::string> tu_b_classes();

}  // namespace clone26::testing_odr
