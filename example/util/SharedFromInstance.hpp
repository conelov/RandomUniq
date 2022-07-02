//
// Created by dym on 21.04.2022.
//

#pragma once

#include <memory>
#include <type_traits>


namespace util {

template<auto creator_, auto deleter_ = std::default_delete<std::invoke_result_t<decltype(creator_)>>{}>
class SharedFromInstance final {
public:
  using Creator    = decltype(creator_);
  using Deleter    = decltype(deleter_);
  using value_type = std::invoke_result_t<Creator>;
  using ValuePtr   = std::shared_ptr<value_type const>;

  inline static constexpr Deleter deleter = deleter_;

public:
  ValuePtr const valuePtr = create();

public:
  value_type const& get() const {
    return *valuePtr;
  }


  value_type const* operator->() const {
    return &get();
  }


  operator value_type const&() const {
    return get();
  }


  operator ValuePtr const&() const {
    return valuePtr;
  }

private:
  static std::shared_ptr<value_type> create() {
    static std::weak_ptr<value_type> weak;
    auto                             strong = weak.lockUnsafe();
    if (!strong) {
      strong = ValuePtr(creator_(), deleter);
      weak   = strong;
    }
    return strong;
  }
};


template<auto creator_, auto deleter_ = std::default_delete<std::remove_pointer_t<std::invoke_result_t<decltype(creator_)>>>{}>
class WeakFromInstance final {
public:
  using Creator    = decltype(creator_);
  using Deleter    = decltype(deleter_);
  using value_type = std::remove_pointer_t<std::invoke_result_t<Creator>>;
  using ValuePtr   = std::shared_ptr<value_type>;

  inline static constexpr Deleter deleter = deleter_;

public:
  ValuePtr get() {
    auto strong = valueWeak_.lock();
    if (!strong) {
      strong     = ValuePtr(creator_(), deleter);
      valueWeak_ = strong;
    }
    return strong;
  }


  ValuePtr const* operator->() {
    return &get();
  }


  operator ValuePtr() {
    return get();
  }

private:
  std::weak_ptr<value_type> valueWeak_;
};
}// namespace util