//
// Created by dym on 29.06.22.
//

#include "example/util/EventFilter.hpp"
#include "example/util/generator.hpp"
#include "ui_FormMain.h"
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/vector_of.hpp>
#include <boost/hana/at_key.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/pair.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/type.hpp>
#include <boost/hana/unpack.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <QApplication>
#include <QEvent>
#include <QStyleFactory>
#include <QWheelEvent>
#include <range/v3/view/zip.hpp>
#include <unordered_map>


namespace {

using GenCtor  = std::function<std::function<std::size_t()>()>;
using PlotCtor = std::function<std::function<void(double, double)>()>;


enum class GenConstraint : std::uint8_t {
  Limited,
  Unlimited
};


struct GenMem final {
  int rangeMin, rangeMax, repeatCount;
};
}// namespace

Q_DECLARE_METATYPE(GenCtor)
Q_DECLARE_METATYPE(PlotCtor)
Q_DECLARE_METATYPE(GenConstraint)
Q_DECLARE_METATYPE(GenMem)


namespace {

template<typename T>
auto typeStr() {
  return QMetaType{qMetaTypeId<T>()}.name();
}


class Plot final : public QMainWindow
    , private Ui::FormMain {
  Q_OBJECT
public:
  Plot() {
    setWindowTitle(QString{BOOST_PP_STRINGIZE(PROJECT_NAME)} + "_v" + BOOST_PP_STRINGIZE(PROJECT_VERSION));

    sa_viewOpt->verticalScrollBar()->setEnabled(false);
    sa_viewOpt->installEventFilter(new util::EventFilter{
      [hsb = sa_viewOpt->horizontalScrollBar()](QObject* obj, QEvent* event) {
        if (event->type() == QEvent::Wheel) {
          auto const wheelEvent = static_cast<QWheelEvent*>(event);
          hsb->setValue(hsb->value() - wheelEvent->angleDelta().x() + wheelEvent->angleDelta().y());
          return true;
        }
        return false;
      },
      sa_viewOpt});

    namespace hana = boost::hana;
    hana::for_each(
      hana::make_tuple(
        hana::make_tuple(
          "linear x3",
          [](auto... args) { return urand::plot::util::uniformIntDistributionLinearMid<3>(args...); },
          GenConstraint::Unlimited,
          GenMem{0, 1'000, 10'000}),
        hana::make_tuple(
          "linear",
          [](auto... args) { return urand::plot::util::uniformIntDistributionLinearMid<1>(args...); },
          GenConstraint::Unlimited,
          GenMem{0, 1'000, 10'000}),
        hana::make_tuple(
          "UniformIntDistributionUniq LinearDoobleGen",
          [](auto... args) { return urand::plot::util::uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::LinearDoobleGen>(args...); },
          GenConstraint::Limited,
          GenMem{0, 100, 50}),
        hana::make_tuple(
          "UniformIntDistributionUniq NonLinearEqualChanceRange",
          [](auto... args) { return urand::plot::util::uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::NonLinearEqualChanceRange>(args...); },
          GenConstraint::Limited,
          GenMem{0, 100, 50})),
      [this](auto&& i) {
        hana::unpack(std::forward<decltype(i)>(i), [this](auto&& name, auto&& gen, GenConstraint constraint, GenMem mem) {
          auto const obj = new QObject{this};
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

    for (auto const [cb, f] : {
           std::make_pair(cb_customXPointCount, f_customXPointCount),
           std::make_pair(cb_customXRange, f_customXRange),
           std::make_pair(cb_customYRange, f_customYRange),
           std::make_pair(cb_customXScale, f_customXScale),
           std::make_pair(cb_customYScale, f_customYScale)}) {
      initBind(cb, &QCheckBox::toggled, cb->isChecked(), f, &QWidget::setVisible);
    }

    on_cb_autoRegen_toggled(cb_autoRegen->isChecked());

    hana::for_each(
      hana::make_tuple(
        hana::make_tuple("Graph", [this] { return [p = qcp_plot->addGraph()](auto... args) { p->addData(args...); }; }),
        hana::make_tuple("Gusto", [this] { return [p = new QCPBars{qcp_plot->xAxis, qcp_plot->yAxis}](auto... args) { p->addData(args...); }; })),
      [this](auto&& tuple) {
        hana::unpack(std::forward<decltype(tuple)>(tuple), [this](auto&& text, auto&& printer) {
          auto const btn = new QRadioButton{std::forward<decltype(text)>(text)};
          btn->setProperty(typeStr<PlotCtor>(), QVariant::fromValue(PlotCtor{std::forward<decltype(printer)>(printer)}));
          gb_plotType->layout()->addWidget(btn);
          plotTypeButtonGroup_->addButton(btn);
        });
      });
    plotTypeButtonGroup_->button(-2)->setChecked(true);
  }

private:
  using Data = boost::bimap<
    boost::bimaps::set_of<std::size_t>,
    boost::bimaps::multiset_of<std::size_t>,
    boost::bimaps::vector_of_relation>;

private:
  Data                data_;
  QButtonGroup* const plotTypeButtonGroup_ = (setupUi(this), new QButtonGroup{gb_plotType});
  int                 genMethodIdxPrev     = 0;

private:
  static QMetaObject::Connection initBind(auto* sender, auto sig, auto&& initv, auto* receiver, auto&& slot) {
    auto connect = QObject::connect(sender, sig, receiver, slot);
    if constexpr (std::is_member_function_pointer_v<std::remove_cvref_t<decltype(slot)>>) {
      std::invoke(std::forward<decltype(slot)>(slot), receiver, initv);
    } else {
      std::invoke(std::forward<decltype(slot)>(slot), initv);
    }
    return connect;
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
    std::array const sbs{sb_rangeMin, sb_rangeMax, sb_repeatCount};
    std::array const mems{&GenMem::rangeMin, &GenMem::rangeMax, &GenMem::repeatCount};
    for (auto const [ui, mem] : ranges::views::zip(sbs, mems)) {
      QSignalBlocker const _{ui};
      std::mem_fn (&QSpinBox::setValue)(ui, std::invoke(mem, v));
    }
  }


  GenMem genMemFromUi() const {
    return {sb_rangeMin->value(),
      sb_rangeMax->value(),
      sb_repeatCount->value()};
  }

private slots:
  void generate() {
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


  void sbRangeChanged() {
    QSignalBlocker const _1{sb_rangeMin}, _2{sb_rangeMax};
    if (sender()->objectName() == BOOST_PP_STRINGIZE(sp_customXScale_max)) {
      sb_rangeMin->setMaximum(sb_rangeMax->value());
    } else {
      sb_rangeMax->setMinimum(sb_rangeMin->value());
    }
  }


  void sbRepeatCountLimitUpdate() {
    sb_repeatCount->setMaximum(std::invoke([this] {
      switch (genInfo()->property(BOOST_PP_STRINGIZE(GenConstraint)).value<GenConstraint>()) {
        case GenConstraint::Limited:
          return sb_rangeMax->value() - sb_rangeMin->value() + 1;
        case GenConstraint::Unlimited:
          return std::numeric_limits<int>::max();
        default:
          assert(false);
          return int{};
      }
    }));
  }


  void on_pb_gen_released() {
    generate();
  }


  void on_cb_genMethod_currentIndexChanged(int idx) {
    genInfo(genMethodIdxPrev)->setProperty(BOOST_PP_STRINGIZE(GenMem), QVariant::fromValue(genMemFromUi()));
    setUiFromGenMem(genInfo(idx)->property(BOOST_PP_STRINGIZE(GenMem)).value<GenMem>());
    genMethodIdxPrev = idx;
  }


  void on_cb_autoRegen_toggled(bool checked) {
    namespace hana = boost::hana;
    hana::for_each(hana::make_tuple(sb_rangeMin, sb_rangeMax, sb_repeatCount, cb_genMethod), [this, checked](auto i) {
      constexpr auto sigMap = hana::make_map(
        hana::make_pair(hana::type_c<QSpinBox>, qOverload<int>(&QSpinBox::valueChanged)),
        hana::make_pair(hana::type_c<QComboBox>, qOverload<int>(&QComboBox::currentIndexChanged)));
      hana::unpack(hana::make_tuple(i, sigMap[hana::type_c<std::remove_pointer_t<decltype(i)>>], this, &Plot::generate),
        [checked](auto&&... args) {
          if (checked) {
            ::QObject::connect(args...);
          } else {
            ::QObject::disconnect(args...);
          }
        });
    });
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