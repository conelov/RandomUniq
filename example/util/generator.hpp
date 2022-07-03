//
// Created by dym on 02.07.2022.
//

#pragma once

#include "randomUniq/UniformIntDistributionUniq.hpp"
#include "randomUniq/util/RandomDevice.hpp"
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/iota.hpp>


namespace urand::plot::util {

template<urand::UniformIntDistributionUniqGenType GenType>
auto uniformIntDistributionUniqAtType(auto l, auto r) {
  return [gen = urand::UniformIntDistributionUniq<std::size_t, GenType>(l, r)]() mutable {
    return gen();
  };
}


template<std::size_t mid>
auto uniformIntDistributionLinearMid(auto l, auto r) {
  static_assert(mid);
  assert(r >= l);
  return [gen = std::uniform_int_distribution<std::size_t>(l, r)]() mutable {
    return ranges::accumulate(ranges::views::iota(0u, mid), 0, [&](const auto lhs, const auto rhs) {
      return lhs + gen(urand::util::RandomDevice<std::mt19937>::get());
    }) / mid;
  };
}
}// namespace urand::plot::util