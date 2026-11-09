#pragma once

// Design C: a clone() mixin with no hooks. AutoCloneable has no template parameters. Its clone() deduces the type
// it is called on, then reflection lists the concrete classes derived from AutoCloneable in the namespace of that
// type and copies with the one whose type matches the dynamic type of the source. A class that derives from it
// writes nothing: there is no override to forget and no type name to repeat.
//
// REQUIREMENT: every translation unit of one program must see the same, finished set of classes in the searched
// namespace. clone() is a template, so its body, with the dispatch over that set, is generated in every translation
// unit that calls it, and the linker keeps one copy of each specialization. Two translation units that see different
// classes in the namespace make the same specialization with different bodies. That violates the one definition
// rule, no diagnostic is required, and the symptom can be a clone_unavailable for a class that the caller sees.
// So define the whole hierarchy in ONE header (figure_reflected.hpp is that header here), add nothing to the
// namespace in another translation unit, and add nothing after the first clone() instantiation. The way to extend is
// a class in another namespace, cloned through its own type: its own namespace is searched for it.
// searched_class_names lets a test check the requirement: see tests/design_c_odr_test.cpp.
//
// Other limits: the set has the classes of one namespace that are complete where clone() is instantiated. Aliases
// and forward declarations are not in it. When the dynamic type of the source is not in it, which happens for a
// class of another namespace cloned through a base reference of the searched one, clone() throws clone_unavailable
// instead of returning an empty pointer. A test or a lint should cover every class that uses the mixin, because
// the failure shows at run time.

#include <memory>
#include <meta>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

namespace clone26 {

// Thrown by AutoCloneable::clone when no class of the searched namespace has the dynamic type of the source. The
// message holds typeid(source).name(), which is a mangled name on GCC and Clang.
struct clone_unavailable : std::logic_error {
  explicit clone_unavailable(const std::type_info& dynamic_type)
      : std::logic_error{std::string{"AutoCloneable::clone: no class of the searched namespace has the dynamic type "} +
                         dynamic_type.name()} {}
};

// The classes that clone() can copy to: the concrete classes of the namespace that derive from the base. The order
// of the checks matters. An alias names a class a second time, so it is skipped. A forward declaration has no bases
// to ask about, so completeness is checked before the inheritance and abstractness queries.
consteval std::vector<std::meta::info> searched_classes(std::meta::info ns, std::meta::info base) {
  std::vector<std::meta::info> found;
  for (auto member : std::meta::members_of(ns, std::meta::access_context::unchecked())) {
    if (std::meta::is_type(member) && std::meta::dealias(member) == member && std::meta::is_class_type(member) &&
        std::meta::is_complete_type(member) && std::meta::is_base_of_type(base, member) &&
        !std::meta::is_abstract_type(member)) {
      found.push_back(member);
    }
  }
  return found;
}

struct AutoCloneable {
  virtual ~AutoCloneable() = default;

  template <class Self>
  auto clone(this Self&& self) -> std::unique_ptr<std::remove_cvref_t<Self>> {
    using T = std::remove_cvref_t<Self>;
    const AutoCloneable& source = self;
    static constexpr auto classes =
        std::define_static_array(searched_classes(std::meta::parent_of(std::meta::dealias(^^T)), ^^AutoCloneable));
    std::unique_ptr<T> out;
    template for (constexpr auto member : classes) {
      using C = typename[:member:];
      if constexpr (std::is_base_of_v<T, C>) {
        if (!out && typeid(source) == typeid(C)) {  // the first match only: a second would use a moved-from source
          if constexpr (std::is_lvalue_reference_v<Self> || std::is_const_v<std::remove_reference_t<Self>>) {
            out = std::make_unique<C>(static_cast<const C&>(self));
          } else {
            out = std::make_unique<C>(std::move(static_cast<C&>(self)));
          }
        }
      }
    }
    if (!out) throw clone_unavailable{typeid(source)};
    return out;
  }
};

namespace {

// The names of the classes that clone() searches in the namespace, as this translation unit sees them, in
// declaration order. The function has internal linkage on purpose, and it is a template: every translation unit
// gets an instantiation of its own, so two translation units cannot share one result by accident. A program that
// wants to check the requirement above exports the result of each translation unit through a function with a name
// of its own and compares the lists. The names are those that the translation unit sees where this function is
// instantiated, which is the point that decides for clone() as well.
template <std::meta::info Namespace>
std::vector<std::string> searched_class_names() {
  std::vector<std::string> names;
  template for (constexpr auto member : std::define_static_array(searched_classes(Namespace, ^^AutoCloneable))) {
    names.emplace_back(std::meta::identifier_of(member));
  }
  return names;
}

}  // namespace

}  // namespace clone26
