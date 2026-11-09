#pragma once

#include <cstddef>

// The numbers the tests use, named once. A side of 4 and a side of 2 give areas of 16 and 4, so a copy that
// mixes up two objects shows in the area.
namespace clone26::testing_values {

inline constexpr double kSide = 4;
inline constexpr double kOtherSide = 2;
inline constexpr double kRadius = 1;
inline constexpr int kGrout = 3;
inline constexpr std::size_t kRowSize = 3;

}  // namespace clone26::testing_values
