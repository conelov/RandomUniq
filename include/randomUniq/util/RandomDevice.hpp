//
// Created by dym on 29.06.22.
//

#pragma once

#include <chrono>
#include <random>


namespace urand::util {

template<typename TGen_>
class RandomDevice {
public:
  using TGen = TGen_;

public:
  RandomDevice(RandomDevice const&) = delete;
  RandomDevice(RandomDevice&&)      = delete;

  static TGen& get() {
    return RandomDevice::instance().mt_;
  }

private:
  RandomDevice() {
    std::random_device rd;
    if (rd.entropy()) {
      std::seed_seq seedSeq{rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd()};
      mt_.seed(seedSeq);
    } else {
      mt_.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }
  }


  static RandomDevice& instance() {
    static RandomDevice s;
    return s;
  }

private:
  TGen mt_;
};
}// namespace urand::util
