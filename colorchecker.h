#pragma once

#include <cstdint>
#include <numeric>

#include "array.h"
#include "color.h"
#include "colors.h"
#include "coord.h"
#include "image.h"
#include "lut.h"
#include "minimum.h"

// Maximum LUT size that has each point adjacent to at least one ColorChecker color.
typedef Lut3d<4, 3, 3> ColorCheckerLut3d;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
const Array<RgbColor, 24> kColorCheckerSrgb = {{{
  {{{{{0x7300, 0x5200, 0x4400}}}}},
  {{{{{0xc200, 0x9600, 0x8200}}}}},
  {{{{{0x6200, 0x7a00, 0x9d00}}}}},
  {{{{{0x5700, 0x6c00, 0x4300}}}}},
  {{{{{0x8500, 0x8000, 0xb100}}}}},
  {{{{{0x6700, 0xbd00, 0xaa00}}}}},
  {{{{{0xd600, 0x7e00, 0x2c00}}}}},
  {{{{{0x5000, 0x5b00, 0xa600}}}}},
  {{{{{0xc100, 0x5a00, 0x6300}}}}},
  {{{{{0x5e00, 0x3c00, 0x6c00}}}}},
  {{{{{0x9d00, 0xbc00, 0x4000}}}}},
  {{{{{0xe000, 0xa300, 0x2e00}}}}},
  {{{{{0x3800, 0x3d00, 0x9600}}}}},
  {{{{{0x4600, 0x9400, 0x4900}}}}},
  {{{{{0xaf00, 0x3600, 0x3c00}}}}},
  {{{{{0xe700, 0xc700, 0x1f00}}}}},
  {{{{{0xbb00, 0x5600, 0x9500}}}}},
  {{{{{0x0800, 0x8500, 0xa100}}}}},
  {{{{{0xf300, 0xf300, 0xf200}}}}},
  {{{{{0xc800, 0xc800, 0xc800}}}}},
  {{{{{0xa000, 0xa000, 0xa000}}}}},
  {{{{{0x7a00, 0x7a00, 0x7900}}}}},
  {{{{{0x5500, 0x5500, 0x5500}}}}},
  {{{{{0x3400, 0x3400, 0x3400}}}}},
}}};
#pragma clang diagnostic pop

template <int32_t X, int32_t Y>
Array<Coord<2>, kColorCheckerSrgb.size()> FindClosest(const Image<X, Y, RgbColor>& image) {
  Array<Coord<2>, kColorCheckerSrgb.size()> closest;
  Array<int32_t, kColorCheckerSrgb.size()> diff;
  diff.fill(INT32_MAX);

  for (int32_t y = 0; y < Y; ++y) {
    const auto& row = image.at(y);

    for (int32_t x = 0; x < X; ++x) {
      const auto& pixel = row.at(x);

      for (int32_t cc = 0; cc < kColorCheckerSrgb.ssize(); ++cc) {
        auto pixel_diff = pixel.AbsDiff(kColorCheckerSrgb.at(cc));
        if (pixel_diff < diff.at(cc)) {
          diff.at(cc) = pixel_diff;
          closest.at(cc) = {{{{x, y}}}};
        }
      }
    }
  }

  return closest;
}

template <int32_t X, int32_t Y>
int32_t ScoreLut(const Image<X, Y, RgbColor>& image, const LutBase& lut) {
  Array<int32_t, kColorCheckerSrgb.size()> diff;
  diff.fill(INT32_MAX);

  image.ForEach([&diff, &lut](const RgbColor& color) {
    const auto pixel = lut.MapColor(color);
    for (int32_t cc = 0; cc < kColorCheckerSrgb.ssize(); ++cc) {
      auto pixel_diff = pixel.AbsDiff(kColorCheckerSrgb.at(cc));
      if (pixel_diff < diff.at(cc)) {
        diff.at(cc) = pixel_diff;
      }
    }
  });

  return std::accumulate(diff.begin(), diff.end(), 0);
}

