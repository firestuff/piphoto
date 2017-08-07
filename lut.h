#pragma once

#include "color.h"

struct Coord3d {
  uint32_t r;
  uint32_t g;
  uint32_t b;
};

template <uint32_t R, uint32_t G, uint32_t B>
class Lut3d : public std::array<std::array<std::array<Color, B>, G>, R> {
 public:
  Color MapColor(const Color& in) const;

 private:
  constexpr static Color InterpolateColor(const Color& i0, const Color& i1, uint32_t mul, uint32_t div);
  constexpr static uint32_t Interpolate(uint32_t i0, uint32_t i1, uint32_t mul, uint32_t div);

  // Return value is (root_indices, remainders)
  constexpr static std::pair<Coord3d, Coord3d> FindRoot(const Color& in);
  constexpr static std::pair<uint32_t, uint32_t> FindChannelRoot(uint32_t value, uint32_t points);

  constexpr static uint32_t BlockSize(uint32_t points);
};

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
constexpr Color Lut3d<R, G, B>::InterpolateColor(const Color& i0, const Color& i1, uint32_t mul, uint32_t div) {
  return {
    Interpolate(i0.r, i1.r, mul, div),
    Interpolate(i0.g, i1.g, mul, div),
    Interpolate(i0.b, i1.b, mul, div),
  };
}

template <uint32_t R, uint32_t G, uint32_t B>
constexpr uint32_t Lut3d<R, G, B>::Interpolate(uint32_t i0, uint32_t i1, uint32_t mul, uint32_t div) {
  return i0 + ((mul * i1) / div);
}

template <uint32_t R, uint32_t G, uint32_t B>
constexpr std::pair<Coord3d, Coord3d> Lut3d<R, G, B>::FindRoot(const Color& in) {
  auto root_r = FindChannelRoot(in.r, R);
  auto root_g = FindChannelRoot(in.r, G);
  auto root_b = FindChannelRoot(in.r, B);
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
  return std::make_pair(std::min(points - 2, value / BlockSize(points)), value % BlockSize(points));;
}

template <uint32_t R, uint32_t G, uint32_t B>
constexpr uint32_t BlockSize(uint32_t points) {
  return kNumColors / (points - 1);
}
