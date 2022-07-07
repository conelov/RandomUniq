//
// Created by dym on 08.07.22.
//

#include "plot/util/ButtonGroup.hpp"
#include <QAbstractButton>


namespace urand::plot::util {

ButtonGroup::~ButtonGroup() = default;


ButtonGroup::ButtonGroup(gsl::not_null<QObject*> parent)
    : QButtonGroup{parent} {
  ::QObject::connect(this, qOverload<QAbstractButton*, bool>(&QButtonGroup::buttonToggled), this, &ButtonGroup::onButtonToggled);
}


void ButtonGroup::onButtonToggled(QAbstractButton* btn, bool checked) {
  if (checked) {
    emit buttonToggledTrue(btn);
  }
}
}// namespace plot