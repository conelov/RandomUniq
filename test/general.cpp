//
// Created by dym on 29.06.22.
//

#include "randomUniq/UniformIntDistributionUniq.hpp"
#include <gtest/gtest.h>
#include <range/v3/algorithm/equal.hpp>
#include <range/v3/algorithm/generate.hpp>
#include <range/v3/algorithm/sort.hpp>
#include <range/v3/view/iota.hpp>


namespace {

template<typename T, std::size_t S>
std::ostream& operator<<(std::ostream& os, std::array<T, S> const& array) {
  for (auto const i : array) {
    os << i << ' ';
  }
  return os;
}
}// namespace


TEST(general, smoke) {
  constexpr std::size_t      totalSize = 10'000;
  std::array<std::size_t, totalSize> array_gen;
  using namespace ranges;
  generate(array_gen, [gen = urand::UniformIntDistributionUniq<std::size_t>(0, totalSize - 1)]() mutable {
    return gen();
  });
  sort(array_gen);

  for (auto const i : array_gen) {
    std::cout << i << ' ';
  }
  ASSERT_TRUE(equal(views::iota(0u, totalSize), array_gen));
}
