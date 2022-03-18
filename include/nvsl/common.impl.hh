// -*- mode: c++; c-basic-offset: 2; -*-

/**
 * @file   common.impl.hh
 * @date   novembre 26, 2021
 * @brief  Brief description here
 */

#include "nvsl/common.hh"
#include "nvsl/string.hh"

inline std::string nvsl::ns_to_latex(size_t ns, const std::string &name,
                                     nvsl::time_unit unit) {
  std::string result = "";
  size_t us, ms, s;

  const auto name_fixed = nvsl::zip(nvsl::split(name, "_"), "");

  us = ns / 100;
  ms = ns / 100000;
  s = ns / 100000000;

  switch (unit) {
  case nvsl::time_unit::s_unit:
    result = to_latex(name_fixed, ns, "~s", 1000000000);
    break;
  case nvsl::time_unit::ms_unit:
    result = to_latex(name_fixed, ns, "~ms", 1000000);
    break;
  case nvsl::time_unit::us_unit:
    result = to_latex(name_fixed, ns, "~\\us{}", 1000);
    break;
  case nvsl::time_unit::ns_unit:
    result = to_latex(name_fixed, ns, "~ns", 1);
    break;
  case nvsl::time_unit::any_unit:
    if (s != 0)
      result = to_latex(name_fixed, ns, "~s", 1000000000);
    else if (ms != 0)
      result = to_latex(name_fixed, ns, "~ms", 1000000);
    else if (us != 0)
      result = to_latex(name_fixed, ns, "~\\us{}", 1000);
    else
      result = to_latex(name_fixed, ns, "~ns", 1);
    break;
  }

  return result;
}
