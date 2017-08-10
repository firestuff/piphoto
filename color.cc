#include "color.h"

#include <iomanip>

std::ostream& operator<<(std::ostream& os, const Color& color) {
  return os
    << std::hex << std::setfill('0')
    << "rgb("
    << "0x" << std::setw(4) << color.at(0) << ", "
    << "0x" << std::setw(4) << color.at(1) << ", "
    << "0x" << std::setw(4) << color.at(2)
    << ")" << std::dec;
}
