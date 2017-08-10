#pragma once

#include <array>
#include <cstdint>
#include <iostream>

#include "intmath.h"

constexpr uint32_t kNumColors = UINT16_MAX;

// 32-bit for compiler convenience, but values are 16-bit
struct Color : public std::array<uint32_t, 3> {
  constexpr uint32_t Difference(const Color& other) const;
};

constexpr uint32_t Color::Difference(const Color& other) const {
  return (
    AbsDiff(this->at(0), other.at(0)) +
    AbsDiff(this->at(1), other.at(1)) +
    AbsDiff(this->at(2), other.at(2))
  );
}

std::ostream& operator<<(std::ostream& os, const Color& color);
