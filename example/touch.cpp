#include "randomUniq/UniformIntDistribution.hpp"
#include <iostream>


int main() {
  urand::UniformIntDistribution<short> rgInt;
  for (std::size_t i{}; i < 100000; ++i) {
    std::cout << rgInt() << '\n';
  }
  return 0;
}
