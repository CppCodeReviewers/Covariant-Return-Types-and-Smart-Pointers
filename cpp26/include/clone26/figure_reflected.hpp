#pragma once

// The figures of design C: the same classes as figure_clone.hpp, derived from the AutoCloneable mixin and with
// nothing to write for cloning. They live in shapes_reflected, because AutoCloneable searches the namespace of
// the type it is called on.

#include <clone26/auto_cloneable.hpp>
#include <numbers>
#include <vector>

namespace shapes_reflected {

inline constexpr int kDefaultGrout = 7;

struct Figure : clone26::AutoCloneable {
  virtual double area() const = 0;
};

struct Square : Figure {
  double side = 1;
  std::vector<double> history{};

  Square() = default;
  explicit Square(double s) : side{s}, history{s} {}

  double area() const override { return side * side; }
};

struct Circle : Figure {
  double radius = 1;

  Circle() = default;
  explicit Circle(double r) : radius{r} {}

  double area() const override { return std::numbers::pi * radius * radius; }
};

struct Tile : Square {  // nothing to forget: no hooks exist
  using Square::Square;
  int grout = kDefaultGrout;

  Tile() = default;
  Tile(double s, int grout_width) : Square{s}, grout{grout_width} {}
};

// Two more members of the namespace that clone() has to get right. A forward declaration is a class that is not
// complete, so it has no bases to ask about. An alias names Tile a second time, and a search that took it for a
// class of its own would copy, or move, twice.
struct Pending;
using TileAlias = Tile;

}  // namespace shapes_reflected
