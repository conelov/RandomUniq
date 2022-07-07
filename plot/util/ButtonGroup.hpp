//
// Created by dym on 08.07.22.
//

#pragma once

#include <gsl/pointers>
#include <QButtonGroup>


namespace urand::plot::util {

class ButtonGroup final : public QButtonGroup {
  Q_OBJECT
public:
  ~ButtonGroup() override;
  ButtonGroup(gsl::not_null<QObject*> parent);

signals:
  void buttonToggledTrue(QAbstractButton*);

private slots:
  void onButtonToggled(QAbstractButton* btn, bool checked);
};
}// namespace urand::plot::util
