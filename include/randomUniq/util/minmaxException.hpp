//
// Created by dym on 01.07.22.
//

#pragma once

#include <stdexcept>
#include <utility>


namespace urand::util {

void minmaxException(auto&& lhs, auto&& rhs) {
  if (std::forward<decltype(lhs)>(lhs) > std::forward<decltype(rhs)>(rhs)) {
    throw std::invalid_argument("lhs > rhs");
  }
}
}// namespace urand::util