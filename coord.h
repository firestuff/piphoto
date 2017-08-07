#pragma once

#include <cstdint>
#include <ostream>

struct Coord {
  uint32_t x;
  uint32_t y;
};

struct Coord3d {
  uint32_t r;
  uint32_t g;
  uint32_t b;
};

std::ostream& operator<<(std::ostream& os, const Coord& coord);
