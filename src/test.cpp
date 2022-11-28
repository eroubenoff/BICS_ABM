#include "gtest/gtest.h"
#include "../BICS_ABM.h"

TEST(HelloTest, BasicAssertions) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
  // EXPECT_EQ(7 * 6, 43);
}
