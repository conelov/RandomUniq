//
// Created by dym on 10.04.2022.
//

#include "example/util/EventFilter.hpp"


namespace util {

EventFilter::~EventFilter() = default;


bool EventFilter::eventFilter(QObject* watched, QEvent* event) {
  return std::invoke(handler, watched, event);
}
}// namespace util