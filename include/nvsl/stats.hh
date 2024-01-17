// -*- mode: c++; c-basic-offset: 2; -*-

#pragma once

/**
 * @file   stats.hh
 * @date   novembre 17, 2021
 * @brief  Class to collect stats, generate summary and latex code
 */

#include "nvsl/error.hh"
#include "nvsl/string.hh"

#include <cassert>
#include <cfloat>
#include <concepts>
#include <cstddef>
#include <filesystem>
#include <ios>
#include <map>
#include <mutex>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "nvsl/envvars.hh"

NVSL_DECL_ENV(NVSL_STAT_DUMP_PERIOD);

namespace nvsl {
  static size_t STAT_DUMP_PERIOD = 16 * 1024UL; // # Samples
#ifdef NVSL_PERIODIC_STAT_DUMP
  constexpr bool periodic_stat_dump = true;
#else
  constexpr bool periodic_stat_dump = false;
#endif

  template <class T>
  concept Integral = std::is_integral<T>::value;

  template <typename T, typename I>
  concept Averageable = Integral<I> && requires(T a, T b, I c) { (a + b) / c; };

  /** @brief Class to track all the stats in a process */
  class StatsBase;
  class StatsCollection {
  public:
    static std::vector<StatsBase *> *stats;

    ~StatsCollection();
    static void dump_stats();
  };

  /** @brief Base class for \ref Stats */
  class StatsBase {
  protected:
    std::string stat_name;
    std::string stat_desc;
    size_t sample_count = 0;

  public:
    StatsBase(bool reg) {
      if (reg) {
#ifdef NVSL_ENABLE_COLLECTION_REGISTRATION
        StatsCollection::stats->push_back(this);
#endif
      }

      const auto stat_dump_prd_def = std::to_string(STAT_DUMP_PERIOD);
      const auto stat_dump_prd_str =
          get_env_str(NVSL_STAT_DUMP_PERIOD_ENV, stat_dump_prd_def);
      STAT_DUMP_PERIOD = std::stoul(stat_dump_prd_str);
    };

    void init(const std::string &name, const std::string &desc) {
      this->stat_name = name;
      this->stat_desc = desc;
    }

    virtual double avg() const { return 0; }
    virtual std::string str() const { return ""; }
    virtual std::string latex(const std::string &prefix = "") const {
      (void)prefix;
      return "";
    }
    virtual void reset() {}
    virtual std::string dump_file_name() {
      const auto basename = nvsl::replace(stat_name, " ", "_");
      return basename + ".nvsl-stats";
    }

    /**
     * @brief Notify the stat base about a sample
     * @details Used for stuff like dumping stats occasionally
     */
    void notify_sample() {
      if constexpr (periodic_stat_dump) {
        if (sample_count++ > STAT_DUMP_PERIOD) {
          const std::filesystem::path STAT_DUMP_DIR("/tmp/");
          const auto ofstream_flags = std::ios::out | std::ios::trunc;
          std::ofstream dump_file(STAT_DUMP_DIR / this->dump_file_name(),
                                  ofstream_flags);

          dump_file << "name: \"" << this->stat_name << "\"" << std::endl
                    << "desc: \"" << this->stat_desc << "\"" << std::endl
                    << "---" << std::endl
                    << this->str() << std::endl;
        }
      }
    }
  };

  /**
   * @brief Stat to measure freq of elements with a name and a description
   * @details
   */
  template <typename T = size_t>
  class StatsFreq : public StatsBase {
  private:
    size_t bucket_cnt;
    T bucket_min, bucket_max, bucket_sz, sum;
    size_t *counts, underflow_cnt, overflow_cnt;
    std::mutex lock;

  public:
    StatsFreq(bool reg = true) : StatsBase(reg){};

    ~StatsFreq() { delete[] counts; }

    /**
     * @brief Initialize the stats' buckets
     * @param name Name of the stat
     * @param desc Description of the stat
     * @param bucket_cnt Number of buckets
     * @param bucket_min Minimum value of the bucket
     * @param bucket_max Maximum value of the bucket
     */
    void init(const std::string &name, const std::string &desc,
              size_t bucket_cnt, T bucket_min, T bucket_max) {
#if defined(DBGE)
      NVSL_ASSERT(bucket_cnt != 0, "Bucket size cannot be zero");
      NVSL_ASSERT(bucket_max > bucket_min,
                  "Bucket max cannot be smaller than bucket min");
#else
      assert(bucket_cnt != 0 && "Bucket size cannot be zero");
      assert(bucket_max > bucket_min &&
             "Bucket max cannot be smaller than bucket min");
#endif // DBGE

      StatsBase::init(name, desc);

      this->bucket_cnt = bucket_cnt;
      this->bucket_min = bucket_min;
      this->bucket_max = bucket_max;
      this->bucket_sz = (bucket_max - bucket_min) / bucket_cnt;
      this->underflow_cnt = 0;
      this->overflow_cnt = 0;
      this->sum = 0;

      this->counts = new size_t[bucket_cnt];
      memset(counts, 0, sizeof(counts[0]) * bucket_cnt);
    }

