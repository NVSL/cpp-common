// -*- mode: c++; c-basic-offset: 2; -*-

#pragma once

/**
 * @file   constants.hh
 * @date   May 21, 2021
 * @brief  Brief description here
 */

#include <cstddef>
#include <cstdint>

namespace nvsl {
  /** @brief Enum to help with size of things */
  enum LP_SZ : size_t
  {
    B = 1,
    KiB = 1024 * B,
    MiB = 1024 * KiB,
    GiB = 1024 * MiB,
    TiB = 1024 * GiB
  };

  enum time_unit {
    s_unit = 0,
    ms_unit,
    us_unit,
    ns_unit,
    any_unit
  };

  constexpr size_t CL_SIZE = 64 * LP_SZ::B;
} // namespace nvsl
