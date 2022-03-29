// -*- mode: c++; c-basic-offset: 2; -*-

/**
 * @file   pmemops_msync.hh
 * @date   June 22, 2020
 * @brief  Persistent memory operations using msync
 */

#include <cstring>
#include <sys/mman.h>

#include "nvsl/error.hh"
#include "nvsl/pmemops/declarations.hh"

inline void nvsl::PMemOpsMsync::persist(void *base, size_t size) const {
  this->flush(base, size);
}

inline void nvsl::PMemOpsMsync::flush(void *base, size_t size) const {
  msync(base, size, MS_SYNC);
}

inline void nvsl::PMemOpsMsync::drain() const {
  /* Do nothing */
}

inline void nvsl::PMemOpsMsync::memcpy(void *dest, void *src, size_t size) const {
  DBGH(4) << "MEMCPY :: pmemdest " << (void *)(dest) << " src " << (void *)(src)
          << " len " << size << std::endl;

  this->memmove(dest, src, size);
}

inline void nvsl::PMemOpsMsync::memmove(void *dest, void *src, size_t size) const {
  std::memmove(dest, src, size);

  this->flush(dest, size);
  this->drain();
}

inline void nvsl::PMemOpsMsync::memset(void *base, char c, size_t size) const {
  DBGH(4) << "MEMSET :: start " << (void *)(base) << " size " << (void *)(size)
          << " char " << c << std::endl;

  std::memset(base, c, size);
  this->flush(base, size);
  this->drain();
}
