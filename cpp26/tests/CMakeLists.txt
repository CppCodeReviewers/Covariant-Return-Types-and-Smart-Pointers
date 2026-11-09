include(GoogleTest)
include(Clone26Targets)

clone26_add_target(design_a_tests TEST
  SOURCES design_a_test.cpp
  REQUIRES DEDUCING_THIS)

# = delete("reason") is a C++26 feature that a compiler may lack on its own, so its demonstration is a target of
# its own.
clone26_add_target(delete_reason_tests TEST
  SOURCES delete_reason_test.cpp
  REQUIRES DELETE_REASON)

clone26_add_target(design_b_tests TEST
  SOURCES design_b_test.cpp
  REQUIRES POLYMORPHIC)
