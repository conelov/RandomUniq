//
// Created by dym on 01.07.22.
//

#include "example/util/RangeScaler.hpp"
#include "example/util/minmaxException.hpp"


namespace urand::plot::util {

RangeScaler::RangeScaler(double minOld, double maxOld, double minNew, double maxNew)
    : minOld_(minOld)
    , maxOld_(maxOld)
    , newDif_(maxNew - minNew) {
  urand::plot::util::minmaxException(minOld, maxOld);
  urand::plot::util::minmaxException(minNew, maxNew);
}


double RangeScaler::operator()(double x) const {
  return newDif_ * (x - minOld_) / (maxOld_ - minOld_);
}
}// namespace urand::plot::util