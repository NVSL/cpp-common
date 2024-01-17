// -*- mode: c++; c-basic-offset: 2; -*-

/**
 * @file   pmemops.hh
 * @date   mars 29, 2022
 * @brief  Common abstraction for PMEM operations (clwb, fence, clflush...)
 */

#pragma once

#include <cassert>
#include <cstdint>
#include <stddef.h>

#undef NVSL_PMEMOPS_MASTER_FILE
#define NVSL_PMEMOPS_MASTER_FILE

#include "nvsl/pmemops/declarations.hh"
#include "nvsl/pmemops/pmemops_clflushopt.hh"
#include "nvsl/pmemops/pmemops_clwb.hh"
#include "nvsl/pmemops/pmemops_msync.hh"
#include "nvsl/pmemops/pmemops_nopersist.hh"

#undef NVSL_PMEMOPS_MASTER_FILE
