// Net 2: a constant-evaluated round trip. The compiler cannot reach a downcast to the wrong type in a constant
// expression, so a clone that returns the wrong dynamic type is a constant that says false, and a clone through
// the derived type is not a constant at all. The classes that keep their type are asserted, and so is
// ForgetfulTile, which does not: the net has to see it. A Tile that lost its override fails the assertions that
// name Tile. This translation unit is built and never run. It lists the concrete classes by hand: one
// static_assert per class and check.
#include <clone26/figure_clone.hpp>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace {

using shapes::Figure;

// Cloned through the base, the copy has the dynamic type of the source.
template <class T>
consteval bool copy_keeps_type() {
  T original{};
  const Figure& base = original;
  auto copy = base.clone();
  const Figure& copied = *copy;
  return typeid(copied) == typeid(original);
}

// An rvalue is moved through move_(), and the result has the dynamic type of the source too.
template <class T>
consteval bool move_keeps_type() {
  T original{};
  const std::type_info& source_type = typeid(original);
  auto moved = static_cast<Figure&&>(original).clone();
  const Figure& moved_ref = *moved;
  return typeid(moved_ref) == source_type;
}

// Cloning through the derived type is a constant expression only when the hooks make the right type: the
// downcast to a class that the hooks did not make is undefined behaviour, which no constant expression has.
template <class T>
concept clones_in_constant_evaluation = requires { typename std::integral_constant<double, T{}.clone()->area()>; };

static_assert(copy_keeps_type<shapes::Square>());
static_assert(copy_keeps_type<shapes::Circle>());
static_assert(copy_keeps_type<shapes::Tile>());
static_assert(move_keeps_type<shapes::Square>());
static_assert(move_keeps_type<shapes::Circle>());
static_assert(move_keeps_type<shapes::Tile>());
static_assert(clones_in_constant_evaluation<shapes::Tile>);

static_assert(!copy_keeps_type<shapes::ForgetfulTile>(), "a copy of ForgetfulTile has to be seen as another type");
static_assert(!move_keeps_type<shapes::ForgetfulTile>(), "a move of ForgetfulTile has to be seen as another type");
static_assert(!clones_in_constant_evaluation<shapes::ForgetfulTile>,
              "a clone of ForgetfulTile through its own type must not be a constant expression");

}  // namespace
