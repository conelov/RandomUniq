//
// Created by dym on 03.07.2022.
//

#pragma once

#include <type_traits>


namespace urand::util {

template<typename T_>
class Range final {
  static_assert(std::is_arithmetic_v<T>);

public:
  using value_type = T_;


  class WrapNum final {
    static_assert(std::is_arithmetic_v<T>);

  public:
    using value_type = T_;

  public:
    value_type get() const {
      return val_;
    }


    operator value_type() const {
      return get();
    }


    WrapNum& set(value_type v) {
      val_ = v;
      return *this;
    }


    WrapNum& operator=(value_type v) {
      return set(v);
    }

  private:
    WrapNum(value_type v = {})
        : val_{v} {
    }

  private:
    value_type val_;

    friend Range;
  };

public:
  Range(value_type min = {}, value_type max = {})
      : min_{min}
      , max_{max} {
  }

private:
  WrapNum min_, max_;
};
}// namespace urand::util