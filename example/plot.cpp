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
#include <boost/hana/zip.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <QApplication>
#include <QStyleFactory>
#include <unordered_map>


namespace {

using GenCtor = std::function<std::function<std::size_t()>()>;


enum class GenConstraint : std::uint8_t {
  Limited,
  Unlimited
};


struct GenMem final {
  int rangeMax, rangeMin, repeatCount;
};
}// namespace

Q_DECLARE_METATYPE(GenCtor)
Q_DECLARE_METATYPE(GenConstraint)
Q_DECLARE_METATYPE(GenMem)


namespace {

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
          "linear x3",
          [](auto l, auto r) {
            return urand::plot::util::uniformIntDistributionLinearMid<3>(l, r);
          },
          GenConstraint::Unlimited,
          GenMem{0, 1'000, 10'000}),
        hana::make_tuple(
          "linear",
          [](auto l, auto r) {
            return urand::plot::util::uniformIntDistributionLinearMid<1>(l, r);
          },
          GenConstraint::Unlimited,
          GenMem{0, 1'000, 10'000}),
        hana::make_tuple(
          "UniformIntDistributionUniq LinearDoobleGen",
          [](auto l, auto r) {
            return urand::plot::util::uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::LinearDoobleGen>(l, r);
          },
          GenConstraint::Limited,
          GenMem{0, 100, 50}),
        hana::make_tuple(
          "UniformIntDistributionUniq NonLinearEqualChanceRange",
          [](auto l, auto r) {
            return urand::plot::util::uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::NonLinearEqualChanceRange>(l, r);
          },
          GenConstraint::Limited,
          GenMem{0, 100, 50})),
      [this](auto&& i) {
        hana::unpack(std::forward<decltype(i)>(i),
          [this](auto&& name, auto&& gen, GenConstraint constraint, GenMem mem) {
            QObject* obj = new QObject{this};
            obj->setProperty("gen", QVariant::fromValue(GenCtor{[this, gen] {
              return std::invoke(gen, sb_rangeMin->value(), sb_rangeMax->value());
            }}));
            obj->setProperty(BOOST_PP_STRINGIZE(GenConstraint), QVariant::fromValue(constraint));
            obj->setProperty(BOOST_PP_STRINGIZE(GenMem), QVariant::fromValue(mem));
            cb_genMethod->addItem(std::forward<decltype(name)>(name), QVariant::fromValue(obj));
          });
      });

    for (auto const sb : {sb_rangeMin, sb_rangeMax}) {
      QObject::connect(sb, qOverload<int>(&QSpinBox::valueChanged), this, &Plot::sbRangeChanged);
      QObject::connect(sb, qOverload<int>(&QSpinBox::valueChanged), this, &Plot::sbRepeatCountLimitUpdate);
    }
    sbRepeatCountLimitUpdate();

    on_sb_customXScale_toggled(sb_customXScale->isChecked());
    on_sb_customYScale_toggled(sb_customYScale->isChecked());
  }

private:
  using Data = boost::bimap<
    boost::bimaps::set_of<std::size_t>,
    boost::bimaps::multiset_of<std::size_t>,
    boost::bimaps::vector_of_relation>;

private:
  int  cb_genMethodPrevIdx = 0;
  Data data_;

private:
  static void setVisibleWidget(bool visible, QWidget* w1, auto*... ws) {
    w1->setVisible(visible);
    (ws->setVisible(visible), ...);
  }


  static void setRange(QCPAxis& axis, auto min, auto max) {
    assert(max >= min);
    axis.setRange(min - min * .01, max + max * .01);
  }

private:
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


  QObject* genInfo(int idx) {
    return cb_genMethod->itemData(idx).value<QObject*>();
  }


  QObject* genInfo() {
    return genInfo(cb_genMethod->currentIndex());
  }


  void setUiFromGenMem(GenMem v) {
    for (auto const [ui, mem] : boost::hana::zip(
           std::array{sb_rangeMin, sb_rangeMax, sb_repeatCount},
           std::array{&GenMem::rangeMin, &GenMem::rangeMax, &GenMem::repeatCount})) {
      QSignalBlocker const _{ui};
      std::mem_fn (&QSpinBox::setValue)(ui, std::invoke(mem, v));
    }
    sb_rangeMin->setValue(v.rangeMin);
    sb_rangeMax->setValue(v.rangeMax);
    sb_repeatCount->setValue(v.repeatCount);
  }


  GenMem genMemFromUi() const {
    return {sb_rangeMin->value(),
      sb_rangeMax->value(),
      sb_repeatCount->value()};
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
    switch (genInfo()->property(BOOST_PP_STRINGIZE(GenConstraint)).value<GenConstraint>()) {
      case GenConstraint::Limited:
        sb_repeatCount->setMaximum(sb_rangeMax->value() - sb_rangeMin->value() + 1);
        break;
      case GenConstraint::Unlimited:
        sb_repeatCount->setMaximum(std::numeric_limits<int>::max());
        break;
      default:
        assert(false);
    }
  }


  void on_sb_customXScale_toggled(bool checked) {
    setVisibleWidget(checked, f_customXScale);
  }


  void on_sb_customYScale_toggled(bool checked) {
    setVisibleWidget(checked, f_customYScale);
  }


  void on_pb_gen_released() {
    data_.clear();
    {
      std::unordered_map<std::size_t, std::size_t> mapTmp;
      auto                                         gen = genInfo()->property("gen").value<GenCtor>()();
      for ([[maybe_unused]] auto const i : ranges::views::iota(0, sb_repeatCount->value())) {
        ++mapTmp[gen()];
      }
      for (auto const [key, value] : std::move(mapTmp)) {
        data_.push_back({key, value});
      }
    }
    replot();
  }


  void on_cb_genMethod_currentIndexChanged(int idx) {
    genInfo(cb_genMethodPrevIdx)->setProperty(BOOST_PP_STRINGIZE(GenMem), QVariant::fromValue(genMemFromUi()));
    setUiFromGenMem(genInfo(idx)->property(BOOST_PP_STRINGIZE(GenMem)).value<GenMem>());
    cb_genMethodPrevIdx = idx;
  }
};
}// namespace


int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  if (QString const preferredStyle = "Fusion";
      QStyleFactory::keys().contains(preferredStyle)) {
    QApplication::setStyle(QStyleFactory::create(preferredStyle));
  }

  Plot plot;
  plot.show();

  return QApplication::exec();
}

#include "plot.moc"