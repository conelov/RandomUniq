#pragma once

#include <vector>


namespace urand {

class UniformIntDistributionUniq final {
public:
  using value_type = int;

public:
  UniformIntDistributionUniq(value_type min = std::numeric_limits<value_type>::min(),
    value_type                          max = std::numeric_limits<value_type>::max());

  // chance: (1 / all_number) * count_number_in


  value_type get();
  value_type operator()();

  operator bool() const noexcept;

  std::size_t empty() const noexcept;

private:
  struct Range {
    value_type min;
    value_type max;

    std::size_t size() const noexcept;
    double chance(std::size_t total) const noexcept;
    //    std::size_t count_number;
    //    float      chance;
  };
  //std::size_t count_number;

  // ranges -> vector<struct{min, max}>
  // range -> struct{min, max}
  std::vector<Range> ranges_;
  std::size_t        totalCounter_;
};
}// namespace urand