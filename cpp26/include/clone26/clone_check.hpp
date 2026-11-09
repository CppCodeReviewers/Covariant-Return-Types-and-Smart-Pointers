#pragma once

// The hook that clone() calls between the virtual hook and the downcast: source is the object that is cloned and
// made is the object the hook returned. The default does nothing. contracts/include/clone26/clone_check.hpp has
// the same name and checks that both have one dynamic type; a target that wants the check puts that directory
// before this one on its include path.
namespace clone26 {

template <class Source, class Made>
constexpr void check_clone_type(const Source&, const Made&) noexcept {}

}  // namespace clone26
