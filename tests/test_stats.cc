// -*- mode: c++; c-basic-offset: 2; -*-

/**
 * @file   test_stats.cc
 * @date   June  21, 2021
 * @brief  Test stats functions
 */

#include <cstdlib>
#include <iostream>

#include "gtest/gtest.h"
#include "nvsl/stats.hh"

TEST(stats, scalar) {
  nvsl::StatsScalar testStat;

  testStat += 100;

  EXPECT_EQ(testStat.avg(), 100);
}
