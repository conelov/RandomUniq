//
// Created by dym on 29.06.22.
//

#include "plot/util/ButtonGroup.hpp"
#include "plot/util/EventFilter.hpp"
#include "plot/util/generator.hpp"
#include "plot/util/rangeScaleView.hpp"
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
#include <gsl/pointers>
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

namespace util = urand::plot::util;


struct TagInverseArg {};


template<typename T>
QByteArray typeStr() {
  return QMetaType{qMetaTypeId<T>()}.name();
}


namespace _ {

template<typename T>
T property(gsl::not_null<QObject*> obj) {
  return obj->property(typeStr<T>()).template value<T>();
}


void setProperty(gsl::not_null<QObject*> obj, auto&&... args) {
  static_assert(sizeof...(args));
  (obj->setProperty(typeStr<std::remove_cvref_t<decltype(args)>>(), QVariant::fromValue(std::forward<decltype(args)>(args))), ...);
}
}// namespace _


void connectToggled(bool connect, auto emitters, auto sigMap, auto* receiver, auto slot) {
  static_assert(std::is_member_function_pointer_v<std::remove_cvref_t<decltype(slot)>>);
  assert(receiver);
  namespace hana = boost::hana;
  hana::for_each(emitters, [=](auto* emitter) {
    assert(emitter);
    hana::unpack(
      hana::make_tuple(emitter, sigMap[hana::type_c<std::remove_pointer_t<decltype(emitter)>>], receiver, slot),
      [=](auto... args) {
        if (connect) {
          ::QObject::connect(args...);
        } else {
          ::QObject::disconnect(args...);
        }
      });
  });
}


void setRange(QCPAxis& axis, auto min, auto max) {
  assert(max >= min);
  axis.setRange(min - min * .01, max + max * .01);
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
          [](auto... args) { return util::uniformIntDistributionLinearMid<3>(args...); },
          GenConstraint::Unlimited,
          GenMem{0, 1'000, 10'000}),
        hana::make_tuple(
          "linear",
          [](auto... args) { return util::uniformIntDistributionLinearMid<1>(args...); },
          GenConstraint::Unlimited,
          GenMem{0, 1'000, 10'000}),
        hana::make_tuple(
          "UniformIntDistributionUniq LinearDoobleGen",
          [](auto... args) { return util::uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::LinearDoobleGen>(args...); },
          GenConstraint::Limited,
          GenMem{0, 100, 50}),
        hana::make_tuple(
          "UniformIntDistributionUniq NonLinearEqualChanceRange",
          [](auto... args) { return util::uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::NonLinearEqualChanceRange>(args...); },
          GenConstraint::Limited,
          GenMem{0, 100, 50})),
      [this](auto&& i) {
        hana::unpack(std::forward<decltype(i)>(i), [this](auto&& name, auto&& gen, GenConstraint constraint, GenMem mem) {
          auto const obj = new QObject{this};
          _::setProperty(obj,
            GenCtor{[this, gen] { return std::invoke(gen, sb_rangeMin->value(), sb_rangeMax->value()); }},
            constraint,
            mem);
          cb_genMethod->addItem(std::forward<decltype(name)>(name), QVariant::fromValue(obj));
        });
      });

    hana::for_each(
      hana::make_tuple(
        hana::make_tuple("Graph", [this] { return [p = qcp_plot->addGraph()](auto... args) { p->addData(args...); }; }),
        hana::make_tuple("Gusto", [this] { return [p = new QCPBars{qcp_plot->xAxis, qcp_plot->yAxis}](auto... args) { p->addData(args...); }; })),
      [this](auto&& tuple) {
        hana::unpack(std::forward<decltype(tuple)>(tuple), [this](auto&& text, auto&& printer) {
          auto const btn = new QRadioButton{std::forward<decltype(text)>(text)};
          _::setProperty(btn, PlotCtor{std::forward<decltype(printer)>(printer)});
          gb_plotType->layout()->addWidget(btn);
          plotTypeButtonGroup_->addButton(btn);
        });
      });
    plotTypeButtonGroup_->button(-2)->setChecked(true);

    hana::for_each(
      hana::make_tuple(
        hana::make_tuple(cb_autoRegen, pb_gen, TagInverseArg{}),
        hana::make_tuple(cb_autoReplot, f_replot, TagInverseArg{}),
        hana::make_tuple(cb_customXPointCount, f_customXPointCount),
        hana::make_tuple(cb_customXRange, f_customXRange),
        hana::make_tuple(cb_customYRange, f_customYRange),
        hana::make_tuple(cb_customXScale, f_customXScale),
        hana::make_tuple(cb_customYScale, f_customYScale)),
      [](auto tuple) {
        hana::unpack(tuple, [](QCheckBox* cb, QWidget* wg, auto... arg) {
          auto slot = [wg](bool checked) { wg->setVisible(sizeof...(arg) ? !checked : checked); };
          slot(cb->isChecked());
          ::QObject::connect(cb, &QCheckBox::toggled, wg, std::move(slot));
        });
      });

    hana::for_each(
      hana::make_tuple(
        hana::make_pair(sb_rangeMin, sb_rangeMax),
        hana::make_pair(dsb_customXRange_min, dsb_customXRange_max),
        hana::make_pair(dsb_customYRange_min, dsb_customYRange_max),
        hana::make_pair(dsb_customXScale_min, dsb_customXScale_max),
        hana::make_pair(dsb_customYScale_min, dsb_customYScale_max)),
      [](auto tuple) {
        hana::unpack(tuple, [](auto minsb, auto maxsb) {
          auto const connector = [](auto* emitter, auto* receiver, auto comp) {
            constexpr auto sigMap = hana::make_map(
              hana::make_pair(hana::type_c<QSpinBox>, qOverload<int>(&QSpinBox::valueChanged)),
              hana::make_pair(hana::type_c<QDoubleSpinBox>, qOverload<double>(&QDoubleSpinBox::valueChanged)));
            ::QObject::connect(emitter, sigMap[hana::type_c<std::remove_pointer_t<decltype(emitter)>>], receiver, [receiver, comp](auto val) {
              if (comp(val, receiver->value())) {
                receiver->setValue(val);
              }
            });
          };
          connector(minsb, maxsb, std::greater{});
          connector(maxsb, minsb, std::less{});
        });
      });

    for (auto const sb : {sb_rangeMin, sb_rangeMax}) {
      ::QObject::connect(sb, qOverload<int>(&QSpinBox::valueChanged), this, &Plot::sbRepeatCountLimitUpdate);
    }

    for (auto const [cb, slot] : {std::make_pair(cb_autoRegen, &Plot::generate), std::make_pair(cb_autoReplot, &Plot::replot)}) {
      ::QObject::connect(cb, &QCheckBox::toggled, this, [slot = std::bind_front(slot, this)](bool checked) {
        if (checked) {
          std::invoke(slot);
        }
      });
    }

    on_cb_autoRegen_toggled(cb_autoRegen->isChecked());
    on_cb_autoReplot_toggled(cb_autoReplot->isChecked());
  }

