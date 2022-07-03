//
// Created by dym on 01.07.22.
//

#include "example/util/RangeScaler.hpp"
#include <cassert>


namespace urand::plot::util {

RangeScaler::RangeScaler(double minOld, double maxOld, double minNew, double maxNew)
    : minOld_(minOld)
    , maxOld_(maxOld)
    , newDif_(maxNew - minNew) {
  assert(maxOld >= minOld);
  assert(maxNew >= minNew);
}


double RangeScaler::operator()(double x) const {
  return newDif_ * (x - minOld_) / (maxOld_ - minOld_);
}
}// namespace urand::plot::util