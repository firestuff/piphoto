#pragma once

#include <array>
#include <cstdint>
#include <numeric>

#include "color.h"
#include "colors.h"
#include "coord.h"
#include "image.h"
#include "lut.h"
#include "minimum.h"

// Maximum LUT size that has each point adjacent to at least one ColorChecker color.
typedef Lut3d<4, 3, 3> ColorCheckerLut3d;

constexpr std::array<Color, 24> kColorCheckerSrgb = {{
  {{{0x7300, 0x5200, 0x4400}}},
  {{{0xc200, 0x9600, 0x8200}}},
  {{{0x6200, 0x7a00, 0x9d00}}},
  {{{0x5700, 0x6c00, 0x4300}}},
  {{{0x8500, 0x8000, 0xb100}}},
  {{{0x6700, 0xbd00, 0xaa00}}},
  {{{0xd600, 0x7e00, 0x2c00}}},
  {{{0x5000, 0x5b00, 0xa600}}},
  {{{0xc100, 0x5a00, 0x6300}}},
  {{{0x5e00, 0x3c00, 0x6c00}}},
  {{{0x9d00, 0xbc00, 0x4000}}},
  {{{0xe000, 0xa300, 0x2e00}}},
  {{{0x3800, 0x3d00, 0x9600}}},
  {{{0x4600, 0x9400, 0x4900}}},
  {{{0xaf00, 0x3600, 0x3c00}}},
  {{{0xe700, 0xc700, 0x1f00}}},
  {{{0xbb00, 0x5600, 0x9500}}},
  {{{0x0800, 0x8500, 0xa100}}},
  {{{0xf300, 0xf300, 0xf200}}},
  {{{0xc800, 0xc800, 0xc800}}},
  {{{0xa000, 0xa000, 0xa000}}},
  {{{0x7a00, 0x7a00, 0x7900}}},
  {{{0x5500, 0x5500, 0x5500}}},
  {{{0x3400, 0x3400, 0x3400}}},
}};

template <uint32_t X, uint32_t Y>
std::array<Coord, kColorCheckerSrgb.size()> FindClosest(const Image<X, Y>& image) {
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

template <uint32_t X, uint32_t Y>
uint32_t ScoreImage(const Image<X, Y>& image) {
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
        }
      }
    }
  }

  return std::accumulate(diff.begin(), diff.end(), UINT32_C(0));
}

template <uint32_t X, uint32_t Y>
std::unique_ptr<Image<X, Y>> HighlightClosest(const Image<X, Y>& image) {
  auto out = std::make_unique<Image<X, Y>>(image);

  auto closest = FindClosest(*out);
  for (uint32_t cc = 0; cc < kColorCheckerSrgb.size(); ++cc) {
    const auto& coord = closest.at(cc);
    const auto& color = kColorCheckerSrgb.at(cc);
    out->DrawSquare({std::max(5U, coord.x) - 5, std::max(5U, coord.y) - 5}, kBlack, 10);
    out->DrawSquare({std::max(6U, coord.x) - 6, std::max(6U, coord.y) - 6}, color, 12);
    out->DrawSquare({std::max(7U, coord.x) - 7, std::max(7U, coord.y) - 7}, color, 14);
    out->DrawSquare({std::max(8U, coord.x) - 8, std::max(8U, coord.y) - 8}, color, 16);
    out->DrawSquare({std::max(9U, coord.x) - 9, std::max(9U, coord.y) - 9}, kWhite, 18);
  }
  return out;
}

template <uint32_t P, uint32_t R, uint32_t G, uint32_t B, uint32_t X, uint32_t Y>
uint32_t OptimizeLut(const Image<X, Y>& image, Lut3d<R, G, B>* lut) {
  uint32_t diff = 0;

  for (uint32_t r = 0; r < R; ++r) {
    auto& rect = lut->at(r);

    for (uint32_t g = 0; g < G; ++g) {
      auto& row = rect.at(g);

      for (uint32_t b = 0; b < B; ++b) {
        auto& color = row.at(b);

        std::cout << "(" << r << ", " << g << ", " << b << ")" << std::endl;

        for (uint32_t c = 0; c < color.size(); ++c) {
          auto min = FindPossibleMinimum<uint32_t, uint32_t, 4>(
            0, UINT16_MAX,
            [&image, &lut, r, g, b, c](uint32_t val) {
              auto test_lut = *lut;
              test_lut[r][g][b].at(c) = val;
              return ScoreImage(*test_lut.MapImage(image));
            });
          std::cout << "\tC" << c << ": " << color.at(c) << " -> " << min << std::endl;
          diff += AbsDiff(color.at(c), min);
          color.at(c) = min;
        }
      }
    }
  }

  return diff;
}
