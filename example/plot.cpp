//
// Created by dym on 29.06.22.
//

#include "randomUniq/UniformIntDistributionUniq.hpp"
#include "randomUniq/util/RandomDevice.hpp"
#include "range/v3/numeric/accumulate.hpp"
#include "range/v3/view/group_by.hpp"
#include "range/v3/view/iota.hpp"
#include "range/v3/view/linear_distribute.hpp"
#include "range/v3/view/zip.hpp"
#include "ui_FormMain.h"
#include <map>
#include <QApplication>


namespace {

void setVisibleWidget(bool visible, QWidget* w1, auto*... ws) {
  w1->setVisible(visible);
  (ws->setVisible(visible), ...);
}


class Plot final : public QMainWindow
    , private Ui::FormMain {
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

    cb_genMethod->clear();
    for (auto const& [key, value] : stringToMethod_) {
      cb_genMethod->addItem(key);
    }

    on_cb_customXScale_toggled(cb_customXScale->isChecked());
    on_cb_customYScale_toggled(cb_customYScale->isChecked());
  }

private:
  using Data_ = std::map<std::size_t, std::size_t>;

private:
  const std::map<QString, Data_ (Plot::*)()> stringToMethod_;

private:
  template<urand::UniformIntDistributionUniqGenType GenType>
  Data_ uniformIntDistributionUniqAtType() {
    const std::size_t                                       repeatCount = sp_repeatCount->value();
    Data_                                                   data;
    urand::UniformIntDistributionUniq<std::size_t, GenType> distribution(0, repeatCount - 1u);
    for ([[maybe_unused]] auto const i : ranges::views::iota(0u, repeatCount - 1u)) {
      ++data[distribution()];
    }
    return data;
  }


  Data_ method_UniformIntDistributionUniq_LinearDoobleGen() {
    return uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::LinearDoobleGen>();
  }


  Data_ method_UniformIntDistributionUniq_NonLinearEqualChanceRange() {
    return uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::NonLinearEqualChanceRange>();
  }


  Data_ linear_impl(std::size_t count) {
    const std::size_t                          repeatCount = sp_repeatCount->value();
    Data_                                      data;
    std::uniform_int_distribution<std::size_t> distribution(0, repeatCount - 1u);
    for ([[maybe_unused]] auto const i : ranges::views::iota(0u, repeatCount - 1u)) {
      auto const key = ranges::accumulate(ranges::views::iota(0u, count), 0, [&](auto const lhs, auto const rhs) {
        return lhs + distribution(urand::util::RandomDevice<std::mt19937>::get());
      }) / count;
      ++data[key];
    }
    return data;
  }


  Data_ method_linear() {
    return linear_impl(1);
  }


  Data_ method_linear_x3() {
    return linear_impl(3);
  }


  void replot() {
    qcp_plot->clearGraphs();
    Data_      keyValueData = std::invoke(stringToMethod_.at(cb_genMethod->currentText()), this);
    auto const graph = qcp_plot->addGraph();
    for (auto const [key, value] : keyValueData) {
      graph->addData(key, value);
    }
    qcp_plot->xAxis->setRange(keyValueData.begin()->first, std::prev(keyValueData.end())->first);
    qcp_plot->yAxis->setRange(keyValueData.begin()->second, std::prev(keyValueData.end())->second);
    qcp_plot->replot();
  }


private slots:
  void on_cb_customXScale_toggled(bool checked) {
    setVisibleWidget(checked, f_customXScale);
  }


  void on_cb_customYScale_toggled(bool checked) {
    setVisibleWidget(checked, f_customYScale);
  }

  void on_pb_gen_released() {
    replot();
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