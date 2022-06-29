//
// Created by dym on 29.06.22.
//

#include "ui_FormMain.h"
#include <QApplication>


namespace {
class Plot final : public QWidget
    , public Ui::FormMain {
  Q_OBJECT
public:
  Plot() {
    setupUi(this);
  }
};
}// namespace

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  Plot plot;
  plot.show();

  return QApplication::exec();
}

#include "plot.moc"