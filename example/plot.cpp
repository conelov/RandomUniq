//
// Created by dym on 29.06.22.
//

#include "example/util/generator.hpp"
#include "ui_FormMain.h"
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/vector_of.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <QAbstractListModel>
#include <QApplication>
#include <unordered_map>


namespace {

void setVisibleWidget(bool visible, QWidget* w1, auto*... ws) {
  w1->setVisible(visible);
  (ws->setVisible(visible), ...);
}


class GenMethodModel final : public QAbstractListModel {
  Q_OBJECT
public:
  enum MethodIndex {
    Index = Qt::UserRole,
    Name,
    Ctor
  };


  enum GenRepeatMode : std::uint8_t {
    Limited,
    NonLimited
  };

public:
  struct Method final {
    using Ctor = std::function<void()> (*)(int, int);

    QString       name;
    Ctor          ctor;
    GenRepeatMode repeatMode;
  };

public:
  GenMethodModel(QObject& parent)
      : QAbstractListModel(&parent) {
  }


  int rowCount(const QModelIndex& parent) const override {
    return data_.size();
  }


  QVariant data(const QModelIndex& index, int role) const override;

private:
  std::array<Method, 4> const data_{
    Method{
      "UniformIntDistributionUniq LinearDoobleGen",
      [](auto l, auto r) -> std::function<void()> {
        return urand::plot::util::uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::LinearDoobleGen>(l, r);
      },
      Limited},
    Method{"UniformIntDistributionUniq NonLinearEqualChanceRange",
      [](auto l, auto r) -> std::function<void()> {
        return urand::plot::util::uniformIntDistributionUniqAtType<urand::UniformIntDistributionUniqGenType::NonLinearEqualChanceRange>(l, r);
      },
      Limited},
    Method{"linear",
      [](auto l, auto r) -> std::function<void()> {
        return urand::plot::util::uniformIntDistributionLinearMid<1>(l, r);
      },
      NonLimited},
    Method{"linear x3",
      [](auto l, auto r) -> std::function<void()> {
        return urand::plot::util::uniformIntDistributionLinearMid<3>(l, r);
      },
      NonLimited}};
};
}// namespace

Q_DECLARE_METATYPE(GenMethodModel::Method::Ctor)


namespace {

QVariant GenMethodModel::data(const QModelIndex& index, int role) const {
  if (index.isValid()) {
    switch (role) {
      case Name:
      case Qt::DisplayRole:
        return data_[index.row()].name;
      case Index:
        return index.row();
      case Ctor:
        return QVariant::fromValue(data_[index.row()].ctor);
    }
  }

  return {};
}


class Plot final : public QMainWindow
    , private Ui::FormMain {
  Q_OBJECT
public:
  Plot() {
    setupUi(this);
    setWindowTitle(BOOST_PP_STRINGIZE(PROJECT_NAME));

    cb_genMethod->clear();
    //    boost::hana::for_each(boost::hana::keys(genMethodType_), [this](auto&& str) {
    //      cb_genMethod->addItem(std::forward<decltype(str)>(str).c_str());
    //    });

    on_cb_customXScale_toggled(cb_customXScale->isChecked());
    on_cb_customYScale_toggled(cb_customYScale->isChecked());
  }

private:
  using Data = boost::bimap<
    boost::bimaps::set_of<std::size_t>,
    boost::bimaps::multiset_of<std::size_t>,
    boost::bimaps::vector_of_relation>;

private:
  Data                  data_;
  GenMethodModel* const genMethodModel = new GenMethodModel{*this};

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
  void on_cb_customXScale_toggled(bool checked) {
    setVisibleWidget(checked, f_customXScale);
  }


  void on_cb_customYScale_toggled(bool checked) {
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