//
// Created by dym on 29.06.22.
//

#pragma once

#include "randomUniq/util/RandomDevice.hpp"
#include <numeric>

namespace urand {

template<typename T_>
class UniformIntDistribution {
  static_assert(std::is_integral_v<T_>);

public:
  using value_type = T_;

public:
  UniformIntDistribution(value_type min = std::numeric_limits<value_type>::min(), value_type max = std::numeric_limits<value_type>::max())
      : range_(min, max) {
  }

  value_type operator()() {
    return range_(util::RandomDevice<std::mt19937>::get());
  }

private:
  std::uniform_int_distribution<value_type> range_;
};
}// namespace urand