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
class Plot final : public QMainWindow
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
  }

private:
  const std::map<QString, QVector<std::size_t> (Plot::*)()> stringToMethod_;

private:
  template<urand::UniformIntDistributionUniqGenType GenType>
  QVector<std::size_t> uniformIntDistributionUniqAtType() {
    const std::size_t                                       repeatCount = sp_repeatCount->value();
    QVector<std::size_t>                                    data(repeatCount);
    urand::UniformIntDistributionUniq<std::size_t, GenType> distribution(0, repeatCount - 1u);
    for ([[maybe_unused]] auto const i : ranges::views::iota(0u, repeatCount - 1u)) {
      ++data[distribution()];
    }
    return data;
  }


  QVector<std::size_t> method_UniformIntDistributionUniq_LinearDoobleGen() {
    return uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::LinearDoobleGen>();
  }


  QVector<std::size_t> method_UniformIntDistributionUniq_NonLinearEqualChanceRange() {
    return uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::NonLinearEqualChanceRange>();
  }


  QVector<std::size_t> linear_impl(std::size_t count) {
    const std::size_t                          repeatCount = sp_repeatCount->value();
    QVector<std::size_t>                       data(repeatCount);
    std::uniform_int_distribution<std::size_t> distribution(0, repeatCount - 1u);
    for ([[maybe_unused]] auto const i : ranges::views::iota(0u, repeatCount - 1u)) {
      auto const key = ranges::accumulate(ranges::views::iota(0u, count), 0, [&](auto const lhs, auto const rhs) {
        return lhs + distribution(urand::util::RandomDevice<std::mt19937>::get());
      }) / count;
      ++data[key];
    }
    return data;
  }


  QVector<std::size_t> method_linear() {
    return linear_impl(1);
  }


  QVector<std::size_t> method_linear_x3() {
    return linear_impl(3);
  }

private slots:
  void on_pb_gen_released() {
    qcp_plot->clearPlottables();
    if (cb_x_axis_scale->isChecked()) {
      std::map<double, std::size_t> keyValueData;
      const auto                    values = std::invoke(stringToMethod_.at(cb_genMethod->currentText()), this);
      const std::size_t             xMax = *std::max(values.cbegin(), values.cend());

      for (std::size_t i = 0; i < sp_barsCount->value(); ++i) {

        auto const key =
        keyValueData[1. / sp_barsCount->value() * xMax] += values[i];
      }
    }
    double xMax = 0, yMax = 0;
    {
      auto const bar  = new QCPBars(qcp_plot->xAxis, qcp_plot->yAxis);
      const auto data = std::invoke(stringToMethod_.at(cb_genMethod->currentText()), this);
      for (std::size_t i{}; i < data.size(); ++i) {
        auto const key   = cb_x_axis_scale->isChecked() ? (1. / static_cast<double>(i) * sp_barsCount->value()) : i;
        auto const value = data[i];
        xMax             = std::max(xMax, key);
        yMax             = std::max(yMax, value);
        bar->addData(key, value);
      }
    }
    if (cb_x_axis_scale->isChecked()) {
      qcp_plot->xAxis->setRange(-0.01, 1.01);
      qcp_plot->yAxis->setRange(0, 1.01);
    } else {
      qcp_plot->xAxis->setRange(-0.01, xMax + xMax * 0.01);
      qcp_plot->yAxis->setRange(0, yMax + yMax * 0.01);
    }

    qcp_plot->replot();
  }

  void on_cb_x_axis_scale_stateChanged(int state) {
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