// -*- mode: c++; c-basic-offset: 2; -*-

/**
 * @file   pmemops_nopersist.hh
 * @date   June 22, 2020
 * @brief  Persistent memory operations with no persistency guarantee
 */

#include <cstring>
#include <sys/mman.h>

#include "nvsl/error.hh"
#include "nvsl/pmemops/declarations.hh"

inline void nvsl::PMemOpsNoPersist::persist(void *base, size_t size) const {
  (void)(base);
  (void)(size);
}

inline void nvsl::PMemOpsNoPersist::flush(void *base, size_t size) const {
  (void)(base);
  (void)(size);
}

inline void nvsl::PMemOpsNoPersist::drain() const {}

inline void nvsl::PMemOpsNoPersist::memcpy(void *dest, void *src,
                                           size_t size) const {
  DBGH(4) << "MEMCPY :: pmemdest " << (void *)(dest) << " src " << (void *)(src)
          << " len " << size << std::endl;

  this->memmove(dest, src, size);
}

inline void nvsl::PMemOpsNoPersist::memmove(void *dest, void *src,
                                            size_t size) const {
  std::memmove(dest, src, size);
}

inline void nvsl::PMemOpsNoPersist::memset(void *base, char c,
                                           size_t size) const {
  DBGH(4) << "MEMSET :: start " << (void *)(base) << " size " << (void *)(size)
          << " char " << c << std::endl;

  std::memset(base, c, size);
}
