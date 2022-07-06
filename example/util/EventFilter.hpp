//
// Created by dym on 10.04.2022.
//

#pragma once

#include <cassert>
#include <functional>
#include <QObject>


namespace util {

class EventFilter final : public QObject {
  Q_OBJECT
public:
  using Handler = std::function<bool(QObject*, QEvent*)>;
  Handler const handler;


public:
  ~EventFilter() override;


  template<typename H>
  EventFilter(H&& hin, QObject* parent = nullptr)
      : QObject{parent}
      , handler(std::forward<H>(hin)) {
    assert(handler);
  }


  bool eventFilter(QObject* watched, QEvent* event) override;
};
}// namespace util