// Net 1, the death target: the same contract_assert as net_contract, built with the enforce semantic, under which a
// violation terminates the process. Each test runs the clone in a child process (a death test), which also covers
// the clone through the derived type, where the downcast would be undefined behaviour without the check.
//
// Two kinds of test. The probes clone ForgetfulTile, which never overrides the hooks, and expect the violation:
// they show the net can fire. The real tests clone the Tile of the project and expect a clean exit: they fail when
// Tile loses its overrides. This target is built with contracts/include before include.
#include <gtest/gtest.h>

#include <clone26/figure_clone.hpp>
#include <cstdlib>
#include <utility>

#include "support/test_values.hpp"

namespace {

using namespace clone26::testing_values;
using shapes::Figure;
using shapes::ForgetfulTile;
using shapes::Tile;

constexpr int kCleanExit = 0;

[[noreturn]] void copy_then_exit(Figure& figure) {
  [[maybe_unused]] auto copy = figure.clone();
  std::exit(kCleanExit);
}

[[noreturn]] void move_then_exit(Figure&& figure) {
  [[maybe_unused]] auto moved = std::move(figure).clone();
  std::exit(kCleanExit);
}

class NetContractDeathTest : public ::testing::Test {
 protected:
  void SetUp() override { GTEST_FLAG_SET(death_test_style, "threadsafe"); }
};

TEST_F(NetContractDeathTest, probe_copy_of_ForgetfulTile_is_caught_through_Figure) {
  ForgetfulTile forgetful{kSide};
  EXPECT_DEATH(copy_then_exit(forgetful), "contract violation");
}

TEST_F(NetContractDeathTest, probe_copy_of_ForgetfulTile_is_caught_through_its_own_type) {
  ForgetfulTile forgetful{kSide};
  EXPECT_DEATH([&] { [[maybe_unused]] auto copy = forgetful.clone(); }(), "contract violation");
}

TEST_F(NetContractDeathTest, probe_move_of_ForgetfulTile_is_caught_through_Figure) {
  ForgetfulTile forgetful{kSide};
  EXPECT_DEATH(move_then_exit(std::move(forgetful)), "contract violation");
}

TEST_F(NetContractDeathTest, Tile_copied_through_a_Figure_reference_is_not_a_violation) {
  Tile tile{kSide, kGrout};
  EXPECT_EXIT(copy_then_exit(tile), ::testing::ExitedWithCode(kCleanExit), "");
}

TEST_F(NetContractDeathTest, Tile_copied_through_its_own_type_is_not_a_violation) {
  Tile tile{kSide, kGrout};
  EXPECT_EXIT(
      {
        [[maybe_unused]] auto copy = tile.clone();
        std::exit(kCleanExit);
      },
      ::testing::ExitedWithCode(kCleanExit), "");
}

TEST_F(NetContractDeathTest, Tile_moved_through_a_Figure_rvalue_is_not_a_violation) {
  Tile tile{kSide, kGrout};
  EXPECT_EXIT(move_then_exit(std::move(tile)), ::testing::ExitedWithCode(kCleanExit), "");
}

}  // namespace
