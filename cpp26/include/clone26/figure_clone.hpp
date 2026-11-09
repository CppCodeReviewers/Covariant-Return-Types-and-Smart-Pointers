#pragma once

// The figures of design A: Figure, Square, Circle and Tile derive from the Cloneable mixin of cloneable.hpp and
// override its two hooks. ForgetfulTile is a Tile that forgot both overrides, kept next to Tile so that the nets
// in cpp26/tests have something to catch. Nothing but the nets uses it.

#include <clone26/cloneable.hpp>
#include <memory>
#include <numbers>
#include <utility>
#include <vector>

namespace shapes {

using clone26::Cloneable;

inline constexpr int kDefaultGrout = 7;

// The implicit copy and move operations of Figure are public, so they are made protected here again.
struct Figure : Cloneable {
  constexpr virtual double area() const = 0;

 protected:
  constexpr Figure() = default;
  constexpr Figure(const Figure&) = default;
  constexpr Figure(Figure&&) = default;
  constexpr Figure& operator=(const Figure&) = default;
  constexpr Figure& operator=(Figure&&) = default;
};

struct Square : Figure {
  double side = 1;
  std::vector<double> history{};

  constexpr Square() = default;
  constexpr explicit Square(double s) : side{s}, history{s} {}

  constexpr double area() const override { return side * side; }

 private:
  constexpr std::unique_ptr<Cloneable> copy_() const override { return std::make_unique<Square>(*this); }
  constexpr std::unique_ptr<Cloneable> move_() override { return std::make_unique<Square>(std::move(*this)); }
};

struct Circle : Figure {
  double radius = 1;

  constexpr Circle() = default;
  constexpr explicit Circle(double r) : radius{r} {}

  constexpr double area() const override { return std::numbers::pi * radius * radius; }

 private:
  constexpr std::unique_ptr<Cloneable> copy_() const override { return std::make_unique<Circle>(*this); }
  constexpr std::unique_ptr<Cloneable> move_() override { return std::make_unique<Circle>(std::move(*this)); }
};

struct Tile : Square {
  using Square::Square;
  int grout = kDefaultGrout;

  constexpr Tile() = default;
  constexpr Tile(double s, int grout_width) : Square{s}, grout{grout_width} {}

 private:
  constexpr std::unique_ptr<Cloneable> copy_() const override { return std::make_unique<Tile>(*this); }
  constexpr std::unique_ptr<Cloneable> move_() override { return std::make_unique<Tile>(std::move(*this)); }
};

// A Tile that forgot to override copy_ and move_.
struct ForgetfulTile : Square {
  using Square::Square;
  int grout = kDefaultGrout;
};

}  // namespace shapes
