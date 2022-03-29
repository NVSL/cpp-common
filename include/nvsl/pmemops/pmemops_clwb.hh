// -*- mode: c++; c-basic-offset: 2; -*-

/**
 * @file   pmemops_clwb.hh
 * @date   June 22, 2020
 * @brief  Persistent memory operations using clwb instruction
 */

#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <immintrin.h>
#include <mmintrin.h>
#include <ratio>
#include <sys/mman.h>
#include <xmmintrin.h>

#include "nvsl/pmemops/declarations.hh"
#include "nvsl/error.hh"

/** @brief Streaming copy 8B */
inline void streaming_wr_8B(void *dest, const void *src) {
  _mm_stream_si64((long long *)dest, *(long long *)src);
}

/** @brief Streaming copy 16B */
inline void streaming_wr_16B(void *dest, const void *src) {
  __m128i xmm0 = _mm_loadu_si128((__m128i *)src);

  _mm_stream_si128((__m128i *)dest, xmm0);
}

/** @brief Streaming copy 32B */
inline void streaming_wr_32B(void *dest, const void *src) {
  __m256i ymm0 = _mm256_loadu_si256((__m256i *)src);

  _mm256_stream_si256((__m256i *)dest, ymm0);
}

/** @brief Streaming copy 1 cachelines (64B) */
inline void streaming_wr_64B(void *dest, const void *src) {
  __m512i zmm0 = _mm512_loadu_si512((const __m512i *)src);

  _mm512_stream_si512((__m512i *)dest, zmm0);
}

/** @brief Streaming copy 2 cachelines (128B) */
inline void streaming_wr_128B(void *dest, const void *src) {
  __m512i zmm0 = _mm512_loadu_si512((const __m512i *)src);
  __m512i zmm1 = _mm512_loadu_si512((const __m512i *)src + 1);

  _mm512_stream_si512((__m512i *)dest, zmm0);
  _mm512_stream_si512((__m512i *)dest + 1, zmm1);
}

/** @brief Streaming copy 2 cachelines (128B) */
inline void streaming_wr_256B(void *dest, const void *src) {
  __m512i zmm0 = _mm512_loadu_si512((const __m512i *)src);
  __m512i zmm1 = _mm512_loadu_si512((const __m512i *)src + 1);
  __m512i zmm2 = _mm512_loadu_si512((const __m512i *)src + 2);
  __m512i zmm3 = _mm512_loadu_si512((const __m512i *)src + 3);

  _mm512_stream_si512((__m512i *)dest, zmm0);
  _mm512_stream_si512((__m512i *)dest + 1, zmm1);
  _mm512_stream_si512((__m512i *)dest + 2, zmm2);
  _mm512_stream_si512((__m512i *)dest + 3, zmm3);
}

inline void nvsl::PMemOpsClwb::streaming_wr(void *dest, const void *src,
                               size_t bytes) const {
  size_t remaining_bytes = bytes;
  size_t cur_off_bytes = 0;

  const auto src_bp = (const uint8_t *)src;
  auto dest_bp = (uint8_t *)dest;

  while (remaining_bytes > 0) {
    if (remaining_bytes >= 256) {
      streaming_wr_256B(dest_bp + cur_off_bytes, src_bp + cur_off_bytes);
      cur_off_bytes += 256;
      remaining_bytes -= 256;
    } else if (remaining_bytes >= 128) {
      streaming_wr_128B(dest_bp + cur_off_bytes, src_bp + cur_off_bytes);
      cur_off_bytes += 128;
      remaining_bytes -= 128;
    } else if (remaining_bytes >= 64) {
      streaming_wr_64B(dest_bp + cur_off_bytes, src_bp + cur_off_bytes);
      cur_off_bytes += 64;
      remaining_bytes -= 64;
    } else if (remaining_bytes >= 32) {
      streaming_wr_32B(dest_bp + cur_off_bytes, src_bp + cur_off_bytes);
      cur_off_bytes += 32;
      remaining_bytes -= 32;
    } else if (remaining_bytes >= 16) {
      streaming_wr_16B(dest_bp + cur_off_bytes, src_bp + cur_off_bytes);
      cur_off_bytes += 16;
      remaining_bytes -= 16;
    } else if (remaining_bytes >= 8) {
      streaming_wr_8B(dest_bp + cur_off_bytes, src_bp + cur_off_bytes);
      cur_off_bytes += 8;
      remaining_bytes -= 8;
    } else {
      assert(0);
    }
  }
}

inline void nvsl::PMemOpsClwb::persist(void *base, size_t size) const {
  this->flush(base, size);
  this->drain();
}

inline void nvsl::PMemOpsClwb::clwb(void *addr) const {
  _mm_clwb(addr);
}

inline void nvsl::PMemOpsClwb::flush(void *base, size_t size) const {
  uintptr_t uptr;

  /*
   * Loop through cache-line-size (typically 64B) aligned chunks
   * covering the given range.
   */
  for (uptr = (uintptr_t)base & ~(CL_SIZE - 1); uptr < (uintptr_t)base + size;
       uptr += CL_SIZE) {
#ifdef CLWB_AVAIL
    this->clwb((void *)uptr);
#else
    NVSL_ERROR("This version of libpuddles was built on a platform "
             "that doesn't support clwb");
#endif
  }
}

inline void nvsl::PMemOpsClwb::drain() const {
#ifdef SFENCE_AVAIL
  _mm_sfence();
#else
  NVSL_ERROR("This version of libpuddles was built on a platform "
           "that doesn't support sfence");
#endif
}

inline void nvsl::PMemOpsClwb::memcpy(void *dest, void *src, size_t size) const {
  DBGH(4) << "MEMCPY :: pmemdest " << (void *)(dest) << " src " << (void *)(src)
          << " len " << size << std::endl;

  this->memmove(dest, src, size);
}

inline void nvsl::PMemOpsClwb::memmove(void *dest, void *src, size_t size) const {
  std::memmove(dest, src, size);

  flush(dest, size);
  drain();
}

inline void nvsl::PMemOpsClwb::memset(void *base, char c, size_t size) const {
  DBGH(4) << "MEMSET :: start " << (void *)(base) << " size " << (void *)(size)
          << " char " << c << std::endl;

  std::memset(base, c, size);
  flush(base, size);
  drain();
}
