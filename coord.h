#pragma once

#include <cstdint>
#include <ostream>

#include "array.h"

template <int32_t D>
struct Coord : public Array<int32_t, D> {};

template <int32_t D>
std::ostream& operator<<(std::ostream& os, const Coord<D>& coord) {
  os << "(";
  for (int32_t d = 0; d < D; ++d) {
    os << coord.at(d);
    if (d < D - 1) {
      os << ", ";
    }
  }
  return os << ")";
}
