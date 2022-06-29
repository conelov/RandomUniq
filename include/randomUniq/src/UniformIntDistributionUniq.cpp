//
// Created by dym on 29.06.22.
//

#include "randomUniq/UniformIntDistributionUniq.hpp"
#include "randomUniq/util/RandomDevice.hpp"
#include <gsl/util>
#include <range/v3/view/counted.hpp>


/*
{
min: 1,
max: 100
chance: 1
}
// -> 50

{
min: 1
max: 49
chance: 100 / all_number * count_number_in
}

{
min: 51
max: 100
chance: 100 / all_number * count_number_in
}

// -> 25

{

 */


namespace urand {


UniformIntDistributionUniq::UniformIntDistributionUniq(UniformIntDistributionUniq::value_type min, UniformIntDistributionUniq::value_type max)
    : totalCounter_(max - min + 1) {
  if (min > max) {
    throw std::invalid_argument("min > max");
  }
  ranges_.push_back({min, max});
}


UniformIntDistributionUniq::value_type UniformIntDistributionUniq::get() {
  if (!totalCounter_) {
    throw std::runtime_error("no contained number");
  }
  auto const _       = gsl::finally([this] { --totalCounter_; });
  auto const itRange = std::next(ranges_.begin(),
    std::uniform_int_distribution<std::size_t>(0, ranges_.size())(util::RandomDevice<std::mt19937>::get()));

  if (itRange->min == itRange->max) {
    auto const result = itRange->min;
    ranges_.erase(itRange);
    return result;
  }
  value_type const result = std::uniform_int_distribution<value_type>(itRange->min, itRange->max)(util::RandomDevice<std::mt19937>::get());

  if (itRange->min == result) {
    return itRange->min++;
  }
  if (itRange->max == result) {
    return itRange->max--;
  }

  {
    Range newRange{itRange->min, result - 1};
    itRange->min = result + 1;
    ranges_.push_back(std::move(newRange));
  }

  return result;
}


UniformIntDistributionUniq::value_type UniformIntDistributionUniq::operator()() {
  return this->get();
}


std::size_t UniformIntDistributionUniq::empty() const noexcept {
  return ranges_.empty();
}


UniformIntDistributionUniq::operator bool() const noexcept {
  return this->empty();
}

}// namespace urand