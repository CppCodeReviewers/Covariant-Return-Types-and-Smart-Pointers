#pragma once

// The contracts flavour of include/clone26/clone_check.hpp: the object the hook made has the dynamic type of the
// source. It runs before the downcast in clone(), so a hook that was not overridden is reported instead of cast
// through. A contract on the virtual hooks themselves is not an option: GCC 16.2 rejects a contract on a
// virtual function.
//
// Only targets whose feature check found contracts that compile and link use this header.
#include <typeinfo>

namespace clone26 {

template <class Source, class Made>
constexpr void check_clone_type(const Source& source, const Made& made) {
  contract_assert(typeid(made) == typeid(source));
}

}  // namespace clone26
