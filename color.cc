#include "color.h"

#include <iomanip>

std::ostream& operator<<(std::ostream& os, const Color& color) {
  return os
    << std::hex << std::setfill('0')
    << "rgb("
    << "0x" << std::setw(4) << color.r << ", "
    << "0x" << std::setw(4) << color.g << ", "
    << "0x" << std::setw(4) << color.b
    << ")" << std::dec;
}
