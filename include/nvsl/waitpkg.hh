#include <immintrin.h>
#include <x86intrin.h>

#pragma once

namespace nvsl {
  enum class TpauseType {
    DeepSleep = 0,  // C0.2
    LightSleep = 1, // C0.1
  };

  static inline void tpause(size_t wait_cycles, TpauseType type) {
    const uint64_t current_tsc = _rdtsc();
    const uint64_t final_tsc = current_tsc + wait_cycles;

    _tpause(static_cast<int>(type), final_tsc);
  }

  static inline void wait(void *addr, size_t wait_cycles, TpauseType type) {
    _umonitor(addr);

    while (true) {
      const uint64_t current_tsc = _rdtsc();
      const uint64_t final_tsc = current_tsc + wait_cycles;

      const auto expired = _umwait(static_cast<int>(type), final_tsc);

      if (!expired) break;
    };
  }
} // namespace nvsl