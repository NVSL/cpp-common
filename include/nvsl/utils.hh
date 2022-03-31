// -*- mode: c++; c-basic-offset: 2; -*-

/**
 * @file   utils.hh
 * @date   mars 31, 2022
 * @brief  Misc functions
 */

#pragma once

#include <unistd.h>
#include <string>
#include <vector>
#include <sys/mman.h>
#include <sstream>

#include "string.hh"

namespace nvsl {
  inline std::string mmap_to_str(void *addr, size_t len, int prot, int flags,
                                 int fd, off_t off) {
    std::vector<std::string> flags_v;
    if (flags & MAP_SHARED) {
      flags_v.push_back("MAP_SHARED");
    }
    if (flags & MAP_PRIVATE) {
      flags_v.push_back("MAP_PRIVATE");
    }
    if (flags & MAP_ANONYMOUS) {
      flags_v.push_back("MAP_ANONYMOUS");
    }
    if (flags & MAP_FIXED) {
      flags_v.push_back("MAP_FIXED");
    }
    if (flags & MAP_FIXED_NOREPLACE) {
      flags_v.push_back("MAP_FIXED_NOREPLACE");
    }
    if (flags & MAP_SYNC) {
      flags_v.push_back("MAP_SYNC");
    }

    const auto flags_str = nvsl::zip(flags_v, " | ");

    std::vector<std::string> prot_v;
    if (prot & PROT_READ) {
      prot_v.push_back("PROT_READ");
    }
    if (prot & PROT_WRITE) {
      prot_v.push_back("PROT_WRITE");
    }
    if (prot & PROT_EXEC) {
      prot_v.push_back("PROT_EXEC");
    }

    const auto prot_str = nvsl::zip(prot_v, " | ");

    const auto params_v = {S(addr), S(len), prot_str, flags_str, S(fd), S(off)};
    const auto params = zip(params_v, ", ");

    return "mmap(" + params + ")";
  }
}
