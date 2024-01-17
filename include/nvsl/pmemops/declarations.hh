// -*- mode: c++; c-basic-offset: 2; -*-

/**
 * @file   definitions.hh
 * @date   mars 29, 2022
 * @brief  All declarations for the pmemops class
 */

#pragma once

#include <cassert>
#include <unistd.h>

#ifndef NVSL_PMEMOPS_MASTER_FILE
#error Do not include this file directly. Include "pmemops.hh" instead.
#endif

namespace nvsl {
  class PMemOps {
  public:
    static const size_t CL_SIZE = 64;

    /** @brief Flush and drain in a single call */
    virtual void persist(void *base, size_t size) const = 0;

    /** @brief Perform writes directly to the memory skipping caches */
    virtual void streaming_wr(void *dest, const void *src,
                              size_t bytes) const = 0;

    /**
     * @brief Order flushes before from flushes after this call
     * @details Usually calls a fence
     */
    virtual void drain() const = 0;

    /**
     * @brief Persistent version of memcpy
     * Warn: Current implementation always flushes and drains after a
     * move
     */
    virtual void memcpy(void *dest, void *src, size_t size) const = 0;

    /**
     * @brief Persistent version of memmove
     * Warn: Current implementation always flushes and drains after a
     * move
     */
    virtual void memmove(void *dest, void *src, size_t size) const = 0;
    virtual void memset(void *base, char c, size_t size) const = 0;

    virtual void flush(void *base, size_t size) const = 0;
  };

  class PMemOpsClwb : public PMemOps {
  private:
    /** @brief Clwb on a single addr, to flush a range use flush() */
    void clwb(void *addr) const;

  public:
    void flush(void *base, size_t size) const;
    void persist(void *base, size_t size) const;
    void drain() const;
    void memcpy(void *dest, void *src, size_t size) const;
    void memmove(void *dest, void *src, size_t size) const;
    void memset(void *base, char c, size_t size) const;
    void streaming_wr(void *dest, const void *src, size_t bytes) const;
  };

  class PMemOpsMsync : public PMemOps {
  private:
    /** @brief Clflusopt on single addr, to flush range, use flush() */
    void clwb(void *addr) const;

  public:
    void flush(void *base, size_t size) const;
    void persist(void *base, size_t size) const;
    void drain() const;
    void memcpy(void *dest, void *src, size_t size) const;
    void memmove(void *dest, void *src, size_t size) const;
    void memset(void *base, char c, size_t size) const;
    void streaming_wr(void *dest, const void *src, size_t bytes) const {
      (void)dest;
      (void)src;
      (void)bytes;
      assert(0 && "unimplemented");
    }
  };

  class PMemOpsClflushOpt : public PMemOps {
  private:
    void clflush_opt(void *addr) const;

  public:
    void flush(void *base, size_t size) const;
    void persist(void *base, size_t size) const;
    void drain() const;
    void memcpy(void *dest, void *src, size_t size) const;
    void memmove(void *dest, void *src, size_t size) const;
    void memset(void *base, char c, size_t size) const;
    void streaming_wr(void *dest, const void *src, size_t bytes) const {
      (void)dest;
      (void)src;
      (void)bytes;
      assert(0 && "unimplemented");
    }
  };

  class PMemOpsNoPersist : public PMemOps {
    void flush(void *base, size_t size) const;
    void persist(void *base, size_t size) const;
    void drain() const;
    void memcpy(void *dest, void *src, size_t size) const;
    void memmove(void *dest, void *src, size_t size) const;
    void memset(void *base, char c, size_t size) const;
    void streaming_wr(void *dest, const void *src, size_t bytes) const {
      (void)dest;
      (void)src;
      (void)bytes;
      assert(0 && "unimplemented");
    }
  };
} // namespace nvsl