    /**
     * @brief Add a value to the frequency map
     * @param[in] val Value to sample
     * @param[in] count=1 Number of times to add this value to the map
     */
    void add(T val, size_t count = 1) {
      std::lock_guard<std::mutex> _auto_mutex(lock);

      if (val < bucket_min) {
        underflow_cnt++;
      } else if (val >= bucket_max) {
        overflow_cnt++;
      } else {
        size_t bucket_idx = (val - bucket_min) / bucket_sz;
        counts[bucket_idx]++;
      }

      this->sum += val;
      this->notify_sample();
    }

    /**
     * @brief Get the total number of samples
     * @return Total number of samples
     */
    size_t total() const {
      return underflow_cnt + overflow_cnt +
             std::accumulate(counts, counts + bucket_cnt, 0);
    }

    /**
     * @brief Get the number of samples in a bucket
     * @param[in] bucket Bucket index
     * @return Number of samples in the bucket
     */
    size_t bucket_count(size_t bucket) const { return counts[bucket]; }

    /**
     * @brief Get the number of samples in overflow and underflow buckets
     * @param[in] underflow_cnt Return the number of samples in underflow bucket
     * @param[in] overflow_cnt Return the number of samples in overflow bucket
     * @return Number of samples in overflow and underflow buckets (whichever is
     * enabled)
     */
    size_t uoflow_count(bool underflow_cnt, bool overflow_cnt) const {
      return (underflow_cnt ? this->underflow_cnt : 0) +
             (overflow_cnt ? this->overflow_cnt : 0);
    }

    /**
     * @brief Generate a string representation of the frequency map
     */
    std::string str() const {
      std::stringstream ss;
      ss << stat_name + ".bucket_count: " << bucket_cnt << "\t# " + stat_desc
         << "\n"
         << stat_name + ".bucket_min: " << bucket_min << "\t# " + stat_desc
         << "\n"
         << stat_name + ".bucket_max: " << bucket_max << "\t# " + stat_desc
         << "\n"
         << stat_name + ".bucket_size: " << bucket_sz << "\t# " + stat_desc
         << "\n"
         << stat_name + ".mean: " << sum / total() << "\t# " + stat_desc << "\n"
         << stat_name + ".underflow_count: " << underflow_cnt
         << "\t# " + stat_desc << "\n"
         << stat_name + ".overflow_count: " << overflow_cnt
         << "\t# " + stat_desc << "\n";

      for (size_t i = 0; i < bucket_cnt; i++) {
        const T bkt_lo = bucket_min + i * bucket_sz;
        const T bkt_hi = bucket_min + (i + 1) * bucket_sz;
        ss << stat_name + ".bucket[" << bkt_lo << ":" << bkt_hi
           << "]: " << counts[i] << std::endl;
      }

      return ss.str();
    }
  };

  /** @brief Counts operations */
  class Counter : public StatsBase {
  private:
    size_t counter;

  public:
    Counter(bool reg = true) : StatsBase(reg), counter(0){};

    void init(const std::string &name, const std::string &desc) {
      StatsBase::init(name, desc);
    }

    Counter &operator++() {
      this->counter++;

      this->notify_sample();
      return *this;
    }

    Counter operator++(int) {
      Counter result = *this;
      ++this->counter;

      this->notify_sample();
      return result;
    }

    size_t value() const { return this->counter; }

    void reset() override { this->counter = 0; }

    /** @brief Get the string representation of the stat */
    std::string str() const override {
      std::stringstream ss;
      ss << StatsBase::stat_name << " = " << value();

      if (stat_desc != "") {
        ss << " # " << stat_desc;
      }

      return ss.str();
    }
  };

  /** @brief Represents a single stat with a name and a description */
  class StatsScalar : public StatsBase {
  private:
    double total;
    size_t count;
    double max_v = -DBL_MAX;
    double min_v = DBL_MAX;

    bool is_time;
    time_unit unit;

  public:
    StatsScalar(bool reg = true) : StatsBase(reg), total(0), count(0){};

    void init(const std::string &name, const std::string &desc,
              bool is_time = false, time_unit unit = time_unit::any_unit) {
      StatsBase::init(name, desc);
      this->is_time = is_time;
      this->unit = unit;
    }

