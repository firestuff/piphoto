#pragma once

#include <cstdint>

constexpr uint32_t kNumColors = (1 << 16);

struct Color {
  // 32-bit for compiler convenience, but values are 16-bit
  uint32_t r;
  uint32_t g;
  uint32_t b;

  constexpr uint32_t Difference(const Color& other) const;
};

constexpr uint32_t Color::Difference(const Color& other) const {
  return (
    ((r > other.r) ? (r - other.r) : (other.r - r)) +
    ((g > other.g) ? (g - other.g) : (other.g - g)) +
    ((b > other.b) ? (b - other.b) : (other.b - b))
  );
}
