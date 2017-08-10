#pragma once

#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>

#include "intmath.h"

constexpr uint32_t kMaxColor = UINT16_MAX;

// 32-bit for compiler convenience, but values are 16-bit
template <uint32_t C>
struct Color : public std::array<uint32_t, C> {
  constexpr uint32_t Difference(const Color<C>& other) const;
  constexpr Color<C> Interpolate(const Color<C>& other, uint32_t mul, uint32_t div) const;
};

struct RgbColor : public Color<3> {};

template <uint32_t C>
constexpr uint32_t Color<C>::Difference(const Color<C>& other) const {
  uint32_t diff = 0;
  for (uint32_t c = 0; c < C; ++c) {
    diff += AbsDiff(this->at(c), other.at(c));
  }
  return diff;
}

template <uint32_t C>
constexpr Color<C> Color<C>::Interpolate(const Color<C>& other, uint32_t mul, uint32_t div) const {
  Color<C> ret;
  for (uint32_t c = 0; c < C; ++c) {
    ret.at(c) = ::Interpolate(this->at(c), other.at(c), mul, div);
  }
  return ret;
}

template <uint32_t C>
std::ostream& operator<<(std::ostream& os, const Color<C>& color) {
  os << std::hex << std::setfill('0') << "Color(";
  for (uint32_t c = 0; c < C; ++c) {
    os << "0x" << std::setw(4) << color.at(0);
    if (c < C - 1) {
      os << ", ";
    }
  }
  return os << ")" << std::dec;
}
