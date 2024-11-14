// -*- mode: c++; c-basic-offset: 2; -*-

/**
 * @file   utils.hh
 * @date   mars 31, 2022
 * @brief  Misc functions
 */

#pragma once

#include <sstream>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

#include "string.hh"

#ifndef MAP_SHARED_VALIDATE
#define MAP_SHARED_VALIDATE \
  0x03 // Works on x86, might not be compatible with
       // other platforms
#endif

#ifndef MAP_SYNC
#define MAP_SYNC 0x080000
#endif

namespace nvsl {
  inline std::string mlock_to_str(void *addr, size_t len) {
    const auto params_v = {S(addr), S(len)};
    const auto params = zip(params_v, ", ");

    return "mlock(" + params + ")";
  }

  inline std::string mmap_to_str(void *addr, size_t len, int prot, int flags,
                                 int fd, off_t off) {
    std::vector<std::string> flags_v;
    if (flags & MAP_SHARED) {
      flags_v.push_back("MAP_SHARED");
    }
    if (flags & MAP_SHARED_VALIDATE) {
      flags_v.push_back("MAP_SHARED_VALIDATE");
    }
    if (flags & MAP_PRIVATE) {
      flags_v.push_back("MAP_PRIVATE");
    }
    if (flags & MAP_ANONYMOUS) {
      flags_v.push_back("MAP_ANONYMOUS");
    }
    if (flags & MAP_FIXED) {
      flags_v.push_back("MAP_FIXED");
    }
    if (flags & MAP_FIXED_NOREPLACE) {
      flags_v.push_back("MAP_FIXED_NOREPLACE");
    }
    if (flags & MAP_SYNC) {
      flags_v.push_back("MAP_SYNC");
    }

    const auto flags_str = nvsl::zip(flags_v, " | ");

    std::vector<std::string> prot_v;
    if (prot & PROT_READ) {
      prot_v.push_back("PROT_READ");
    }
    if (prot & PROT_WRITE) {
      prot_v.push_back("PROT_WRITE");
    }
    if (prot & PROT_EXEC) {
      prot_v.push_back("PROT_EXEC");
    }

    const auto prot_str = nvsl::zip(prot_v, " | ");

    const auto params_v = {S(addr), S(len), prot_str, flags_str, S(fd), S(off)};
    const auto params = zip(params_v, ", ");

    return "mmap(" + params + ")";
  }

  inline std::string fd_to_fname(const int fd) {
    std::string result = "";

    /* Read the file name for the fd */
    if (fd != -1) {
      /* Read the file name for the fd */
      auto fd_path = "/proc/self/fd/" + std::to_string(fd);
      constexpr size_t path_max = 4096;
      char buf[path_max];
      const ssize_t rl_ret = readlink(fd_path.c_str(), buf, path_max);

      if (rl_ret == -1) {
        DBGH(1) << "Readline for fd " << fd
                << " failed. Readlink path: " << fd_path << std::endl;
        DBGH(1) << PSTR() << std::endl;
      } else {
        buf[rl_ret] = 0;

        DBGH(3) << "Mmaped fd " << fd << " to path " << S(buf) << std::endl;

        result = S(buf);
      }
    }

    return result;
  }

  constexpr inline size_t round_down(size_t val, size_t factor) {
    return (val / factor) * factor;
  }

  constexpr inline size_t round_up(size_t val, size_t factor) {
    return ((val + factor - 1) / factor) * factor;
  }

  /**
   * @brief Checks the memory for errors
   * @param[in] vram_ptr Pointer to the begining of the region to check
   * @param bytes Bytes to scan
   * @warn This function will overwrite memory. Any existing data will be lost
   *
   * @return Number of 8byte words with errors
   */
  inline size_t memcheck(void *vram_ptr, size_t bytes) {
    const auto u64_ptr = reinterpret_cast<uint64_t *>(vram_ptr);

    std::cerr << "Memsetting...";
    memset(vram_ptr, 0xFF, bytes);
    std::cerr << "done\n";

    size_t err_cnt = 0;
    std::cerr << "Reading memory...";
    for (size_t i = 0; i < bytes / sizeof(uint64_t); i++) {
      if (u64_ptr[i] != -1UL) {
        err_cnt++;
      }
    }
    std::cerr << "done. Total errors = " << err_cnt << "\n";

    return err_cnt;
  }

  /**
   * @brief Get the CPU utilization
   * @return CPU utilization as a float between 0 and 1
   */
  inline float get_cpu_utilization() {
    std::ifstream file("/proc/stat");
    if (!file.is_open()) {
        std::cerr << "Unable to open /proc/stat file.\n";
        return -1.0;
    }

    std::string line;
    std::getline(file, line);  // Read the first line (aggregate CPU data)
    file.close();

    // Parse the first line
    std::istringstream ss(line);
    std::string cpu;
    long user, nice, system, idle, iowait, irq, softirq, steal;
    ss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

    // Calculate total and idle time
    long idleTime = idle + iowait;
    long totalTime = user + nice + system + idle + iowait + irq + softirq + steal;

    // Wait for a short interval to calculate difference in usage
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Re-open and re-read /proc/stat to get new data
    file.open("/proc/stat");
    if (!file.is_open()) {
        std::cerr << "Unable to open /proc/stat file.\n";
        return -1.0;
    }
    std::getline(file, line);
    file.close();

    // Parse the line again
    std::istringstream ss2(line);
    ss2 >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

    // Recalculate total and idle time
    long idleTime2 = idle + iowait;
    long totalTime2 = user + nice + system + idle + iowait + irq + softirq + steal;

    // Calculate utilization
    float utilization = 1.0 - (float)(idleTime2 - idleTime) / (totalTime2 - totalTime);
    return utilization;
}
} // namespace nvsl
