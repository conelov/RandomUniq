//
// Created by dym on 29.06.22.
//

#include "randomUniq/util/RandomDevice.hpp"
#include "range/v3/view/iota.hpp"
#include "ui_FormMain.h"
#include <map>
#include <QApplication>
#include <QDebug>


namespace {
class Plot final : public QWidget
    , public Ui::FormMain {
  Q_OBJECT
public:
  Plot() {
    setupUi(this);

    constexpr std::size_t              resolutionX = 100;
    std::size_t                        resolutionY = 0;
    std::map<std::size_t, std::size_t> data;
    {
      constexpr std::size_t                      repeatCount = 1'000'000;
      std::uniform_int_distribution<std::size_t> distribution(0, resolutionX);
      for (auto const i : ranges::views::iota(0u, repeatCount - 1u)) {
        auto const key = (distribution(urand::util::RandomDevice<std::mt19937>::get()) +
                           distribution(urand::util::RandomDevice<std::mt19937>::get()) +
                           distribution(urand::util::RandomDevice<std::mt19937>::get())) / 3;
        resolutionY    = std::max(resolutionY, ++data[key]);
      }
    }
    auto const bars = new QCPBars(qcp_plot->xAxis, qcp_plot->yAxis);
    bars->setWidth((1. / resolutionX) * 0.9);
    for (auto const [key, value] : data) {
      bars->addData(static_cast<double>(key) / resolutionX, static_cast<double>(value) / resolutionY);
    }
    qcp_plot->xAxis->setRange(-0.01, 1.01);
    qcp_plot->yAxis->setRange(0, 1.01);
    qcp_plot->replot();
  }
};
}// namespace

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  Plot         plot;
  plot.show();

  return QApplication::exec();
}

#include "plot.moc"