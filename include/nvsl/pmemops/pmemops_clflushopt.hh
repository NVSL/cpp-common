// -*- mode: c++; c-basic-offset: 2; -*-

/**
 * @file   pmemops_clflushopt.hh
 * @date   June 22, 2020
 * @brief  Persistent memory operations using clflushopt instruction
 */

#include <cstring>
#include <sys/mman.h>
#include <xmmintrin.h>

#include "nvsl/error.hh"
#include "nvsl/pmemops/declarations.hh"

inline void nvsl::PMemOpsClflushOpt::persist(void *base, size_t size) const {
  DBGH(4) << "Persisting " << (base) << "of size " << (void *)(size)
          << std::endl;
  this->flush(base, size);
  this->drain();
}

inline void nvsl::PMemOpsClflushOpt::clflush_opt(void *addr) const {
  asm volatile(".byte 0x66; clflush %0" : "+m"(*(volatile char *)(addr)));
}

inline void nvsl::PMemOpsClflushOpt::flush(void *base, size_t size) const {
  uintptr_t uptr;

  /*
   * Loop through cache-line-size (typically 64B) aligned chunks
   * covering the given range.
   */
  for (uptr = (uintptr_t)base & ~(CL_SIZE - 1); uptr < (uintptr_t)base + size;
       uptr += CL_SIZE) {

#ifdef CLFLUSHOPT_AVAIL
    this->clflush_opt((void *)uptr);
#else
    NVSL_ERROR("This version of libpuddles was built on a platform "
               "that doesn't support clflushopt");
#endif
  }
}

inline void nvsl::PMemOpsClflushOpt::drain() const {
#ifdef SFENCE_AVAIL
  _mm_sfence();
#else
  NVSL_ERROR("This version of libpuddles was built on a platform "
             "that doesn't support sfence");
#endif
}

inline void nvsl::PMemOpsClflushOpt::memcpy(void *dest, void *src,
                                            size_t size) const {
  DBGH(4) << "MEMCPY :: pmemdest " << (void *)(dest) << " src " << (void *)(src)
          << " len " << size << std::endl;

  this->memmove(dest, src, size);
}

inline void nvsl::PMemOpsClflushOpt::memmove(void *dest, void *src,
                                             size_t size) const {
  std::memmove(dest, src, size);

  this->flush(dest, size);
  this->drain();
}

inline void nvsl::PMemOpsClflushOpt::memset(void *base, char c,
                                            size_t size) const {
  DBGH(4) << "MEMSET :: start " << (void *)(base) << " size " << (void *)(size)
          << " char " << c << std::endl;

  std::memset(base, c, size);
  flush(base, size);
  drain();
}
