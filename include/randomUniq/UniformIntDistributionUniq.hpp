#pragma once

#include "randomUniq/util/RandomDevice.hpp"
#include <cstdint>
#include <gsl/util>
#include <range/v3/view/counted.hpp>
#include <vector>


namespace urand {

enum class UniformIntDistributionUniqGenType : std::uint8_t {
  NonLinearEqualChanceRange,
  LinearDoobleGen
};


template<typename T_, UniformIntDistributionUniqGenType GenType_ = UniformIntDistributionUniqGenType::LinearDoobleGen>
class UniformIntDistributionUniq final {
  static_assert(std::is_integral_v<T_>);

public:
  using value_type              = T_;
  static constexpr auto GenType = GenType_;

public:
  UniformIntDistributionUniq(value_type min = std::numeric_limits<value_type>::min(),
    value_type                          max = std::numeric_limits<value_type>::max())
      : totalCounter_(max - min + 1) {
    if (min > max) {
      throw std::invalid_argument("min > max");
    }
    ranges_.push_back({min, max});
  }


  value_type get() {
    if (!totalCounter_) {
      throw std::runtime_error("no contained number");
    }
    const auto _       = gsl::finally([this] { --totalCounter_; });
    const auto itRange = [this] {
      if constexpr (GenType == UniformIntDistributionUniqGenType::LinearDoobleGen) {
        double       res    = 0;
        const double random = std::uniform_real_distribution<double>(0, 1)(util::RandomDevice<std::mt19937>::get());
        for (auto it = ranges_.begin(); it != ranges_.end(); ++it) {
          res += it->chance(totalCounter_);
          if (res >= random) {
            return it;
          }
        }
        return std::prev(ranges_.end());
      } else {
        return std::next(ranges_.begin(), std::uniform_int_distribution<int>(0, ranges_.size() - 1)(util::RandomDevice<std::mt19937>::get()));
      }
    }();

    if (itRange->min == itRange->max) {
      const auto result = itRange->min;
      ranges_.erase(itRange);
      return result;
    }
    value_type const result = std::uniform_int_distribution<value_type>(itRange->min, itRange->max)(util::RandomDevice<std::mt19937>::get());

    if (itRange->min == result) {
      return itRange->min++;
    }
    if (itRange->max == result) {
      return itRange->max--;
    }

    {
      Range newRange{itRange->min, result - 1};
      itRange->min = result + 1;
      ranges_.push_back(std::move(newRange));
    }

    return result;
  }


  value_type operator()() {
    return get();
  }


  std::size_t empty() const noexcept {
    return ranges_.empty();
  }


  operator bool() const noexcept {
    return empty();
  }

private:
  class Range {
  public:
    value_type min;
    value_type max;

  public:
    std::size_t size() const noexcept {
      return max - min + 1;
    }


    double chance(std::size_t total) const noexcept {
      return 1. / total * size();
    }
  };

private:
  std::vector<Range> ranges_;
  std::size_t        totalCounter_;

private:
  //  auto rangeRandom() const noexcept {
  //    if constexpr (GenType == UniformIntDistributionUniqGenType::LinearDoobleGen) {
  //      double       res    = 0;
  //      const double random = std::uniform_real_distribution<double>(0, 1)(util::RandomDevice<std::mt19937>::get());
  //      for (auto it = ranges_.begin(); it != ranges_.end(); ++it) {
  //        res += it->chance(totalCounter_);
  //        if (res >= random) {
  //          return it;
  //        }
  //      }
  //      return std::prev(ranges_.end());
  //    } else {
  //      return std::next(ranges_.begin(), std::uniform_int_distribution<int>(0, ranges_.size() - 1)(util::RandomDevice<std::mt19937>::get()));
  //    }
  //  }
};
}// namespace urand