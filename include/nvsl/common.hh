// -*- mode: c++; c-basic-offset: 2; -*-

/**
 * @file   common.hh
 * @date   December 25, 2020
 * @brief  Common macros, constexpr and function definitions
 */

#pragma once

#include "nvsl/constants.hh"
#include "nvsl/envvars.hh"
#include <chrono>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#include <fnmatch.h>

#define NVSL_NODISCARD [[nodiscard]]
#define NVSL_UNUSED __attribute__((unused))
#define NVSL_EXPORT __attribute__((visibility("default")))
#define NVSL_NOINLINE __attribute__((noinline))

#define NVSL_BEGIN_IGNORE_WPEDANTIC \
  _Pragma("GCC diagnostic push")  \
      _Pragma("GCC diagnostic ignored \"-Wpedantic\"")

#define NVSL_END_IGNORE_WPEDANTIC _Pragma("GCC diagnostic pop")

// Do magic! Creates a unique name using the line number
#define NVSL_LINE_NAME(prefix) NVSL_JOIN(prefix, __LINE__)
#define NVSL_JOIN(symbol1, symbol2) NVSL_DO_JOIN(symbol1, symbol2)
#define NVSL_DO_JOIN(symbol1, symbol2) symbol1##symbol2

/** @brief Wrapper for std::lock_guard */
#define NVSL_GUARD(mtx) \
  std::lock_guard<std::mutex> NVSL_LINE_NAME(nvsl_macro_lock_guard)((mtx))

/** @brief Wrapper for ptr to std::lock_guard */
#define NVSL_GUARD_PTR(mtx) \
  std::lock_guard<std::mutex> NVSL_LINE_NAME(nvsl_macro_lock_guard)((*mtx))

static const char *NVSL_LOG_LEVEL_ENV NVSL_UNUSED =
    (char *)("NVSL_LOG_LEVEL");

static std::ofstream nullstream;

extern std::ofstream log_st;
#ifdef RELEASE
#define DBG 0 && std::cerr
#elif defined(NVSL_SIMPLIFIED_TERM_IO)
#define DBG log_st
#else
#define DBG (std::cerr)
#endif

namespace nvsl {
  /** @brief Match a pattern using wildcar */
  inline bool wildcard(const std::string &pat, const std::string &str) {
    return not fnmatch(pat.c_str(), str.c_str(), 0);
  }

  static inline bool is_log_enabled(int level) {
    uint8_t log_lvl = 0;

    const char *val = std::getenv(NVSL_LOG_LEVEL_ENV);
    if (val != nullptr) {
      const std::string val_str = std::string(val);

      try {
        log_lvl = std::stoul(val_str, nullptr, 10);

        if (log_lvl > 4) {
          throw std::out_of_range("Valid values: [0-4]");
        }
      } catch (std::out_of_range &e) {
        std::cerr << "LP FATAL: " << NVSL_LOG_LEVEL_ENV
                  << " is out of range. Valid values: [0-4]" << std::endl;
        std::exit(1);
      } catch (std::invalid_argument &e) {
        std::cerr << "LP FATAL: "
                  << "Unable to parse " << NVSL_LOG_LEVEL_ENV
                  << " env variable." << std::endl;
        std::exit(1);
      }
    }

    if (level <= log_lvl) {
      return true;
    } else {
      return false;
    }
  }

  static inline bool is_caller_enabled(const std::string &caller) {
    bool result = true;

    const char *val = std::getenv(NVSL_LOG_WILDCARD_ENV);
    if (val != nullptr) {
      const std::string val_str = std::string(val);

      result = wildcard(val_str, caller);
    }

    return result;
  }
} // namespace nvsl

inline std::string nvsl_cur_time_str() {
  std::stringstream ss;

  auto now = std::chrono::system_clock::now();
  auto timeNow = std::chrono::system_clock::to_time_t(now);
  auto time_str = std::string(std::ctime(&timeNow));

  ss << time_str.substr(0, time_str.length() - 1) << " [" << getpid() << "]";

  return ss.str();
}

