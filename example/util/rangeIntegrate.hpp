//
// Created by dym on 01.07.22.
//

#pragma once

#include "example/util/minmaxException.hpp"
#include "example/util/RangeScaler.hpp"
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/range/operations.hpp>
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
  minmaxException(toMin, toMax);

  const std::size_t count = source.size();
  using Pair = std::pair<double, typename std::remove_cvref_t<decltype(source.begin()->second)>>;
  for (auto const [key, value] : source
      | ranges::views::transform(
        [count, scaler = RangeScaler(source.begin()->first, std::prev(source.end())->first, toMin, toMax * count)](Pair pair) {
          return Pair{scaler(pair.first) / static_cast<double>(count), pair.second};
        })
      | ranges::views::chunk_by([](Pair lhs, Pair rhs) {
          return lhs.first == rhs.first;
        })
      | ranges::views::transform([](auto&&range) {
          return Pair{
            ranges::front(range).first, ranges::accumulate(std::forward<decltype(range)>(range), 0, [](auto lhs, Pair pair) {
              return lhs + pair.second;
            })};
        })) {
    std::invoke(std::forward<decltype(f)>(f), key, value);
  }
}
}// namespace urand::plot::util