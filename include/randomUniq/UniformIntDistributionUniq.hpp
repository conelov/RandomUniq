#pragma once

#include <unordered_set>


namespace urand {

class UniformIntDistributionUniq final {
public:
  UniformIntDistributionUniq(int min, int max);

  int operator()();

private:
  struct Range;

  std::unordered_set<int> set_;
};
}// namespace urand