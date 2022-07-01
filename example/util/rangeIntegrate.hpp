//
// Created by dym on 01.07.22.
//

#pragma once

#include "example/util/minmaxException.hpp"
#include "example/util/RangeScaler.hpp"
#include <range/v3/view/chunk_by.hpp>
#include <range/v3/view/common.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/for_each.hpp>
#include <range/v3/view/group_by.hpp>
#include <range/v3/view/linear_distribute.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/unique.hpp>
#include <range/v3/view/zip.hpp>


namespace urand::plot::util {

void rangeIntegrate(auto const& source, double toMin, double toMax, std::size_t toCount, auto&& f) {
  auto rangeStepAtCount = [](auto f, auto l, std::size_t count) {
    minmaxException(f, l);
    return (l - f + 1.) / count;
  };

  using Pair = decltype(*source.begin());

  auto toRange = ranges::views::linear_distribute(toMin + rangeStepAtCount(toMin, toMax, toCount), toMax, toCount)
    | ranges::views::drop(1)
    | ranges::views::common;
  auto const fromRange = source
    | ranges::views::transform(
      [scaler = RangeScaler(source.begin()->first, std::prev(source.end())->first, toMin, toMax)](Pair pair) {
        return Pair{scaler(pair.first), pair.second};
      })
    | ranges::views::chunk_by([](Pair const lhs, Pair const rhs) {
        return lhs.first == rhs.second;
      });
  //    auto itTo = toRange.begin();
  //    auto itFrom = fromRange.begin();
  //    while(itFrom != fromRange.end() && itTo != toRange.end()) {
  //      std::find(itFrom, fromRange.end(), [](auto const pair) {
  //        return true;
  //      });
  //    }
}
}// namespace urand::plot::util