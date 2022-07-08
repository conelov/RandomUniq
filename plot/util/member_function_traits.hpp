//
// Created by dym on 08.07.22.
//

#pragma once

#include <cstdint>


namespace urand::plot::util {

// https://stackoverflow.com/a/28105160/18626156
template<typename>
struct member_function_traits;


template<typename Return, typename Object, typename... Args>
struct member_function_traits<Return (Object::*)(Args...)> {
  typedef Return  return_type;
  typedef Object  instance_type;
  typedef Object& instance_reference;

  // Can mess with Args... if you need to, for example:
  static constexpr std::size_t argument_count = sizeof...(Args);
};


// If you intend to support const member functions you need another specialization.
template<typename Return, typename Object, typename... Args>
struct member_function_traits<Return (Object::*)(Args...) const> {
  typedef Return        return_type;
  typedef Object        instance_type;
  typedef Object const& instance_reference;

  // Can mess with Args... if you need to, for example:
  static constexpr std::size_t argument_count = sizeof...(Args);
};


template<typename R, typename C, typename... Args>
C function_pointer_class(R (C::*)(Args...));
}// namespace urand::plot::util