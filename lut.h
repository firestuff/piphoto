#pragma once

#include "color.h"
#include "coord.h"

template <uint32_t R, uint32_t G, uint32_t B>
class Lut3d : public std::array<std::array<std::array<Color, B>, G>, R> {
 public:
  static Lut3d<R, G, B> Identity();

  Color MapColor(const Color& in) const;

  template <uint32_t X, uint32_t Y>
  std::unique_ptr<Image<X, Y>> MapImage(const Image<X, Y>& in) const;

 private:
  constexpr static Color InterpolateColor(const Color& i0, const Color& i1, uint32_t mul, uint32_t div);
  constexpr static uint32_t Interpolate(uint32_t i0, uint32_t i1, uint32_t mul, uint32_t div);

  // Return value is (root_indices, remainders)
  constexpr static std::pair<Coord3d, Coord3d> FindRoot(const Color& in);
  constexpr static std::pair<uint32_t, uint32_t> FindChannelRoot(uint32_t value, uint32_t points);

  constexpr static uint32_t BlockSize(uint32_t points);
};

// Minimum size LUT
typedef Lut3d<2, 2, 2> MinimalLut3d;

template <uint32_t R, uint32_t G, uint32_t B>
Lut3d<R, G, B> Lut3d<R, G, B>::Identity() {
  Lut3d<R, G, B> ret;

  Color color;
  for (uint32_t r = 0; r < R; ++r) {
    auto& rect = ret.at(r);
    color.at(0) = std::min(kNumColors - 1, BlockSize(R) * r);

    for (uint32_t g = 0; g < G; ++g) {
      auto& row = rect.at(g);
      color.at(1) = std::min(kNumColors - 1, BlockSize(G) * g);

      for (uint32_t b = 0; b < B; ++b) {
        color.at(2) = std::min(kNumColors - 1, BlockSize(B) * b);
        row.at(b) = color;
      }
    }
  }

  return ret;
}

template <uint32_t R, uint32_t G, uint32_t B>
Color Lut3d<R, G, B>::MapColor(const Color& in) const {
  const auto root_rem = FindRoot(in);
  const auto& root = root_rem.first;
  const auto& rem = root_rem.second;

  // https://en.wikipedia.org/wiki/Trilinear_interpolation
  auto inter00 = InterpolateColor(
    this->at(root.r + 0).at(root.g + 0).at(root.b + 0),
    this->at(root.r + 1).at(root.g + 0).at(root.b + 0),
    rem.r,
    BlockSize(R));

  auto inter01 = InterpolateColor(
    this->at(root.r + 0).at(root.g + 0).at(root.b + 1),
    this->at(root.r + 1).at(root.g + 0).at(root.b + 1),
    rem.r,
    BlockSize(R));

  auto inter10 = InterpolateColor(
    this->at(root.r + 0).at(root.g + 1).at(root.b + 0),
    this->at(root.r + 1).at(root.g + 1).at(root.b + 0),
    rem.r,
    BlockSize(R));

  auto inter11 = InterpolateColor(
    this->at(root.r + 0).at(root.g + 1).at(root.b + 1),
    this->at(root.r + 1).at(root.g + 1).at(root.b + 1),
    rem.r,
    BlockSize(R));

  auto inter0 = InterpolateColor(inter00, inter10, rem.g, BlockSize(G));
  auto inter1 = InterpolateColor(inter01, inter11, rem.g, BlockSize(G));

  return InterpolateColor(inter0, inter1, rem.b, BlockSize(B));
}

template <uint32_t R, uint32_t G, uint32_t B>
template <uint32_t X, uint32_t Y>
std::unique_ptr<Image<X, Y>> Lut3d<R, G, B>::MapImage(const Image<X, Y>& in) const {
  auto out = std::make_unique<Image<X, Y>>();

  for (uint32_t y = 0; y < Y; ++y) {
    for (uint32_t x = 0; x < X; ++x) {
      Coord coord = {x, y};
      out->SetPixel(coord, MapColor(in.GetPixel(coord)));
    }
  }

  return out;
}

template <uint32_t R, uint32_t G, uint32_t B>
constexpr Color Lut3d<R, G, B>::InterpolateColor(const Color& i0, const Color& i1, uint32_t mul, uint32_t div) {
  return {{{
    Interpolate(i0.at(0), i1.at(0), mul, div),
    Interpolate(i0.at(1), i1.at(1), mul, div),
    Interpolate(i0.at(2), i1.at(2), mul, div),
  }}};
}

template <uint32_t R, uint32_t G, uint32_t B>
constexpr uint32_t Lut3d<R, G, B>::Interpolate(uint32_t i0, uint32_t i1, uint32_t mul, uint32_t div) {
  if (i1 > i0) {
    return i0 + ((mul * (i1 - i0)) / div);
  } else {
    return i1 + ((mul * (i0 - i1)) / div);
  }
}

template <uint32_t R, uint32_t G, uint32_t B>
constexpr std::pair<Coord3d, Coord3d> Lut3d<R, G, B>::FindRoot(const Color& in) {
  auto root_r = FindChannelRoot(in.at(0), R);
  auto root_g = FindChannelRoot(in.at(1), G);
  auto root_b = FindChannelRoot(in.at(2), B);
  return {
    {root_r.first, root_g.first, root_b.first},
    {root_r.second, root_g.second, root_b.second},
  };
}

template <uint32_t R, uint32_t G, uint32_t B>
constexpr std::pair<uint32_t, uint32_t> Lut3d<R, G, B>::FindChannelRoot(const uint32_t value, const uint32_t points) {
  // points - 1 is the last point index. Since we're going to fidn the cube
  // around this point by adding to the root, we need to be at least 1 less
  // than that.
  uint32_t index = std::min(points - 2, value / BlockSize(points));
  return std::make_pair(index, value - (index * BlockSize(points)));
}

template <uint32_t R, uint32_t G, uint32_t B>
constexpr uint32_t Lut3d<R, G, B>::BlockSize(uint32_t points) {
  return kNumColors / (points - 1);
}
