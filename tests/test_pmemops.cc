// -*- mode: c++; c-basic-offset: 2; -*-

/**
 * @file   test_pmemops.cc
 * @date   Mars  29, 2021
 * @brief  Test pmemops functions
 */

#include <cstdlib>
#include <iostream>

#include "gtest/gtest.h"
#include "nvsl/pmemops.hh"

void do_check(nvsl::PMemOps *pmemops) {
  char src[1024], dst[1024];
  pmemops->memset(src, 'c', 1024);
  pmemops->memcpy(dst, src, 1024);

  EXPECT_EQ(0, memcmp(src, dst, 1024));

  EXPECT_EQ('c', src[0]);
  EXPECT_EQ('c', src[1023]);

  EXPECT_EQ('c', dst[0]);
  EXPECT_EQ('c', dst[1023]);  
}

TEST(pmemops, clwb_if_available) {
  nvsl::PMemOps *pmemops
#ifdef CLWB_AVAIL
    = new nvsl::PMemOpsClwb();
#else
    = new nvsl::PMemOpsMsync();
  std::cout << "CLWB unavailable" << std::endl;
#endif

  do_check(pmemops);
}

TEST(pmemops, clflushopt_if_available) {
  nvsl::PMemOps *pmemops_clflushopt
#ifdef CLFLUSHOPT_AVAIL
    = new nvsl::PMemOpsClflushOpt();
#else
    = new nvsl::PMemOpsMsync();
  std::cout << "CLFlushOpt unavailable" << std::endl;
#endif

  do_check(pmemops_clflushopt);
}

TEST(pmemops, msync) {
  nvsl::PMemOps *pmemops_np = new nvsl::PMemOpsNoPersist();

  do_check(pmemops_np);
}
