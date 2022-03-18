// -*- mode: c++; c-basic-offset: 2; -*-

/**
 * @file   test_string.cc
 * @date   June  21, 2021
 * @brief  Test string functions
 */

#include <cstdlib>
#include <iostream>

#include "gtest/gtest.h"
#include "nvsl/string.hh"

TEST(string, split) {
  const std::string test_str = "Hello! World.";
  const auto toks = nvsl::split(test_str, " ");

  EXPECT_TRUE(toks[0] == "Hello!");
  EXPECT_TRUE(toks[1] == "World.");
  
}
