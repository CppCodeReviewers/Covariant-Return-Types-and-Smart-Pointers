# Feature checks. Each one compiles and links a small program that uses the feature, with the standard and the
# flags the project is configured with (CMAKE_CXX_STANDARD, CMAKE_CXX_FLAGS). A target that needs a feature the
# compiler lacks is not created, and Clone26Targets.cmake says so with a "skipping" line.
#
# Set CMAKE_CXX_STANDARD before including this file: try_compile honours it (policy CMP0067).

include(CheckCXXSourceCompiles)

# CMake caches the result of every check. When the compiler or its flags change in an existing build directory,
# the cached answers are about another configuration, so they are dropped and the checks run again.
set(clone26_probe_variables CLONE26_HAVE_DEDUCING_THIS CLONE26_HAVE_DELETE_REASON CLONE26_HAVE_POLYMORPHIC CLONE26_HAVE_REFLECTION)
set(clone26_probe_signature "${CMAKE_CXX_COMPILER}|${CMAKE_CXX_COMPILER_VERSION}|${CMAKE_CXX_FLAGS}|${CMAKE_CXX_STANDARD}")
if(NOT "${CLONE26_PROBE_SIGNATURE}" STREQUAL "${clone26_probe_signature}")
  foreach(variable IN LISTS clone26_probe_variables)
    unset(${variable} CACHE)
  endforeach()
  set(CLONE26_PROBE_SIGNATURE "${clone26_probe_signature}" CACHE INTERNAL "Compiler and flags the feature checks ran with")
endif()

set(CLONE26_DESCRIPTION_DEDUCING_THIS "explicit object parameters")
check_cxx_source_compiles([=[
#ifndef __cpp_explicit_this_parameter
#error "no explicit object parameters"
#endif
struct S {
  template <class Self>
  int f(this Self&&) { return 1; }
};
int main() { return S{}.f() - 1; }
]=] CLONE26_HAVE_DEDUCING_THIS)

set(CLONE26_DESCRIPTION_DELETE_REASON "= delete(\"reason\")")
check_cxx_source_compiles([=[
struct S {
  S& operator=(const S&) = delete("assigning through a base slices");
};
int main() { return 0; }
]=] CLONE26_HAVE_DELETE_REASON)

set(CLONE26_DESCRIPTION_POLYMORPHIC "std::polymorphic (__cpp_lib_polymorphic)")
check_cxx_source_compiles([=[
#include <memory>
#ifndef __cpp_lib_polymorphic
#error "no std::polymorphic"
#endif
struct Base {
  virtual ~Base() = default;
};
int main() {
  std::polymorphic<Base> held{std::in_place_type<Base>};
  auto copy = held;
  return &*copy == &*held ? 1 : 0;
}
]=] CLONE26_HAVE_POLYMORPHIC)

set(CLONE26_DESCRIPTION_REFLECTION "reflection (__cpp_impl_reflection)")
check_cxx_source_compiles([=[
#include <meta>
#ifndef __cpp_impl_reflection
#error "no reflection"
#endif
struct S {
  int a;
};
constexpr auto kMembers = std::define_static_array(
    std::meta::members_of(^^S, std::meta::access_context::unchecked()));
int main() {
  template for (constexpr auto member : kMembers) {
    static_assert(!std::meta::is_namespace(member));
  }
  return 0;
}
]=] CLONE26_HAVE_REFLECTION)