/* Log to stderr with a decorator */
struct DBGH {
  bool enabled;
  DBGH(const DBGH &obj) { this->enabled = obj.enabled; }

#ifndef RELEASE
  DBGH(uint8_t lvl, const char *caller = __builtin_FUNCTION()) {
#else
  DBGH(uint8_t lvl) {
#endif

#ifndef RELEASE
    this->enabled =
        nvsl::is_log_enabled(lvl) && nvsl::is_caller_enabled(caller);

    if (this->enabled) [[likely]] {
#ifdef NVSL_SIMPLIFIED_TERM_IO
      DBG << lp_cur_time_str() << " | ";
#else
      DBG << "[\x1B[1m" << std::setw(20) << std::string(caller) << "()"
          << "\x1B[0m"
          << "]:" << (int)lvl << " ";
#endif // NVSL_SIMPLIFIED_TERM_IO
    }
#endif // !RELEASE
  }

  template <typename T>
  friend const DBGH &operator<<(const DBGH &dbgh, const T &obj);

  friend const DBGH &operator<<(const DBGH &s,
                                std::ostream &(*f)(std::ostream &));
  friend const DBGH &operator<<(const DBGH &s, std::ostream &(*f)(std::ios &));
  friend const DBGH &operator<<(const DBGH &s,
                                std::ostream &(*f)(std::ios_base &));
};

template <typename T>
inline const DBGH &operator<<(const DBGH &dbgh, const T &obj) {
  if (dbgh.enabled) {
    DBG << obj;
  }

  return dbgh;
}

inline const DBGH &operator<<(const DBGH &s,
                              std::ostream &(*f)(std::ostream &)) {
#ifndef RELEASE
  if (s.enabled) f(DBG);
#endif
  return s;
}

inline const DBGH &operator<<(const DBGH &s, std::ostream &(*f)(std::ios &)) {
#ifndef RELEASE
  if (s.enabled) f(DBG);
#endif
  return s;
}

inline const DBGH &operator<<(const DBGH &s,
                              std::ostream &(*f)(std::ios_base &)) {
#ifndef RELEASE
  if (s.enabled) f(DBG);
#endif
  return s;
}

#ifdef RELEASE

#define DBG 0 && std::cerr
#define DBGF(s)
#define DBGW (std::cerr << "Warning: ")
#define DBGE (std::cerr << "Error: ")

#else

/* Log current function to stderr with a decorator */
#define DBGF(l) \
  (DBGH(l) << std::string(__FUNCTION__) << "() called." << std::endl);

/* Log warning to stderr with decorator */
#ifdef NVSL_SIMPLIFIED_TERM_IO
#define DBGW (DBGH(0) << "Warning: ")
#else
#define DBGW                                                               \
  (DBG << "[\x1B[1m" << std::setw(20) << std::string(__FUNCTION__) << "()" \
       << "\x1B[0m"                                                        \
       << "]\x1B[95m WARNING: \x1B[0m")
#endif

/* Log error to stderr with decorator */
#ifdef NVSL_SIMPLIFIED_TERM_IO
#define DBGE (DBG << "ERROR: \x1B[0m")
#else
#define DBGE                                                                \
  (DBG << "[\x1B[31m" << std::setw(20) << std::string(__FUNCTION__) << "()" \
       << "\x1B[0m"                                                         \
       << "]\x1B[95m ERROR: \x1B[0m")
#endif
#endif

namespace nvsl {
  /** @brief Converts pointer to a string */
  static inline std::string ptr_to_string(const void *addr) {
    std::stringstream ss;

    ss << addr;

    return ss.str();
  }

  /** @brief Align ptr to the next cache line (64 bytes) */
  inline constexpr void *align_cl(auto *ptr) {
    uint8_t *bptr = (uint8_t *)ptr;
    auto result = (void *)(((uint64_t)(bptr + 63) >> 6) << 6);

    DBGH(4) << "Aligned " << (void *)ptr << " -> " << (void *)result
            << std::endl;

    return result;
  }

  /** @brief Align a ptr to 4KiB page boundary */
  inline constexpr void *align_4kb(auto *ptr) {
    uint8_t *bptr = (uint8_t *)ptr;
    auto result = (void *)(((uint64_t)(bptr + (4 * KiB - 1)) >> 12) << 12);

    DBGH(4) << "Aligned " << (void *)ptr << " -> " << (void *)result
            << std::endl;

    return result;
  }

  /** @brief Align a ptr to 2MiB page boundary */
  inline constexpr void *align_2mb(auto *ptr) {
    uint8_t *bptr = (uint8_t *)ptr;
    auto result = (void *)(((uint64_t)(bptr + (2 * MiB - 1)) >> 21) << 21);

    DBGH(4) << "Aligned " << (void *)ptr << " -> " << (void *)result
            << std::endl;

    return result;
  }

  /** @brief Rounds bytes to the multiple of mult greater than it */
  inline constexpr auto round_bytes(auto bytes, auto mult) -> decltype(bytes) {
    if (mult > bytes)
      throw std::runtime_error("Mult " + std::to_string(mult) +
                               " is greater than bytes " +
                               std::to_string(bytes));

    size_t alloc_bytes = bytes;
    if (bytes % mult != 0) {
      alloc_bytes = ((bytes / mult) + 1) * mult;
    }
    return alloc_bytes;
  }

  /** @brief return a string with hex repr of a pointer */
  static inline std::string ptr_to_hexstr(void *ptr) {
    std::stringstream ss;
    ss << (void *)ptr;
    std::string result = ss.str();
    return ss.str();
  }

