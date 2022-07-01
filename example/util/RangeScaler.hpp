//
// Created by dym on 01.07.22.
//

#pragma once


namespace urand::plot::util {

class RangeScaler final {
public:
  RangeScaler(double minOld, double maxOld, double minNew, double maxNew);

  double operator()(double x) const;

private:
  const double minOld_, maxOld_, newDif_;
};
}// namespace urand::plot::util