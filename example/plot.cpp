//
// Created by dym on 29.06.22.
//

#include "example/util/rangeScaleView.hpp"
#include "randomUniq/UniformIntDistributionUniq.hpp"
#include "randomUniq/util/RandomDevice.hpp"
#include "ui_FormMain.h"
#include <boost/hana/string.hpp>
#include <boost/hana/map.hpp>
#include <map>
#include <QApplication>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/generate.hpp>


namespace {

void setVisibleWidget(bool visible, QWidget* w1, auto*... ws) {
  w1->setVisible(visible);
  (ws->setVisible(visible), ...);
}


class Plot final : public QMainWindow
    , private Ui::FormMain {
  Q_OBJECT
private:
//  static constexpr auto stringToMethodHana_ = boost::hana::make_map (
//    boost::hana::make_pair(BOOST_HANA_STRING("UniformIntDistributionUniq LinearDoobleGen"), ranges::views::generate([]{
//      return true;
//    }))
//    );
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
    for (const auto& [key, value] : stringToMethod_) {
      cb_genMethod->addItem(key);
    }

    on_cb_customXScale_toggled(cb_customXScale->isChecked());
    on_cb_customYScale_toggled(cb_customYScale->isChecked());
  }

private:
  using Data_ = std::map<std::size_t, std::size_t>;

private:
  const std::map<QString, Data_ (Plot::*)()> stringToMethod_;
  Data_                                      data_;

private:
  template<urand::UniformIntDistributionUniqGenType GenType>
  Data_ uniformIntDistributionUniqAtType() {
    const std::size_t                                       repeatCount = sp_repeatCount->value();
    Data_                                                   data;
    urand::UniformIntDistributionUniq<std::size_t, GenType> distribution(0, repeatCount - 1u);
    for ([[maybe_unused]] const auto i : ranges::views::iota(0u, repeatCount - 1u)) {
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
    for ([[maybe_unused]] const auto i : ranges::views::iota(0u, repeatCount - 1u)) {
      const auto key = ranges::accumulate(ranges::views::iota(0u, count), 0, [&](const auto lhs, const auto rhs) {
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


  void rangeIntegrate(double const toMin, double const toMax, std::size_t const toCount, auto&& f) const {
  }


  void replot() {
    qcp_plot->clearGraphs();
    const auto graph = qcp_plot->addGraph();

    data_ = std::invoke(stringToMethod_.at(cb_genMethod->currentText()), this);
    for (const auto [key, value] : data_) {
      graph->addData(key, value);
    }
    qcp_plot->xAxis->setRange(data_.begin()->first, std::prev(data_.end())->first);
    qcp_plot->yAxis->setRange(data_.begin()->second, std::prev(data_.end())->second);
    qcp_plot->replot();

    rangeIntegrate(sp_customXScale_min->value(), sp_customXScale_max->value(), sp_barsCount->value(), [] {});
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