  /* NOTE: First template is filled automatically */

  /** @brief Wrapper for reinterpret_cast to void* */
  template <typename I>
  static inline void *P(I arg) {
    return reinterpret_cast<void *>(arg);
  }

  /** @brief Wrapper for reinterpret_cast */
  template <typename O, typename I>
  static inline O RCast(I arg) {
    return reinterpret_cast<O>(arg);
  }

  /** @brief Wrapper for static_cast */
  template <typename O, typename I>
  static inline O SCast(I arg) {
    return static_cast<O>(arg);
  }

  /** @brief Wrapper for dynamic_cast */
  template <typename O, typename I>
  static inline O DCast(I arg) {
    return dynamic_cast<O>(arg);
  }

  /** @brief Get page number from virtual memory address */
  auto page_num(auto ptr) -> decltype(ptr) {
    using retType = decltype(ptr);

    auto ptr_ul = RCast<uint64_t>(ptr);
    return RCast<retType>(ptr_ul >> 12);
  }

  /**
   * @brief hexdump style dump of a memory region
   *
   * @warn Any use of buf_to_hexstr should be in a #ifndef RELEASE. DBGH()
   * optimizations do remove calls to buf_to_hexstr
   */
  static inline std::string __attribute__((__const__))
  buf_to_hexstr(const char *buf, size_t bytes) {
    std::stringstream result;

    result << std::hex;

    size_t cur_byte = 0;
    std::stringstream val;
    while (cur_byte < bytes) {
      result << "0x" << std::hex << std::setfill('0') << std::setw(8)
             << ((uint64_t)(buf + cur_byte) >> 4) << 4 << "  ";
      for (size_t i = 0; i < 16; i++) {
        const size_t idx = cur_byte + i;

        if (i % 4 == 0 && i != 0) {
          result << " ";
        }

        uint32_t elem = static_cast<uint8_t>(buf[idx]);
        result << std::hex << std::setfill('0') << std::setw(2)
               << std::uppercase << elem;

        if (buf[idx] >= ' ' and buf[idx] <= '~') {
          val << buf[idx];
        } else {
          val << ".";
        }
      }
      cur_byte += 16;

      if (cur_byte < bytes) {
        result << "    " << val.str() << std::endl;
        std::stringstream().swap(val);
      }
    }

    return result.str();
  }

  inline bool is_pid_running(pid_t pid) {
    while (waitpid(-1, 0, WNOHANG) > 0) {
      // Wait for defunct....
    }

    if (0 == kill(pid, 0)) return 1; // Process exists

    return 0;
  }

  template <typename A, typename B, typename C>
  inline uint64_t rebase_ptr(A old_base, B new_base, C ptr) {
    uint64_t result =
        (uint64_t)(new_base) + ((uint64_t)(ptr) - (uint64_t)(old_base));
    DBGH(4) << "old_base: " << (void *)(old_base)
            << " new_base: " << (void *)new_base
            << " off: " << ((uint64_t)(ptr) - (uint64_t)(old_base))
            << " result: " << (void *)(result) << std::endl;

    return result;
  }

  inline std::string ns_to_hr(size_t ns_total) {
    std::stringstream ss;

    const size_t s = ns_total / (1000000000);
    const size_t ms = (ns_total - s * 1000000000) / (1000000);
    const size_t us = (ns_total - s * 1000000000 - ms * 1000000) / (1000);
    const size_t ns =
        (ns_total - s * 1000000000 - ms * 1000000 - us * 1000) / (1000);

    ss << s << "s " << ms << "ms " << us << "us " << ns << "ns";

    return ss.str();
  }

  template <typename T>
  std::string to_latex(const std::string &name, T val,
                       const std::string &suffix, size_t div_factor = 1) {
    double scale = 0.1;
    double div_val = (int)((val / (double)div_factor) / scale) * scale;
    std::stringstream ss;

    ss << "\\newcommand{\\" << name << "}{" << std::fixed
       << std::setprecision(1) << div_val << suffix << "}";

    return ss.str();
  }

  std::string ns_to_latex(size_t ns, const std::string &name,
                          time_unit unit = time_unit::any_unit);

  inline std::string uint64_to_base64(uint64_t val) {
    std::string result = "12345678901";

    for (size_t i = 0; i < 11; i++) {
      uint8_t chunk = (val >> (i * 6)) & ((1 << 6) - 1);
      result[i] = (char)(chunk + ' ');
    }
    return result;
  }

  inline uint64_t base64_to_uint64(std::string val) {
    uint64_t result = 0;

    for (size_t i = 0; i < 11; i++) {
      result += (val[i] - ' ') << (i * 6);
    }

    return result;
  }
} // namespace nvsl
