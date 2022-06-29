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
  const std::map<QString, QMap<double, double> (Plot::*)()> stringToMethod_;

private:
  template<urand::UniformIntDistributionUniqGenType GenType>
  QMap<double, double> uniformIntDistributionUniqAtType() {
    QMap<double, double>  data;
    {
      const std::size_t                                       repeatCount = sp_repeatCount->value();
      urand::UniformIntDistributionUniq<std::size_t, GenType> distribution(0, repeatCount);
      for ([[maybe_unused]] auto const i : ranges::views::iota(0u, repeatCount - 1u)) {
        ++data[distribution()];
      }
    }
    return data;
  }


  QMap<double, double> method_UniformIntDistributionUniq_LinearDoobleGen() {
    return uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::LinearDoobleGen>();
  }


  QMap<double, double> method_UniformIntDistributionUniq_NonLinearEqualChanceRange() {
    return uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::NonLinearEqualChanceRange>();
  }


  QMap<double, double> linear_impl(std::size_t count) {
    QMap<double, double> data;
    {
      const std::size_t                          repeatCount = sp_repeatCount->value();
      std::uniform_int_distribution<std::size_t> distribution(0, repeatCount);
      for ([[maybe_unused]] auto const i : ranges::views::iota(0u, repeatCount - 1u)) {
        auto const key = ranges::accumulate(ranges::views::iota(0u, count), 0, [&](auto const lhs, auto const rhs) {
          return lhs + distribution(urand::util::RandomDevice<std::mt19937>::get());
        }) / count;
        ++data[key];
      }
    }
    return data;
  }


  QMap<double, double> method_linear() {
    return linear_impl(1);
  }


  QMap<double, double> method_linear_x3() {
    return linear_impl(3);
  }

private slots:
  void on_pb_gen_released() {
    qcp_plot->clearPlottables();
    auto const data = std::invoke(stringToMethod_.at(cb_genMethod->currentText()), this);
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