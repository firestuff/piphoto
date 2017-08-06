#pragma once

#include <cstdint>
#include <ostream>

struct Coord {
  uint32_t x;
  uint32_t y;
};

std::ostream& operator<<(std::ostream& os, const Coord& coord);
