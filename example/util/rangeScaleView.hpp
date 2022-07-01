//
// Created by dym on 01.07.22.
//

#pragma once

#include "example/util/minmaxException.hpp"
#include "example/util/RangeScaler.hpp"
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/range/operations.hpp>
#include <range/v3/view/chunk_by.hpp>
#include <range/v3/view/transform.hpp>


namespace urand::plot::util {

auto rangeScaleView(auto fromMin, auto fromMax, std::size_t count, auto toMin, auto toMax) {
  minmaxException(fromMin, fromMax);
  minmaxException(toMin, toMax);

  return ranges::views::transform(
           [count, scaler = RangeScaler(fromMin, fromMax, toMin, toMax * count)](auto pair) {
             return std::pair<double, typename std::remove_cvref_t<decltype(pair.second)>>{
               scaler(pair.first) / static_cast<double>(count), pair.second};
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