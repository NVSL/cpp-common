// -*- mode: c++; c-basic-offset: 2; -*-

#pragma once

/**
 * @file   stats.hh
 * @date   novembre 17, 2021
 * @brief  Class to collect stats, generate summary and latex code
 */

#include "nvsl/string.hh"
#include "nvsl/error.hh"

#include <concepts>
#include <cstddef>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace nvsl {
  template <class T>
  concept Integral = std::is_integral<T>::value;

  template <typename T, typename I>
  concept Averageable = Integral<I> && requires(T a, T b, I c) {
    (a + b) / c;
  };

  /** @brief Class to track all the stats in a process */
  class StatsBase;
  class StatsCollection {
  public:
    static std::vector<StatsBase *> stats;

    ~StatsCollection();
  };

  /** @brief Base class for \ref Stats */
  class StatsBase {
  protected:
    std::string stat_name;
    std::string stat_desc;

  public:
    StatsBase(bool reg) {
      if (reg) {
#ifdef NVSL_ENABLE_COLLECTION_REGISTRATION
        StatsCollection::stats.push_back(this);
#endif
      }
    };

    void init(const std::string &name, const std::string &desc) {
      this->stat_name = name;
      printf("Stat name = %s\n", this->stat_name.c_str());
      this->stat_desc = desc;
    }

    virtual double avg() const { return 0; }
    virtual std::string str() const { return ""; }
    virtual std::string latex(const std::string &prefix = "") const {
      (void)prefix;
      return "";
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
    T bucket_min, bucket_max, bucket_sz;
    size_t *counts, underflow_cnt, overflow_cnt;

  public:
    StatsFreq(bool reg = true) : StatsBase(reg) {};

    ~StatsFreq() {
      delete[] counts;
    }

    /**
     * @brief Initialize the stats' buckets
     * @param name Name of the stat
     * @param desc Description of the stat
     * @param bucket_cnt Number of buckets
     * @param bucket_min Minimum value of the bucket
     * @param bucket_max Maximum value of the bucket
     * @param bucket_sz Size of the bucket
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
#endif  // DBGE

      StatsBase::init(name, desc);
      
      this->bucket_cnt = bucket_cnt;
      this->bucket_min = bucket_min;
      this->bucket_max = bucket_max;
      this->bucket_sz = (bucket_max-bucket_min)/bucket_cnt;
      this->underflow_cnt = 0;
      this->overflow_cnt = 0;

      this->counts = new size_t[bucket_cnt];
      memset(counts, 0, sizeof(counts[0])*bucket_cnt);
    }

    /**
     * @brief Add a value to the frequency map
     * @param[in] val Value to sample
     * @param[in] count=1 Number of times to add this value to the map
     */
    void add(T val, size_t count = 1) {
      if (val < bucket_min) {
        underflow_cnt++;
      } else if (val >= bucket_max) {
        overflow_cnt++;
      } else {
        size_t bucket_idx = (val-bucket_min)/bucket_sz;
        counts[bucket_idx]++;
      }      
    }

    /**
     * @brief Get the total number of samples
     * @return Total number of samples
     */
    size_t total() const {
      return underflow_cnt + overflow_cnt +
             std::accumulate(counts, counts+bucket_cnt, 0);
    }

    /**
     * @brief Get the number of samples in a bucket
     * @param[in] bucket Bucket index
     * @return Number of samples in the bucket
     */
    size_t bucket_count(size_t bucket) const {
      return counts[bucket];
    }

    /**
     * @brief Get the number of samples in overflow and underflow buckets
     * @param[in] underflow_cnt Return the number of samples in underflow bucket
     * @param[in] overflow_cnt Return the number of samples in overflow bucket
     * @return Number of samples in overflow and underflow buckets (whichever is enabled)
     */
    size_t uoflow_count(bool underflow_cnt, bool overflow_cnt) const {
      return underflow_cnt ? this->underflow_cnt : 0 +
             overflow_cnt ? this->overflow_cnt : 0;
    }

    /**
     * @brief Generate a string representation of the frequency map
     */
    std::string str() const {
      std::stringstream ss;
      ss << stat_name + ".bucket_count: " << bucket_cnt       << "\t# " + stat_desc << "\n"
         << stat_name + ".bucket_min: " << bucket_min         << "\t# " + stat_desc << "\n"
         << stat_name + ".bucket_max: " << bucket_max           << "\t# " + stat_desc << "\n"
         << stat_name + ".bucket_size: " << bucket_sz         << "\t# " + stat_desc << "\n"
         << stat_name + ".underflow_count: " << underflow_cnt << "\t# " + stat_desc << "\n"
         << stat_name + ".overflow_count: " << overflow_cnt   << "\t# " + stat_desc << "\n";

      for (size_t i = 0; i < bucket_cnt; i++) {
        ss << stat_name + ".bucket[" << i << "]: " << counts[i] << std::endl;
      }

      return ss.str();      
    }
  };

  /** @brief Counts operations */
  class Counter : public StatsBase {
  private:
    size_t counter;
  public:
    Counter(bool reg = true) : StatsBase(reg), counter(0) {
      printf("Counter constructed\n");
    };

    void init(const std::string &name, const std::string &desc) {
      StatsBase::init(name, desc);
    }
    
    Counter& operator++() {
      this->counter++;
      return *this;
    }

    Counter operator++(int) {
      Counter result = *this;
      ++this->counter;

      return result;
    }

    size_t value() const {
      return this->counter;
    }

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
      lhs.total += rhs;
      lhs.count++;
      return lhs;
    }

    StatsScalar &operator+=(const auto rhs) {
      this->total += rhs;
      this->count++;
      return *this;
    }

    /** @brief Get the average value per operation */
    double avg() const override { return total / (double)count; }

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
    if (get_env_val(NVSL_GEN_STATS_ENV)) {
      std::cout << std::endl << "==== Stats ====" << std::endl;
      for (const auto stat : StatsCollection::stats) {
        std::cout << stat->str() << std::endl;
        std::cerr << stat->latex() << std::endl;
      }
    }
  }
} // namespace nvsl
