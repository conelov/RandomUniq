//
// Created by dym on 01.07.22.
//

#pragma once

#include "plot/util/RangeScaler.hpp"
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/range/operations.hpp>
#include <range/v3/view/chunk_by.hpp>
#include <range/v3/view/transform.hpp>


namespace urand::plot::util {

auto rangeScaleView(double fromMin, double fromMax, double toMin, double toMax) {
  return ranges::views::transform([scaler = RangeScaler{fromMin, fromMax, toMin, toMax}](auto pair) {
    return std::make_pair(scaler(pair.first), pair.second);
  })
    | ranges::views::chunk_by([](auto lhs, auto rhs) {
        return lhs.first == rhs.first;
      })
    | ranges::views::transform([](auto&& range) {
        return decltype(ranges::front(range)){
          ranges::front(range).first, ranges::accumulate(std::forward<decltype(range)>(range), 0, [](auto lhs, auto pair) {
            return lhs + pair.second;
          })};
      });
}
}// namespace urand::plot::util