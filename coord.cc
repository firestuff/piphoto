#include "coord.h"

std::ostream& operator<<(std::ostream& os, const Coord& coord) {
  return os << "(" << coord.x << ", " << coord.y << ")";
}

