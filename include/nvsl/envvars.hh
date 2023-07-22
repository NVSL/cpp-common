// -*- mode: c++; c-basic-offset: 2; -*-

#pragma once

/**
 * @file   envvars.hh
 * @date   January 21, 2021
 * @brief  Declare environment variables and access their values
 */

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
#include <string>
#endif

#define NVSL_DECL_ENV(name) \
  static const char *name##_ENV __attribute__((unused)) = (char *)(#name)

NVSL_DECL_ENV(NVSL_NO_STACKTRACE);
NVSL_DECL_ENV(NVSL_LOG_WILDCARD);
NVSL_DECL_ENV(NVSL_GEN_STATS);

/**
 * @brief Looks up a boolean like env var
 * @details Behavior:
 * 1. Env variable missing -> return false
 * 2. 0 -> return false
 * 1. 1 -> return true
 */
static inline bool get_env_val(const char *var) {
  bool result = false;

  const char *val = getenv(var);
  if (val != NULL) {
    if (strncmp(val, "1", 1) == 0) {
      result = true;
    }
  }

  return result;
}

/**
 * @brief Looks up a string value from env var
 * @details Behavior:
 * 1. Env variable missing -> empty string
 * @param[in] var Environment variable's name
 * @param[in] def Default value to return if unset
 */
static inline char *get_env_str(const char *var, const char *def) {
  char *result = (char *)def;

  char *val = getenv(var);
  if (val != NULL) {
    result = val;
  }

  return result;
}

#ifdef __cplusplus
#include <string>

/**
 * @brief Looks up a boolean like env var
 * @details Behavior:
 * 1. Env variable missing -> return false
 * 2. 0 -> return false
 * 1. 1 -> return true
 */
static inline bool get_env_val(const std::string var) {
  return get_env_val(var.c_str());
}

static inline std::string get_env_str(const std::string var,
                                      const std::string def = "") {
  return std::string(get_env_str(var.c_str(), def.c_str()));
}
#endif
