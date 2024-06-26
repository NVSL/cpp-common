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
#include <emmintrin.h>
#include <immintrin.h>
#include <mmintrin.h>
#include <ratio>
#include <sys/mman.h>
#include <xmmintrin.h>

#include "nvsl/error.hh"
#include "nvsl/pmemops/declarations.hh"

/** @brief Streaming copy 4B */
inline void streaming_wr_4B(void *dest, const void *src) {
  DBGH(4) << "Streaming write 4B: " << dest << " -> " << src << "\n";
  _mm_stream_si32((int *)dest, *(int *)src);
}

/** @brief Streaming copy 8B */
inline void streaming_wr_8B(void *dest, const void *src) {
  DBGH(4) << "Streaming write 8B: " << dest << " -> " << src << "\n";
  _mm_stream_si64((long long *)dest, *(long long *)src);
}

/** @brief Streaming copy 16B */
inline void streaming_wr_16B(void *dest, const void *src) {
  DBGH(4) << "Streaming write 16B: " << dest << " -> " << src << "\n";
  __m128i xmm0 = _mm_loadu_si128((__m128i *)src);

  _mm_stream_si128((__m128i *)dest, xmm0);
}

/** @brief Streaming copy 32B */
inline void streaming_wr_32B(void *dest, const void *src) {
  DBGH(4) << "Streaming write 32B: " << dest << " -> " << src << "\n";
#ifdef __AVX__
  __m256i ymm0 = _mm256_loadu_si256((__m256i *)src);

  _mm256_stream_si256((__m256i *)dest, ymm0);
#else
  assert(0 && "AVX512F function called on unsupported processor");
#endif
}

/** @brief Streaming copy 1 cachelines (64B) */
inline void streaming_wr_64B(void *dest, const void *src) {
  DBGH(4) << "Streaming write 64B: " << dest << " -> " << src << "\n";
#ifdef __AVX512F__
  __m512i zmm0 = _mm512_loadu_si512((const __m512i *)src);

  _mm512_stream_si512((__m512i *)dest, zmm0);
#else
  assert(0 && "AVX512F function called on unsupported processor");
#endif
}

/** @brief Streaming copy 2 cachelines (128B) */
inline void streaming_wr_128B(void *dest, const void *src) {
  DBGH(4) << "Streaming write 128B: " << dest << " -> " << src << "\n";
#ifdef __AVX512F__
  __m512i zmm0 = _mm512_loadu_si512((const __m512i *)src);
  __m512i zmm1 = _mm512_loadu_si512((const __m512i *)src + 1);

  _mm512_stream_si512((__m512i *)dest, zmm0);
  _mm512_stream_si512((__m512i *)dest + 1, zmm1);
#else
  assert(0 && "AVX512F function called on unsupported processor");
#endif
}

/** @brief Streaming copy 2 cachelines (128B) */
inline void streaming_wr_256B(void *dest, const void *src) {
  DBGH(4) << "Streaming write 256B: " << dest << " -> " << src << "\n";
#ifdef __AVX512F__
  __m512i zmm0 = _mm512_loadu_si512((const __m512i *)src);
  __m512i zmm1 = _mm512_loadu_si512((const __m512i *)src + 1);
  __m512i zmm2 = _mm512_loadu_si512((const __m512i *)src + 2);
  __m512i zmm3 = _mm512_loadu_si512((const __m512i *)src + 3);

  _mm512_stream_si512((__m512i *)dest, zmm0);
  _mm512_stream_si512((__m512i *)dest + 1, zmm1);
  _mm512_stream_si512((__m512i *)dest + 2, zmm2);
  _mm512_stream_si512((__m512i *)dest + 3, zmm3);
#else
  assert(0 && "AVX512F function called on unsupported processor");
#endif
}

inline void nvsl::PMemOpsClwb::streaming_wr(void *dest, const void *src,
                                            size_t bytes) const {
  size_t remaining_bytes = bytes;
  size_t cur_off_bytes = 0;

  const auto src_bp = (const uint8_t *)src;
  auto dest_bp = (uint8_t *)dest;

  while (remaining_bytes > 0) {
#ifdef __AVX512F__
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
    } else
#endif
#if __AVX__
        if (remaining_bytes >= 32) {
      streaming_wr_32B(dest_bp + cur_off_bytes, src_bp + cur_off_bytes);
      cur_off_bytes += 32;
      remaining_bytes -= 32;
    } else
#endif
        if (remaining_bytes >= 16) {
      streaming_wr_16B(dest_bp + cur_off_bytes, src_bp + cur_off_bytes);
      cur_off_bytes += 16;
      remaining_bytes -= 16;
    } else if (remaining_bytes >= 8) {
      streaming_wr_8B(dest_bp + cur_off_bytes, src_bp + cur_off_bytes);
      cur_off_bytes += 8;
      remaining_bytes -= 8;
    } else if (remaining_bytes >= 4) {
      streaming_wr_4B(dest_bp + cur_off_bytes, src_bp + cur_off_bytes);
      cur_off_bytes += 4;
      remaining_bytes -= 4;
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

inline void nvsl::PMemOpsClwb::clflush(void *addr) const {
  _mm_clflush(addr);
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

/** @brief Evict the given range from the cache */
inline void nvsl::PMemOpsClwb::evict(void *base, size_t size) const {
  uintptr_t uptr;

  /*
   * Loop through cache-line-size (typically 64B) aligned chunks
   * covering the given range.
   */
  for (uptr = (uintptr_t)base & ~(CL_SIZE - 1); uptr < (uintptr_t)base + size;
       uptr += CL_SIZE) {
    this->clflush((void *)uptr);
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

inline void nvsl::PMemOpsClwb::memcpy(void *dest, void *src,
                                      size_t size) const {
  DBGH(4) << "MEMCPY :: pmemdest " << (void *)(dest) << " src " << (void *)(src)
          << " len " << size << std::endl;

  this->memmove(dest, src, size);
}

inline void nvsl::PMemOpsClwb::memmove(void *dest, void *src,
                                       size_t size) const {
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
