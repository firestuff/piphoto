#pragma once

#include <array>

#include "color.h"
#include "coord.h"
#include "image.h"
#include "lut.h"

typedef Lut3d<4, 3, 3> ColorCheckerLut3d;

constexpr std::array<Color, 24> kColorCheckerSrgb = {{
  {0x7300, 0x5200, 0x4400},
  {0xc200, 0x9600, 0x8200},
  {0x6200, 0x7a00, 0x9d00},
  {0x5700, 0x6c00, 0x4300},
  {0x8500, 0x8000, 0xb100},
  {0x6700, 0xbd00, 0xaa00},
  {0xd600, 0x7e00, 0x2c00},
  {0x5000, 0x5b00, 0xa600},
  {0xc100, 0x5a00, 0x6300},
  {0x5e00, 0x3c00, 0x6c00},
  {0x9d00, 0xbc00, 0x4000},
  {0xe000, 0xa300, 0x2e00},
  {0x3800, 0x3d00, 0x9600},
  {0x4600, 0x9400, 0x4900},
  {0xaf00, 0x3600, 0x3c00},
  {0xe700, 0xc700, 0x1f00},
  {0xbb00, 0x5600, 0x9500},
  {0x0800, 0x8500, 0xa100},
  {0xf300, 0xf300, 0xf200},
  {0xc800, 0xc800, 0xc800},
  {0xa000, 0xa000, 0xa000},
  {0x7a00, 0x7a00, 0x7900},
  {0x5500, 0x5500, 0x5500},
  {0x3400, 0x3400, 0x3400},
}};

template <uint32_t X, uint32_t Y>
std::array<Coord, kColorCheckerSrgb.size()> ColorCheckerClosest(const Image<X, Y>& image) {
  std::array<Coord, kColorCheckerSrgb.size()> closest;
  std::array<uint32_t, kColorCheckerSrgb.size()> diff;
  diff.fill(UINT32_MAX);

  for (uint32_t y = 0; y < Y; ++y) {
    const auto& row = image.at(y);

    for (uint32_t x = 0; x < X; ++x) {
      const auto& pixel = row.at(x);

      for (uint32_t cc = 0; cc < kColorCheckerSrgb.size(); ++cc) {
        auto pixel_diff = pixel.Difference(kColorCheckerSrgb.at(cc));
        if (pixel_diff < diff.at(cc)) {
          diff.at(cc) = pixel_diff;
          closest.at(cc) = {x, y};
        }
      }
    }
  }

  return closest;
}
