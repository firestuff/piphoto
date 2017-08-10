#pragma once

#include <cstdint>
#include <ostream>

template <uint32_t D>
struct Coord : public std::array<uint32_t, D> {};

template <uint32_t D>
std::ostream& operator<<(std::ostream& os, const Coord<D>& coord) {
  os << "(";
  for (uint32_t d = 0; d < D; ++d) {
    os << coord.at(d);
    if (d < D - 1) {
      os << ", ";
    }
  }
  return os << ")";
}
