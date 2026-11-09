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

clone26_add_target(design_c_tests TEST
  SOURCES design_c_test.cpp
  REQUIRES DEDUCING_THIS REFLECTION)

# The guard for the requirement of design C: two translation units of one executable export the classes that
# clone() searches, as each of them sees them, and a third compares the lists.
clone26_add_target(design_c_odr TEST
  SOURCES design_c_odr_test.cpp design_c_tu_a.cpp design_c_tu_b.cpp
  REQUIRES DEDUCING_THIS REFLECTION)
