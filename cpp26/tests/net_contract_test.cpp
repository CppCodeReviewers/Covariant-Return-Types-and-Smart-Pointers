// Net 1, the counting target: the contract_assert behind clone26::check_clone_type compares the dynamic type of the
// source with the dynamic type of the object the hook made, before the downcast in clone(). This target replaces the
// contract violation handler with one that counts violations, and it is built with the observe semantic, so a
// violation calls the handler and the program goes on. The tests read the counter.
//
// The classes of the project must record no violation. ForgetfulTile, which never overrides the hooks, must record
// exactly one violation per clone. Every clone below goes through a Figure&, so the check runs on a Figure and
// the downcast is a no-op: nothing here is undefined behaviour. net_contract_death covers the enforce semantic and
// the clone through the derived type.
//
// This target is built with contracts/include before include, which swaps in the checked clone_check.hpp.
#include <gtest/gtest.h>

#include <clone26/figure_clone.hpp>
#include <contracts>
#include <utility>

#include "support/test_values.hpp"

namespace {

int violations = 0;

}  // namespace

// The replaceable handler of the contract-checking implementation. It is called for each violation under observe.
void handle_contract_violation(const std::contracts::contract_violation&) { ++violations; }

namespace {

using namespace clone26::testing_values;
using shapes::Circle;
using shapes::Figure;
using shapes::ForgetfulTile;
using shapes::Square;
using shapes::Tile;

class NetContractCountTest : public ::testing::Test {
 protected:
  void SetUp() override { violations = 0; }
};

TEST_F(NetContractCountTest, the_classes_that_override_both_hooks_record_no_violation) {
  Square square{kSide};
  Circle circle{kRadius};
  Tile tile{kSide, kGrout};
  Figure& square_base = square;
  Figure& circle_base = circle;
  Figure& tile_base = tile;

  auto from_square = square_base.clone();
  auto from_circle = circle_base.clone();
  auto from_tile = tile_base.clone();
  auto moved_square = static_cast<Figure&&>(square).clone();
  auto moved_tile = static_cast<Figure&&>(tile).clone();

  EXPECT_EQ(violations, 0);
}

TEST_F(NetContractCountTest, Tile_copied_through_a_Figure_reference_is_not_a_violation) {
  Tile tile{kSide, kGrout};
  Figure& base = tile;
  auto copy = base.clone();

  EXPECT_EQ(violations, 0);
}

TEST_F(NetContractCountTest, Tile_moved_through_a_Figure_rvalue_is_not_a_violation) {
  Tile tile{kSide, kGrout};
  auto moved = static_cast<Figure&&>(tile).clone();

  EXPECT_EQ(violations, 0);
}

TEST_F(NetContractCountTest, probe_ForgetfulTile_copied_through_a_Figure_reference_is_one_violation) {
  ForgetfulTile forgetful{kSide};
  Figure& base = forgetful;
  auto copy = base.clone();

  EXPECT_EQ(violations, 1);
}

TEST_F(NetContractCountTest, probe_ForgetfulTile_moved_through_a_Figure_rvalue_is_one_violation) {
  ForgetfulTile forgetful{kSide};
  auto moved = static_cast<Figure&&>(forgetful).clone();

  EXPECT_EQ(violations, 1);
}

}  // namespace
