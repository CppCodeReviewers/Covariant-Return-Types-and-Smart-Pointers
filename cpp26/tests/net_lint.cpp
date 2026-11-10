// Net 3a: a lint. Reflection lists every concrete class derived from shapes::Figure in the namespace and keeps
// those that do not declare an override of both hooks. The namespace holds one class that forgot, ForgetfulTile,
// so the lint has to name exactly that class. The lint is run the way a project would run it as a guard,
// static_assert(lacking.empty()), whenever the list has to be empty. Here the list is checked for its content:
// a Tile that lost its override would make it longer. This translation unit is built and never run. A class in
// another namespace or another translation unit is not seen.
#include <clone26/figure_clone.hpp>
#include <clone26/reflection_net.hpp>
#include <cstddef>
#include <meta>
#include <string_view>

namespace {

// The classes of the hierarchy: Square, Circle, Tile and ForgetfulTile. The namespace also has an alias of Tile and a
// forward declaration, which are not classes of the hierarchy and must not be listed.
constexpr std::size_t kHierarchySize = 4;
constexpr auto kConcrete = std::define_static_array(clone26::concrete_figures(^^shapes, ^^shapes::Figure));

static_assert(kConcrete.size() == kHierarchySize,
              "the lint has to list each class once, and neither an alias nor a forward declaration");

constexpr std::string_view kExpectation = "the lint has to name exactly ForgetfulTile as lacking an override";
constexpr auto kLacking = std::define_static_array(clone26::lacking_hooks(^^shapes, ^^shapes::Figure));

static_assert(kLacking.size() == 1 && kLacking[0] == ^^shapes::ForgetfulTile,
              clone26::lint_message(kExpectation, kLacking));

}  // namespace