    friend StatsScalar operator+(StatsScalar lhs, const auto rhs) {
      lhs.max_v = std::max((double)rhs, lhs.max_v);
      lhs.min_v = std::min((double)rhs, lhs.min_v);
      lhs.total += rhs;
      lhs.count++;

      lhs.notify_sample();
      return lhs;
    }

    StatsScalar &operator+=(const auto rhs) {
      this->max_v = std::max((double)rhs, this->max_v);
      this->min_v = std::min((double)rhs, this->min_v);
      this->total += rhs;
      this->count++;

      this->notify_sample();
      return *this;
    }

    void reset() override {
      this->count = 0;
      this->total = 0;
      this->max_v = -DBL_MAX;
      this->min_v = DBL_MAX;
    }

    /** @brief Get the average value per operation */
    double avg() const override {
      if (count == 0) {
        return 0;
      } else {
        return total / (double)count;
      }
    }

    /** @brief Get the string representation of the stat */
    std::string str() const override {
      std::stringstream ss;
      ss << stat_name << " = " << avg();

      if (this->is_time) {
        ss << " (" << ns_to_hr(this->avg()) << ")";
      }

      if (stat_desc != "") {
        ss << " # " << stat_desc;
      }

      return ss.str();
    }

    std::string latex(const std::string &prefix = "") const override {
      std::string name = "stat" + prefix + this->stat_name;
      std::string result = "";
      size_t ns, us, ms, s;

      name = nvsl::zip(nvsl::split(name, "_"), "");

      ns = this->avg();
      us = this->avg() / 1000;
      ms = this->avg() / 1000000;
      s = this->avg() / 1000000000;

      switch (unit) {
      case nvsl::time_unit::s_unit:
        result = to_latex(name, ns, "~s", 1000000000);
        break;
      case nvsl::time_unit::ms_unit:
        result = to_latex(name, ns, "~ms", 1000000);
        break;
      case nvsl::time_unit::us_unit:
        result = to_latex(name, ns, "~\\us{}", 1000);
        break;
      case nvsl::time_unit::ns_unit:
        result = to_latex(name, ns, "~ns", 1);
        break;
      case nvsl::time_unit::any_unit:
        if (s != 0)
          result = to_latex(name, ns, "~s", 1000000000);
        else if (ms != 0)
          result = to_latex(name, ns, "~ms", 1000000);
        else if (us != 0)
          result = to_latex(name, ns, "~\\us{}", 1000);
        else
          result = to_latex(name, ns, "~ns", 1);
        break;
      }

      result = result + " % total ops = " + std::to_string(this->count);

      return result;
    };

    double max() const {
      if (this->max_v == -DBL_MAX) {
        return 0;
      } else {
        return this->max_v;
      }
    }

    double min() const {
      if (this->min_v == DBL_MAX) {
        return 0;
      } else {
        return this->min_v;
      }
    }

    size_t counts() const { return count; }
  };

  /** @brief Represents a vector of stats, each with a name */
  class StatsNamedVector : public StatsBase {
  private:
    std::map<std::string, StatsScalar> vec;
    time_unit unit;

  public:
    StatsNamedVector(bool reg = true) : StatsBase(reg){};

    void init(const std::string &name, const std::string &desc,
              time_unit unit = time_unit::any_unit) {
      StatsBase::init(name, desc);
      this->unit = unit;
    }

    StatsScalar &operator[](const std::string &memb_name) {
      const auto memb = this->vec.find(memb_name);
      const auto exists = memb != this->vec.end();

      if (not exists) {
        vec.emplace(std::make_pair(memb_name, false));
        vec[memb_name].init(memb_name, "", false, this->unit);
      }

      return vec[memb_name];
    }

    /** @brief Get the string representation of the stat */
    std::string str() const override {
      std::stringstream ss;

      for (const auto &[k, v] : this->vec) {
        ss << this->stat_name << "." << k << " = " << v.avg() << std::endl;
      }

      return ss.str();
    }

    std::string latex(const std::string &prefix = "") const override {
      std::stringstream ss;

      for (const auto &[k, v] : this->vec) {
        ss << v.latex(prefix + this->stat_name) << std::endl;
      }

      return ss.str();
    }
  };

  inline StatsCollection::~StatsCollection() {
    dump_stats();
  }

  inline void StatsCollection::dump_stats() {
    if (get_env_val(NVSL_GEN_STATS_ENV)) {
      std::cout << std::endl
                << "==== " << StatsCollection::stats->size()
                << " Stats ====" << std::endl;
      for (const auto stat : *StatsCollection::stats) {
        fprintf(stdout, "%s\n", stat->str().c_str());
        fprintf(stderr, "%s\n", stat->latex().c_str());
      }
    }
  }
} // namespace nvsl