private:
  using Data = boost::bimap<
    boost::bimaps::set_of<std::size_t>,
    boost::bimaps::multiset_of<std::size_t>,
    boost::bimaps::vector_of_relation>;

private:
  Data                     data_;
  util::ButtonGroup* const plotTypeButtonGroup_ = (setupUi(this), new util::ButtonGroup{gb_plotType});
  int                      genMethodIdxPrev_    = 0;

private:
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
    std::unordered_map<std::size_t, std::size_t> mapTmp;
    for (auto const                  gen = std::invoke(_::property<GenCtor>(genInfo()));
         [[maybe_unused]] auto const i : ranges::views::iota(0, sb_repeatCount->value())) {
      ++mapTmp[gen()];
    }
    for (auto const [key, value] : std::move(mapTmp)) {
      data_.push_back({key, value});
    }
    replot();
  }


  void replot() {
    if (data_.empty()) {
      return;
    }

    qcp_plot->clearPlottables();
    auto const [minX, maxX] = cb_customXRange->isChecked()
      ? std::make_pair(dsb_customXRange_min->value(), dsb_customXRange_max->value())
      : std::make_pair<double, double>(data_.left.begin()->first, std::prev(data_.left.end())->first);

    for (const auto graph = std::invoke(_::property<PlotCtor>(plotTypeButtonGroup_->checkedButton()));
         auto const [key, value] : std::as_const(data_)
           | ranges::views::transform([](auto pair) { return std::make_pair(pair.left, pair.right); })
           | util::rangeScaleView(data_.left.begin()->first, std::prev(data_.left.end())->first, minX, maxX)) {
      std::invoke(graph, key, value);
    }
    setRange(*qcp_plot->xAxis, data_.left.begin()->first, std::prev(data_.left.end())->first);
    setRange(*qcp_plot->yAxis, 0u, std::prev(data_.right.end())->first);

    qcp_plot->replot();
  }


  void sbRepeatCountLimitUpdate() {
    sb_repeatCount->setMaximum(std::invoke([this] {
      switch (_::property<GenConstraint>(genInfo())) {
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


  void on_cb_autoRegen_toggled(bool checked) {
    namespace hana = boost::hana;
    connectToggled(checked,
      hana::make_tuple(sb_rangeMin, sb_rangeMax, sb_repeatCount, cb_genMethod),
      hana::make_map(
        hana::make_pair(hana::type_c<QSpinBox>, qOverload<int>(&QSpinBox::valueChanged)),
        hana::make_pair(hana::type_c<QComboBox>, qOverload<int>(&QComboBox::currentIndexChanged))),
      this, &Plot::generate);
  }


  void on_cb_genMethod_currentIndexChanged(int idx) {
    _::setProperty(genInfo(genMethodIdxPrev_), genMemFromUi());
    setUiFromGenMem(_::property<GenMem>(genInfo(idx)));
    genMethodIdxPrev_ = idx;
    sbRepeatCountLimitUpdate();
  }


  void on_pb_replot_released() {
    replot();
  }


  void on_cb_autoReplot_toggled(bool checked) {
    namespace hana = boost::hana;
    connectToggled(checked,
      hana::make_tuple(
        plotTypeButtonGroup_,
        cb_customXPointCount, sb_customXPointCount,
        cb_customXRange, dsb_customXRange_min, dsb_customXRange_max,
        cb_customYRange, dsb_customYRange_min, dsb_customYRange_max,
        cb_customXScale, dsb_customXScale_min, dsb_customXScale_max,
        cb_customYScale, dsb_customYScale_min, dsb_customYScale_max),
      hana::make_map(
        hana::make_pair(hana::type_c<QCheckBox>, &QCheckBox::toggled),
        hana::make_pair(hana::type_c<QSpinBox>, qOverload<int>(&QSpinBox::valueChanged)),
        hana::make_pair(hana::type_c<QDoubleSpinBox>, qOverload<double>(&QDoubleSpinBox::valueChanged)),
        hana::make_pair(hana::type_c<util::ButtonGroup>, &util::ButtonGroup::buttonToggledTrue)),
      this, &Plot::replot);
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