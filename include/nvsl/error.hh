// -*- mode: c++; c-basic-offset: 2; -*-

/**
 * @file   error.hh
 * @date   March 27, 2021
 * @brief  Brief description here
 */

#pragma once

#include <cstring>
#include <iostream>
#include <optional>

#include "nvsl/common.hh"
#include "nvsl/envvars.hh"
#include "nvsl/trace.hh"

static inline std::string print_stuff__(std::string msg, std::string dec) {
  std::cout << msg << " -- " << dec << std::endl;
  return msg;
}

/** @brief Convert errno to exception */
#define PERROR_EXCEPTION(errcode, msg)                         \
  ([]() { DBGE << msg << std::endl; }(), nvsl::dump_maps(),    \
   nvsl::print_trace(),                                        \
   std::system_error(errcode, std::generic_category()))

/** @brief Throw exception with msg if val is NULL */
#define ASSERT_NON_NULL(val, msg)      \
  do {                                 \
    DBGE << msg << std::endl;          \
    dump_maps();                       \
    print_trace();                     \
    if ((val) == NULL) {               \
      throw std::runtime_error((msg)); \
    }                                  \
  } while (0);

#ifdef RELEASE
/** @brief Throw exception with msg if val is NULL */
#define NVSL_ERROR(msg)         \
  do {                        \
    DBGE << msg << std::endl; \
    exit(1);                  \
  } while (0);
#elif defined(BUILDING_PUDDLED) || defined(BUILDING_LIBCOMMON)
/** @brief Throw exception with msg if val is NULL */
#define NVSL_ERROR(msg) NVSL_ERROR_CLEAN(msg)
#else
/** @brief Throw exception with msg if val is NULL */
#define NVSL_ERROR(msg)                                  \
  do {                                                   \
    DBGE << msg << std::endl;                            \
    if (not get_env_val(NVSL_NO_STACKTRACE_ENV)) {       \
      nvsl::dump_maps();                                 \
      nvsl::print_trace();                               \
    }                                                    \
    exit(1);                                             \
  } while (0);
#endif // RELEASE

/** @brief Exit with no debugging info */
#define NVSL_ERROR_CLEAN(msg)   \
  do {                        \
    DBGE << msg << std::endl; \
    exit(1);                  \
  } while (0);

/** @brief Assert a condition w/ msg and generate backtrace on fail */
#ifndef NDEBUG
#define NVSL_ASSERT(cond, msg)                                     \
  if (!(cond)) [[unlikely]] {                                      \
    DBGE << __FILE__ << ":" << __LINE__ << " Assertion `" << #cond \
         << "' failed: " << msg << std::endl;                      \
    if (not get_env_val(NVSL_NO_STACKTRACE_ENV)) {                 \
      nvsl::print_trace();                                         \
    }                                                              \
    exit(1);                                                       \
  }
#else
#define LP_ASSERT(cond, msg) \
  if (true) {                \
    (void)(cond);            \
  }
#endif

/** @brief Convert errno to std::string */
static inline std::string PSTR() {
  return std::string(__FILE__) + ":" + std::to_string(__LINE__) + " " +
         std::string(strerror(errno));
}

using std::make_optional;
using std::optional;
