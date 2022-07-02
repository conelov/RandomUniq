//
// Created by dym on 01.07.22.
//

#include "example/util/generator.hpp"
#include "example/util/rangeScaleView.hpp"
#include <gtest/gtest.h>
#include <range/v3/algorithm/equal.hpp>
#include <range/v3/algorithm/sort.hpp>
#include <range/v3/view/single.hpp>
#include <range/v3/view/zip.hpp>


namespace {
namespace _ {

template<auto comp = std::equal_to{}>
struct PairComp final {
  bool operator()(auto lhs, auto rhs) const {
    return comp(lhs.first, rhs.first) && comp(lhs.second, rhs.second);
  }
};
}// namespace _
}// namespace


TEST(PROJECT_NAME_RandomUniq, rangeScaleView_throw) {
  using namespace ranges;
  ASSERT_THROW((views::zip(views::single(0), views::single(0))
                 | urand::plot::util::rangeScaleView(0, 0, 0, 1, 0)),
    std::invalid_argument);
}

TEST(PROJECT_NAME_RandomUniq, rangeScaleView) {
  {
    const std::map<std::size_t, std::size_t> source{
      {10u, 1u},
      {11u, 2u}};
    const std::map<double, std::size_t> mod{
      {0, 1u},
      {1, 2u}};
    ASSERT_TRUE(ranges::equal(source | urand::plot::util::rangeScaleView(source.begin()->first, std::prev(source.end())->first, source.size(), 0, 1),
      mod,
      _::PairComp{}));
  }
  {
    const std::map<std::size_t, std::size_t> source{
      {10u, 1u},
      {12u, 2u},
      {20u, 3}};
    const std::map<double, std::size_t> mod{
      {0, 1u},
      {2, 2u},
      {10, 3u}};
    ASSERT_TRUE(ranges::equal(source | urand::plot::util::rangeScaleView(source.begin()->first, std::prev(source.end())->first, source.size(), 0, 10),
      mod,
      _::PairComp{}));
  }
}

TEST(PROJECT_NAME_RandomUniq, generator) {
  using namespace urand::plot::util;
  constexpr std::size_t          count = 999;
  std::array<std::size_t, count> source;
  for (auto gen = uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::LinearDoobleGen>(0, count - 1);
       auto& i : source) {
    i = gen();
  }
  ranges::sort(source);
  ASSERT_TRUE(ranges::equal(ranges::views::iota(0u, count), source));
}