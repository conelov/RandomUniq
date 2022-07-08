//
// Created by dym on 01.07.22.
//

#pragma once

#include <cassert>


namespace urand::plot::util {

class RangeScaler final {
public:
  RangeScaler(double minOld, double maxOld, double minNew, double maxNew)
      : diffOld{(assert(maxOld >= minOld), maxOld - minOld)}
      , minOld_{minOld}
      , diffNew_{(assert(maxNew >= minNew), maxNew - minNew)}
      , minNew_{minNew} {
  }


  double operator()(double x) const {
    return (diffNew_ * (x - minOld_) / diffOld) + minNew_;
  }

private:
  double const diffOld, minOld_, diffNew_, minNew_;
};
}// namespace urand::plot::util