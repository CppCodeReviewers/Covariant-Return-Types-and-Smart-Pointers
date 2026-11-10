#pragma once

// Design B: std::polymorphic<Figure>. The wrapper copies the dynamic type of what it holds, so the classes need
// no clone(), no hooks and no override to forget. The classes mirror figure_clone.hpp, in their own namespace so
// that both headers can live in one program without two definitions of one name.
//
// The header needs a library with std::polymorphic. The build includes it only in a target that a feature check
// found such a library for.

#include <memory>
#include <numbers>
#include <utility>
#include <vector>

namespace shapes_polymorphic {

inline constexpr int kDefaultGrout = 7;

struct Figure {
  constexpr virtual ~Figure() = default;
  constexpr virtual double area() const = 0;
};

struct Square : Figure {
  double side = 1;
  std::vector<double> history{};

  constexpr Square() = default;
  constexpr explicit Square(double s) : side{s}, history{s} {}

  constexpr double area() const override { return side * side; }
};

struct Circle : Figure {
  double radius = 1;

  constexpr Circle() = default;
  constexpr explicit Circle(double r) : radius{r} {}

  constexpr double area() const override { return std::numbers::pi * radius * radius; }
};

struct Tile : Square {
  using Square::Square;
  int grout = kDefaultGrout;

  constexpr Tile() = default;
  constexpr Tile(double s, int grout_width) : Square{s}, grout{grout_width} {}
};

}  // namespace shapes_polymorphic