template <int32_t X, int32_t Y>
std::unique_ptr<Image<X, Y, RgbColor>> HighlightClosest(const Image<X, Y, RgbColor>& image) {
  auto out = std::make_unique<Image<X, Y, RgbColor>>(image);

  auto closest = FindClosest(*out);
  for (int32_t cc = 0; cc < kColorCheckerSrgb.ssize(); ++cc) {
    const auto& coord = closest.at(cc);
    const auto& color = kColorCheckerSrgb.at(cc);
    out->DrawSquare({{{{coord.at(0) - 5, coord.at(1) - 5}}}}, kBlack, 10);
    out->DrawSquare({{{{coord.at(0) - 6, coord.at(1) - 6}}}}, color, 12);
    out->DrawSquare({{{{coord.at(0) - 7, coord.at(1) - 7}}}}, color, 14);
    out->DrawSquare({{{{coord.at(0) - 8, coord.at(1) - 8}}}}, color, 16);
    out->DrawSquare({{{{coord.at(0) - 9, coord.at(1) - 9}}}}, kWhite, 18);
  }
  return out;
}

template <int32_t LUT_X, int32_t LUT_Y, int32_t LUT_Z, int32_t IMG_X, int32_t IMG_Y>
int32_t OptimizeLut(const ImageColorBase<RgbColor>& image, Lut3d<LUT_X, LUT_Y, LUT_Z>* lut) {
  auto snapshot = *lut;
  int32_t diff = 0;

  for (int32_t x = 0; x < LUT_X; ++x) {
    auto& rect = lut->at(x);

    for (int32_t y = 0; y < LUT_Y; ++y) {
      auto& row = rect.at(y);

      for (int32_t z = 0; z < LUT_Z; ++z) {
        auto& color = row.at(z);

        std::cout << Coord<3>{{{{x, y, z}}}} << std::endl;

        for (int32_t c = 0; c < color.size(); ++c) {
          auto& channel = color.at(c);

          auto min = FindPossibleMinimum<int32_t, int32_t, 8>(
            -UINT16_MAX, UINT16_MAX * 2,
            [&image, &snapshot, x, y, z, c](int32_t val) {
              auto test_lut = snapshot;
              test_lut.at(x).at(y).at(z).at(c) = val;
              return ScoreLut(image, test_lut);
            });
          // Magic value of 8 is the number of points making up a square, so the number
          // of points that control any given given LUT mapping.
          auto new_value = Interpolate(channel, min, INT32_C(1), INT32_C(8));
          std::cout << "\tC" << c << ": " << channel << " -> " << new_value << " (interpolated from " << min << ")" << std::endl;
          diff += AbsDiff(channel, new_value);
          channel = new_value;
        }
      }
    }
  }

  return diff;
}

template <int32_t LUT_X, int32_t IMG_X, int32_t IMG_Y>
int32_t OptimizeLut(const Image<IMG_X, IMG_Y, RgbColor>& image, Lut1d<LUT_X>* lut) {
  auto snapshot = *lut;
  int32_t diff = 0;

  for (int32_t x = 0; x < LUT_X; ++x) {
    auto& color = lut->at(x);

    std::cout << Coord<1>{{{{x}}}} << std::endl;

    for (int32_t c = 0; c < color.ssize(); ++c) {
      auto& channel = color.at(c);

      auto min = FindPossibleMinimum<int32_t, int32_t, 8>(
        -UINT16_MAX, UINT16_MAX * 2,
        [&image, &snapshot, x, c](int32_t val) {
          auto test_lut = snapshot;
          test_lut.at(x).at(c) = val;
          return ScoreLut(image, test_lut);
        });
      // Magic value of 8 is the number of points making up a square, so the number
      // of points that control any given given LUT mapping.
      auto new_value = Interpolate(channel, min, INT32_C(1), INT32_C(8));
      std::cout << "\tC" << c << ": " << channel << " -> " << new_value << " (interpolated from " << min << ")" << std::endl;
      diff += AbsDiff(channel, new_value);
      channel = new_value;
    }
  }

  return diff;
}
