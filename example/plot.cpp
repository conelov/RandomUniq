//
// Created by dym on 29.06.22.
//

#include "example/util/generator.hpp"
#include "example/util/rangeScaleView.hpp"
#include "ui_FormMain.h"
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/vector_of.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/keys.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/string.hpp>
#include <QApplication>
#include <unordered_map>


namespace {

void setVisibleWidget(bool visible, QWidget* w1, auto*... ws) {
  w1->setVisible(visible);
  (ws->setVisible(visible), ...);
}


class Plot final : public QMainWindow
    , private Ui::FormMain {
  Q_OBJECT
public:
  Plot() {
    setupUi(this);

    cb_genMethod->clear();
    boost::hana::for_each(boost::hana::keys(stringToMethod_), [this](auto&& str) {
      cb_genMethod->addItem(std::forward<decltype(str)>(str).c_str());
    });

    on_cb_customXScale_toggled(cb_customXScale->isChecked());
    on_cb_customYScale_toggled(cb_customYScale->isChecked());
  }

private:
  using Data_ = boost::bimap<
    boost::bimaps::set_of<std::size_t>,
    boost::bimaps::multiset_of<std::size_t>,
    boost::bimaps::vector_of_relation>;

private:
  static constexpr auto stringToMethod_ = [] {
    using namespace boost::hana::literals;
    using boost::hana::make_pair;
    return boost::hana::make_map(
      make_pair("UniformIntDistributionUniq LinearDoobleGen"_s, [](auto l, auto r) {
        return urand::plot::util::uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::LinearDoobleGen>(l, r);
      }),
      make_pair("UniformIntDistributionUniq NonLinearEqualChanceRange"_s, [](auto l, auto r) {
        return urand::plot::util::uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::NonLinearEqualChanceRange>(l, r);
      }),
      make_pair("linear"_s, [](auto l, auto r) {
        return urand::plot::util::uniformIntDistributionLinearMid<1>(l, r);
      }),
      make_pair("linear x3"_s, [](auto l, auto r) {
        return urand::plot::util::uniformIntDistributionLinearMid<3>(l, r);
      }));
  }();
  Data_ data_;

private:
  void setRange(QCPAxis& axis, auto min, auto max) {
    urand::util::minmaxException(min, max);
    axis.setRange(min - min * .01, max + max * .01);
  }


  void replot() {
    qcp_plot->clearGraphs();
    const auto graph = qcp_plot->addGraph();

    for (auto const [key, value] : data_) {
      graph->addData(key, value);
    }
    setRange(*qcp_plot->xAxis, data_.left.begin()->first, std::prev(data_.left.end())->first);
    setRange(*qcp_plot->yAxis, 0u, std::prev(data_.right.end())->first);

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
    boost::hana::for_each(stringToMethod_, [this](auto const& pair) {
      if (boost::hana::first(pair).c_str() != cb_genMethod->currentText()) {
        return;
      }
      data_.clear();
      std::unordered_map<std::size_t, std::size_t> mapTmp;
      for (auto                        gen = boost::hana::second(pair)(sb_rangeMin->value(), sb_rangeMax->value());
           [[maybe_unused]] auto const i : ranges::views::iota(0, sp_repeatCount->value())) {
        ++mapTmp[gen()];
      }
      for (auto const [key, value] : std::move(mapTmp)) {
        data_.push_back({key, value});
      }
    });
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