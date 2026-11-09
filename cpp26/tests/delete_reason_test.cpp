// = delete("reason") in a base. C++26 allows a reason in a deleted function. It is not a way to stop slicing:
// a deleted copy assignment in the base deletes the assignment of every derived class too, so the class stays
// unassignable even though it never mentioned assignment. The classes of the project keep the base's
// assignment protected and defaulted instead (see design_a_test.cpp). The type traits state what the compiler
// does, so the build passing is the proof.
#include <gtest/gtest.h>

#include <type_traits>

namespace {

struct DeletedBase {
  virtual ~DeletedBase() = default;
  DeletedBase& operator=(const DeletedBase&) = delete ("assigning through a base slices; use clone()");
};

struct DeletedDerived : DeletedBase {};

TEST(DeleteReason, a_deleted_assignment_in_the_base_deletes_the_derived_assignment) {
  static_assert(!std::is_assignable_v<DeletedBase&, const DeletedBase&>);
  static_assert(!std::is_copy_assignable_v<DeletedDerived>);
  static_assert(!std::is_move_assignable_v<DeletedDerived>);
  SUCCEED();
}

}  // namespace
