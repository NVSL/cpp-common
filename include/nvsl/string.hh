// -*- mode: c++; c-basic-offset: 2; -*-

#pragma once

/**
 * @file   string.hh
 * @date   Sep 23, 2021
 * @brief  Usefult string functions. Mostly resemble python's
 */

#include "nvsl/error.hh"
#include <string>
#include <vector>

namespace nvsl {
  /** @brief Split string using a delimeter into a vector of strings */
  inline auto split(const std::string &str, const std::string &delim,
                    size_t assert_length = UINT64_MAX) {
    std::vector<std::string> result;
    size_t prev = 0, pos = 0;

    do {
      pos = str.find(delim, prev);

      if (pos == std::string::npos) pos = str.length();

      std::string token = str.substr(prev, pos - prev);

      if (!token.empty()) result.push_back(token);

      prev = pos + delim.length();
    } while (pos < str.length() && prev < str.length());

    if (assert_length != UINT64_MAX) {
      NVSL_ASSERT(result.size() == assert_length, "Not enough tokens");
    }

    return result;
  }

  /** @brief Concat all the elements of a string vector into a stingle string */
  inline auto zip(const std::vector<std::string> arr,
                  const std::string join_str) {
    std::string result = "";

    size_t iter = 0;
    for (const auto &tok : arr) {
      result += tok;

      if (iter++ != arr.size() - 1) result += join_str;
    }

    return result;
  }

  /** @brief Checks if a string is suffix of another string */
  inline auto is_suffix(const std::string &suffix, const std::string &str) {
    auto mismatch =
        std::mismatch(suffix.rbegin(), suffix.rend(), str.rbegin()).first;
    return mismatch == suffix.rend();
  }

  /** @brief Checks if a string is prefix of another string */
  inline auto is_prefix(const std::string &prefix, const std::string &str) {
    auto mismatch =
        std::mismatch(prefix.begin(), prefix.end(), str.begin()).first;
    return mismatch == prefix.end();
  }

  template <typename T>
  inline const std::string S(T val) { return std::to_string(val); }

  template <>
  inline const std::string S(void* val) { return std::to_string((size_t)val); }

  template <>
  inline const std::string S(const char *c) { return std::string(c); }

  template <>
  inline const std::string S(char *c) { return std::string(c); }
  
  inline std::string ltrim(const std::string &str) {
    const auto start = str.find_first_not_of(" \t\n");

    if (start == std::string::npos) return str;

    return str.substr(start, str.size() - start);
  }

  inline std::string rtrim(const std::string &str) {
    const auto end = str.find_last_not_of(" \t\n");

    if (end == std::string::npos) return str;

    return str.substr(0, end + 1);
  }

  inline std::string trim(const std::string &str) { return ltrim(rtrim(str)); }
} // namespace nvsl
