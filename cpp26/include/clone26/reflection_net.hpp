#pragma once

// Reflection over one namespace of one translation unit: which classes are concrete Figures, and which of them
// do not declare an override of both hooks. The lint (net_lint) and the generated tests (net_generated) share it.
// A class in another namespace or another translation unit is not seen, and neither is an alias or a forward
// declaration.

#include <cstddef>
#include <meta>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace clone26 {

inline constexpr auto kAccess = std::meta::access_context::unchecked();
inline constexpr std::string_view kCopyHook = "copy_";
inline constexpr std::string_view kMoveHook = "move_";

consteval bool declares_override(std::meta::info cls, std::string_view hook) {
  for (auto member : std::meta::members_of(cls, kAccess)) {
    if (std::meta::is_function(member) && std::meta::has_identifier(member) &&
        std::meta::identifier_of(member) == hook && std::meta::is_override(member)) {
      return true;
    }
  }
  return false;
}

consteval std::vector<std::meta::info> concrete_figures(std::meta::info ns, std::meta::info base) {
  std::vector<std::meta::info> found;
  for (auto member : std::meta::members_of(ns, kAccess)) {
    // The order matters. An alias names a class a second time, so it is skipped. A forward declaration has no bases
    // to ask about, so completeness is checked before the inheritance and abstractness queries.
    if (std::meta::is_type(member) && std::meta::dealias(member) == member && std::meta::is_class_type(member) &&
        std::meta::is_complete_type(member) && member != base && std::meta::is_base_of_type(base, member) &&
        !std::meta::is_abstract_type(member)) {
      found.push_back(member);
    }
  }
  return found;
}

consteval std::vector<std::meta::info> lacking_hooks(std::meta::info ns, std::meta::info base) {
  std::vector<std::meta::info> found;
  for (auto cls : concrete_figures(ns, base)) {
    if (!declares_override(cls, kCopyHook) || !declares_override(cls, kMoveHook)) found.push_back(cls);
  }
  return found;
}

// The text of a failed lint: what the lint had to find, and the name of each class it did find. A static_assert
// message has to outlive the constant evaluation, so the text is stored with define_static_string.
consteval std::string_view lint_message(std::string_view expectation, std::span<const std::meta::info> found) {
  std::string text{expectation};
  text += "; it names: ";
  if (found.empty()) text += "nothing";
  for (std::size_t i = 0; i < found.size(); ++i) {
    if (i != 0) text += ", ";
    text += std::meta::identifier_of(found[i]);
  }
  return std::string_view{std::define_static_string(text)};
}

}  // namespace clone26
