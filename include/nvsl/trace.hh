// -*- mode: c++; c-basic-offset: 2; -*-

/**
 * @file   trace.hh
 * @date   May 10, 2021
 * @brief  Brief description here
 */

#pragma once

#include <algorithm>
#include <linux/prctl.h>
#include <string>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

namespace nvsl {
  /** @brief Dump virtual memory mappings to a file */
  __attribute__((visibility(("default")))) inline void dump_maps() {}

  /** @brief Return current stack trace from gdb as std::string */
  static inline std::string get_stack_trace() {
    char pid_buf[30];
    sprintf(pid_buf, "%d", getpid());
    char name_buf[512];
    name_buf[readlink("/proc/self/exe", name_buf, 511)] = 0;
    prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY, 0, 0, 0);

    std::string gdb_cmd = "/usr/bin/gdb -batch -quiet -ex bt " +
                          std::string(name_buf) + " " + std::string(pid_buf) +
                          " 2>&1";

    FILE *child_f = popen(gdb_cmd.c_str(), "r");
    std::string output;

    if (child_f) {
      char c;
      while ((c = getc(child_f)) != EOF)
        output += c;
    }

    pclose(child_f);

    return output;
  }

  inline int get_stack_depth(void) {
    auto stack = get_stack_trace();
    return std::count(stack.begin(), stack.end(), '\n');
  }

  /** @brief Print current stack trace using gdb */
  static inline void print_trace(bool start_gdb = true) {
    char pid_buf[30];
    sprintf(pid_buf, "%d", getpid());
    char name_buf[512];
    name_buf[readlink("/proc/self/exe", name_buf, 511)] = 0;
    prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY, 0, 0, 0);
    int child_pid = fork();
    if (!child_pid) {
      dup2(2, 1); // redirect output to stderr - edit: unnecessary?
      if (start_gdb) {
        execl("/usr/bin/gdb", "gdb", "-quiet", "-ex", "bt", name_buf, pid_buf,
              NULL);
      } else {
        execl("/usr/bin/gdb", "gdb", "-batch", "-quiet", "-ex", "bt", name_buf,
              pid_buf, NULL);
      }
      abort(); /* If gdb failed to start */
    } else {
      waitpid(child_pid, NULL, 0);
    }
  }

} // namespace nvsl
