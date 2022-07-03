//
// Created by dym on 29.06.22.
//

#include "example/util/generator.hpp"
#include "ui_FormMain.h"
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/vector_of.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/unpack.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <QApplication>
#include <unordered_map>


namespace {

using GenCtor = std::function<std::function<void()>()>;

enum class GenConstraint : std::uint8_t {
  Limited,
  Unlimited
};
}// namespace

Q_DECLARE_METATYPE(GenCtor)
Q_DECLARE_METATYPE(GenConstraint)


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
    setWindowTitle(BOOST_PP_STRINGIZE(PROJECT_NAME));

    cb_genMethod->clear();
    namespace hana = boost::hana;
    hana::for_each(
      hana::make_tuple(
        hana::make_tuple(
          "UniformIntDistributionUniq LinearDoobleGen",
          [](auto l, auto r) {
            return urand::plot::util::uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::LinearDoobleGen>(l, r);
          },
          GenConstraint::Limited),
        hana::make_tuple(
          "UniformIntDistributionUniq NonLinearEqualChanceRange",
          [](auto l, auto r) {
            return urand::plot::util::uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::NonLinearEqualChanceRange>(l, r);
          },
          GenConstraint::Limited),
        hana::make_tuple(
          "linear",
          [](auto l, auto r) {
            return urand::plot::util::uniformIntDistributionLinearMid<1>(l, r);
          },
          GenConstraint::Unlimited),
        hana::make_tuple(
          "linear x3",
          [](auto l, auto r) {
            return urand::plot::util::uniformIntDistributionLinearMid<3>(l, r);
          },
          GenConstraint::Unlimited)),
      [this](auto&& i) {
        hana::unpack(std::forward<decltype(i)>(i),
          [this](auto&& name, auto&& gen, GenConstraint constraint) {
            QObject* obj = new QObject{this};
            obj->setProperty("gen", QVariant::fromValue(GenCtor{[this, gen] {
              return std::invoke(gen, sb_rangeMin->value(), sb_rangeMax->value());
            }}));
            obj->setProperty("genConstraint", QVariant::fromValue(constraint));
            cb_genMethod->addItem(std::forward<decltype(name)>(name), QVariant::fromValue(obj));
          });
      });


    for (auto const sb : {sb_rangeMin, sb_rangeMax}) {
      QObject::connect(sb, qOverload<int>(&QSpinBox::valueChanged), this, &Plot::sbRangeChanged);
      QObject::connect(sb, qOverload<int>(&QSpinBox::valueChanged), this, &Plot::sbRepeatCountLimitUpdate);
    }
    QObject::connect(cb_genMethod, qOverload<int>(&QComboBox::currentIndexChanged), this, &Plot::sbRepeatCountLimitUpdate);


    on_sb_customXScale_toggled(sb_customXScale->isChecked());
    on_sb_customYScale_toggled(sb_customYScale->isChecked());
  }

private:
  using Data = boost::bimap<
    boost::bimaps::set_of<std::size_t>,
    boost::bimaps::multiset_of<std::size_t>,
    boost::bimaps::vector_of_relation>;

private:
  Data data_;

private:
  void setRange(QCPAxis& axis, auto min, auto max) {
    assert(max >= min);
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


  //  void setGenRangeConstraints(GenRepeatMode repeatMode) {
  //  }


  void setGenRangeConstraints() {
    //    setGenRangeConstraints(boost::hana::for_each(genMethodType_, [this](auto const& pair) {
    //      if (boost::hana::first(pair).c_str() != cb_genMethod->currentText()) {
    //        return;
    //      }
    //
    //      return boost::hana::second(pair).repeatMode;
    //    }));
  }

private slots:
  void sbRangeChanged() {
    QSignalBlocker const _1{sb_rangeMin}, _2{sb_rangeMax};
    if (sender()->objectName() == BOOST_PP_STRINGIZE(sp_customXScale_max)) {
      sb_rangeMin->setMaximum(sb_rangeMax->value());
    } else {
      sb_rangeMax->setMinimum(sb_rangeMin->value());
    }
  }


  void sbRepeatCountLimitUpdate() {
    switch (cb_genMethod->itemData(cb_genMethod->currentIndex()).value<QObject*>()->property("genConstraint").value<GenConstraint>()) {
      case GenConstraint::Limited:
        sb_repeatCount->setMaximum(sb_rangeMax->value() - sb_rangeMin->value() + 1);
        break;
      case GenConstraint::Unlimited:
        sb_repeatCount->setMaximum(std::numeric_limits<int>::max());
        break;
    }
  }


  void on_sb_customXScale_toggled(bool checked) {
    setVisibleWidget(checked, f_customXScale);
  }


  void on_sb_customYScale_toggled(bool checked) {
    setVisibleWidget(checked, f_customYScale);
  }


  void on_pb_gen_released() {
    //    boost::hana::for_each(genMethodType_, [this](auto const& pair) {
    //      if (boost::hana::first(pair).c_str() != cb_genMethod->currentText()) {
    //        return;
    //      }
    //
    //      data_.clear();
    //      {
    //        std::unordered_map<std::size_t, std::size_t> mapTmp;
    //        auto                                         gen = boost::hana::second(pair).ctor(sb_rangeMin->value(), sb_rangeMax->value());
    //        for ([[maybe_unused]] auto const i : ranges::views::iota(0, sb_repeatCount->value())) {
    //          ++mapTmp[gen()];
    //        }
    //        for (auto const [key, value] : std::move(mapTmp)) {
    //          data_.push_back({key, value});
    //        }
    //      }
    //
    //      switch (boost::hana::second(pair).repeatMode) {
    //        case GenRepeatMode::NonLimited:
    //          sb_rangeMax->setValue(std::numeric_limits<int>::max());
    //          sb_repeatCount->setValue(std::numeric_limits<int>::max());
    //        case GenRepeatMode::Limited:
    //        default:
    //          assert(false);
    //      }
    //    });
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