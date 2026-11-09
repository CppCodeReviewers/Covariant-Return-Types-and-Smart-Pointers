#pragma once

// Design A: a clone() mixin with hooks. Cloneable has no template parameters. One member function template with an
// explicit object parameter (deducing this) deduces the type clone() is called on and returns
// std::unique_ptr<Derived>. Two private virtual hooks, copy_() and move_(), do the virtual part. They return
// std::unique_ptr<Cloneable>, so they are not covariant and no raw owning pointer crosses a virtual call.
//
// clone() on an lvalue or a const object copies through copy_(). clone() on a non-const rvalue moves through
// move_(). A class that derives from a Cloneable and does not override both hooks is cloned as its nearest
// ancestor that does: clone() then makes an object of the wrong type and the downcast is undefined behaviour.
//
// The copy and move operations are protected and defaulted, so a user of a base reference cannot slice by
// assigning. A derived class stays assignable. A deleted operator in the base would delete the derived one too.

#include <memory>
#include <type_traits>

namespace clone26 {

struct Cloneable {
  constexpr virtual ~Cloneable() = default;

  template <class Self>
  constexpr auto clone(this Self&& self) -> std::unique_ptr<std::remove_cvref_t<Self>> {
    using T = std::remove_cvref_t<Self>;
    static_assert(std::is_base_of_v<Cloneable, T>);
    std::unique_ptr<Cloneable> made;
    if constexpr (std::is_lvalue_reference_v<Self> || std::is_const_v<std::remove_reference_t<Self>>) {
      made = static_cast<const Cloneable&>(self).copy_();
    } else {
      made = static_cast<Cloneable&>(self).move_();
    }
    return std::unique_ptr<T>(static_cast<T*>(made.release()));
  }

 protected:
  constexpr Cloneable() = default;
  constexpr Cloneable(const Cloneable&) = default;
  constexpr Cloneable(Cloneable&&) = default;
  constexpr Cloneable& operator=(const Cloneable&) = default;
  constexpr Cloneable& operator=(Cloneable&&) = default;

 private:
  constexpr virtual std::unique_ptr<Cloneable> copy_() const = 0;
  constexpr virtual std::unique_ptr<Cloneable> move_() = 0;
};

}  // namespace clone26
