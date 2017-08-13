#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>

#include "array.h"
#include "intmath.h"

constexpr int32_t kMinColor = 0;
constexpr int32_t kMaxColor = UINT16_MAX;
constexpr int32_t kNumColors = (kMaxColor - kMinColor) + 1;

class ColorBase {};

template <int32_t C>
struct Color : public Array<int32_t, C>, public ColorBase {
  constexpr int32_t AbsDiff(const Color<C>& other) const;
  constexpr Color<C> Interpolate(const Color<C>& other, int32_t mul, int32_t div) const;
  constexpr Color<C> Crop() const;
};


struct RgbColor;
struct HsvColor;


struct RgbColor : public Color<3> {
  RgbColor() = default;
  RgbColor(const Color<3>& src);

  operator HsvColor() const;
};


struct HsvColor : public Color<3> {
  HsvColor() = default;
  HsvColor(const Color<3>& src);
};


template <int32_t C>
constexpr int32_t Color<C>::AbsDiff(const Color<C>& other) const {
  int32_t diff = 0;
  for (int32_t c = 0; c < C; ++c) {
    diff += ::AbsDiff(this->at(c), other.at(c));
  }
  return diff;
}

template <int32_t C>
constexpr Color<C> Color<C>::Interpolate(const Color<C>& other, int32_t mul, int32_t div) const {
  Color<C> ret;
  for (int32_t c = 0; c < C; ++c) {
    ret.at(c) = ::Interpolate(this->at(c), other.at(c), mul, div);
  }
  return ret;
}

template <int32_t C>
constexpr Color<C> Color<C>::Crop() const {
  Color<C> ret;
  for (int32_t c = 0; c < C; ++c) {
    ret.at(c) = std::max(kMinColor, std::min(kMaxColor, this->at(c)));
  }
  return ret;
}

template <int32_t C>
std::ostream& operator<<(std::ostream& os, const Color<C>& color) {
  os << std::hex << std::setfill('0') << "Color(";
  for (int32_t c = 0; c < C; ++c) {
    os << "0x" << std::setw(4) << color.at(c);
    if (c < C - 1) {
      os << ", ";
    }
  }
  return os << ")" << std::dec;
}
