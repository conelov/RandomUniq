//
// Created by dym on 29.06.22.
//

#include "randomUniq/UniformIntDistributionUniq.hpp"
#include "randomUniq/util/RandomDevice.hpp"
#include "range/v3/numeric/accumulate.hpp"
#include "range/v3/view/iota.hpp"
#include "ui_FormMain.h"
#include <map>
#include <QApplication>


namespace {
class Plot final : public QWidget
    , public Ui::FormMain {
  Q_OBJECT
public:
  Plot()
      : stringToMethod_{
        {"UniformIntDistributionUniq LinearDoobleGen", &Plot::method_UniformIntDistributionUniq_LinearDoobleGen},
        {"UniformIntDistributionUniq NonLinearEqualChanceRange", &Plot::method_UniformIntDistributionUniq_NonLinearEqualChanceRange},
        {"linear", &Plot::method_linear},
        {"linear x3", &Plot::method_linear_x3},
      } {
    setupUi(this);
    qcp_plot->xAxis->setRange(-0.01, 1.01);
    qcp_plot->yAxis->setRange(0, 1.01);
  }

private:
  const std::map<QString, void (Plot::*)()> stringToMethod_;

private:
  template<urand::UniformIntDistributionUniqGenType GenType>
  void uniformIntDistributionUniqAtType() {
    constexpr std::size_t              resolutionX = 100;
    std::size_t                        resolutionY = 0;
    std::map<std::size_t, std::size_t> data;
    {
      const std::size_t                                       repeatCount = sp_repeatCount->value();
      urand::UniformIntDistributionUniq<std::size_t, GenType> distribution(0, repeatCount);
      for ([[maybe_unused]] auto const i : ranges::views::iota(0u, repeatCount - 1u)) {
        resolutionY = std::max(resolutionY, ++data[distribution()]);
      }
    }
    auto const bars = new QCPBars(qcp_plot->xAxis, qcp_plot->yAxis);
    bars->setWidth((1. / resolutionX) * 0.8);
    for (auto const [key, value] : data) {
      bars->addData(static_cast<double>(key) / resolutionX, static_cast<double>(value) / resolutionY);
    }
  }


  void method_UniformIntDistributionUniq_LinearDoobleGen() {
    uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::LinearDoobleGen>();
  }


  void method_UniformIntDistributionUniq_NonLinearEqualChanceRange() {
    uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::NonLinearEqualChanceRange>();
  }


  void linear_impl(std::size_t count) {
    constexpr std::size_t              resolutionX = 100;
    std::size_t                        resolutionY = 0;
    std::map<std::size_t, std::size_t> data;
    {
      const std::size_t                          repeatCount = sp_repeatCount->value();
      std::uniform_int_distribution<std::size_t> distribution(0, resolutionX);
      for ([[maybe_unused]] auto const i : ranges::views::iota(0u, repeatCount - 1u)) {
        auto const key = ranges::accumulate(ranges::views::iota(0u, count), 0, [&](auto const lhs, auto const rhs) {
          return lhs + distribution(urand::util::RandomDevice<std::mt19937>::get());
        }) / count;
        resolutionY    = std::max(resolutionY, ++data[key]);
      }
    }
    auto const bars = new QCPBars(qcp_plot->xAxis, qcp_plot->yAxis);
    bars->setWidth((1. / resolutionX) * 0.9);
    for (auto const [key, value] : data) {
      bars->addData(static_cast<double>(key) / resolutionX, static_cast<double>(value) / resolutionY);
    }
  }


  void method_linear() {
    linear_impl(1);
  }


  void method_linear_x3() {
    linear_impl(3);
  }

private slots:
  void on_pb_gen_released() {
    qcp_plot->clearPlottables();
    std::invoke(stringToMethod_.at(cb_genMethod->currentText()), this);
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