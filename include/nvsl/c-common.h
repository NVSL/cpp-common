#pragma once

#ifdef __cplusplus
#error "Do not include c-common.h from C++ source"
#else
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "nvsl/defs.h"

static int lvl_num = 5;

void nvsl_fatal(const char *fmt, ...);
int nvsl_is_log_enabled(int check_lvl);
int nvsl_log(const int lvl, const char *fmt, ...);

void nvsl_fatal(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  fprintf(stderr, "FATAL: ");
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");

  va_end(args);
  exit(1);
}

int nvsl_is_log_enabled(int check_lvl) {
  if (lvl_num > 4) {
    const char *lvl = getenv(NVSL_LOG_LEVEL_ENV);

    if (lvl != NULL) {
      lvl_num = atoi(lvl);

      if (lvl_num > 4 || lvl_num < 0) {
        nvsl_fatal("NVSL_LOG_LEVEL is not in range [0,4]");
      }
    } else {
      lvl_num = 0;
    }
  }

  return (check_lvl <= lvl_num);
}

int nvsl_log(const int lvl, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  int res = 0;

  if (nvsl_is_log_enabled(lvl)) {
    res = vfprintf(stderr, fmt, args);
  }

  va_end(args);

  return res;
}

#endif // __cplusplus
