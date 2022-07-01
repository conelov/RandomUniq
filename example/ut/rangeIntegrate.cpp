//
// Created by dym on 01.07.22.
//

#include "example/util/rangeScaleView.hpp"
#include <gtest/gtest.h>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/linear_distribute.hpp>
#include <range/v3/view/zip.hpp>


TEST(PROJECT_NAME_RandomUniq, rangeIntegrate) {
  std::map<std::size_t, std::size_t> source;
  for (auto const pair : ranges::views::zip(
         ranges::views::iota(90, 100), ranges::views::linear_distribute(100, 1000, 10))) {
    source.emplace(pair);
  }

  for (auto const [key, value] : source) {
    std::cout << key << "\t\t\t" << value << '\n';
  }

  std::cout << '\n';

  for (auto const [key, value] : source | urand::plot::util::rangeScaleView(source.begin()->first, std::prev(source.end())->first, source.size(), 0, 1)) {
    std::cout << key << "\t\t" << value << '\n';
  }
}