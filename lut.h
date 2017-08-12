#pragma once

#include "color.h"
#include "coord.h"

// Hardcoded to Color<3>, so color dimensions == LUT dimensions
template <uint32_t X, uint32_t Y, uint32_t Z>
class Lut3d : public std::array<std::array<std::array<Color<3>, X>, Y>, Z> {
 public:
  static Lut3d<X, Y, Z> Identity();

  Color<3> MapColor(const Color<3>& in) const;

  template <uint32_t IMG_X, uint32_t IMG_Y, uint32_t C>
  std::unique_ptr<Image<IMG_X, IMG_Y, C>> MapImage(const Image<IMG_X, IMG_Y, C>& in) const;

 private:
  // Return value is (root_indices, remainders)
  constexpr static std::pair<Coord<3>, Coord<3>> FindRoot(const Color<3>& in);
  constexpr static std::pair<uint32_t, uint32_t> FindChannelRoot(uint32_t value, uint32_t points);

  constexpr static uint32_t BlockSize(uint32_t points);
};

// Minimum size LUT
typedef Lut3d<2, 2, 2> MinimalLut3d;

template <uint32_t X, uint32_t Y, uint32_t Z>
Lut3d<X, Y, Z> Lut3d<X, Y, Z>::Identity() {
  Lut3d<X, Y, Z> ret;

  Color<3> color;
  for (uint32_t x = 0; x < X; ++x) {
    auto& rect = ret.at(x);
    color.at(0) = std::min(kMaxColor, static_cast<int32_t>(BlockSize(X) * x));

    for (uint32_t y = 0; y < Y; ++y) {
      auto& row = rect.at(y);
      color.at(1) = std::min(kMaxColor, static_cast<int32_t>(BlockSize(Y) * y));

      for (uint32_t z = 0; z < Z; ++z) {
        color.at(2) = std::min(kMaxColor, static_cast<int32_t>(BlockSize(Z) * z));
        row.at(z) = color;
      }
    }
  }

  return ret;
}

template <uint32_t X, uint32_t Y, uint32_t Z>
Color<3> Lut3d<X, Y, Z>::MapColor(const Color<3>& in) const {
  const auto root_rem = FindRoot(in);
  const auto& root = root_rem.first;
  const auto& rem = root_rem.second;

  // https://en.wikipedia.org/wiki/Trilinear_interpolation
  auto inter00 =
    this->at(root.at(0) + 0).at(root.at(1) + 0).at(root.at(2) + 0).Interpolate(
    this->at(root.at(0) + 1).at(root.at(1) + 0).at(root.at(2) + 0),
    static_cast<int32_t>(rem.at(0)),
    BlockSize(X));

  auto inter01 =
    this->at(root.at(0) + 0).at(root.at(1) + 0).at(root.at(2) + 1).Interpolate(
    this->at(root.at(0) + 1).at(root.at(1) + 0).at(root.at(2) + 1),
    static_cast<int32_t>(rem.at(0)),
    BlockSize(X));

  auto inter10 =
    this->at(root.at(0) + 0).at(root.at(1) + 1).at(root.at(2) + 0).Interpolate(
    this->at(root.at(0) + 1).at(root.at(1) + 1).at(root.at(2) + 0),
    static_cast<int32_t>(rem.at(0)),
    BlockSize(X));

  auto inter11 =
    this->at(root.at(0) + 0).at(root.at(1) + 1).at(root.at(2) + 1).Interpolate(
    this->at(root.at(0) + 1).at(root.at(1) + 1).at(root.at(2) + 1),
    static_cast<int32_t>(rem.at(0)),
    BlockSize(X));

  auto inter0 = inter00.Interpolate(inter10, static_cast<int32_t>(rem.at(1)), BlockSize(Y));
  auto inter1 = inter01.Interpolate(inter11, static_cast<int32_t>(rem.at(1)), BlockSize(Y));

  return inter0.Interpolate(inter1, static_cast<int32_t>(rem.at(2)), BlockSize(Z)).Crop();
}

template <uint32_t X, uint32_t Y, uint32_t Z>
template <uint32_t IMG_X, uint32_t IMG_Y, uint32_t C>
std::unique_ptr<Image<IMG_X, IMG_Y, C>> Lut3d<X, Y, Z>::MapImage(const Image<IMG_X, IMG_Y, C>& in) const {
  auto out = std::make_unique<Image<IMG_X, IMG_Y, C>>();

  for (uint32_t y = 0; y < IMG_Y; ++y) {
    for (uint32_t x = 0; x < IMG_X; ++x) {
      Coord<2> coord = {{{x, y}}};
      out->SetPixel(coord, MapColor(in.GetPixel(coord)));
    }
  }

  return out;
}

template <uint32_t X, uint32_t Y, uint32_t Z>
constexpr std::pair<Coord<3>, Coord<3>> Lut3d<X, Y, Z>::FindRoot(const Color<3>& in) {
  auto root_x = FindChannelRoot(static_cast<uint32_t>(in.at(0)), X);
  auto root_y = FindChannelRoot(static_cast<uint32_t>(in.at(1)), Y);
  auto root_z = FindChannelRoot(static_cast<uint32_t>(in.at(2)), Z);
  return {
    {{{root_x.first, root_y.first, root_z.first}}},
    {{{root_x.second, root_y.second, root_z.second}}},
  };
}

template <uint32_t X, uint32_t Y, uint32_t Z>
constexpr std::pair<uint32_t, uint32_t> Lut3d<X, Y, Z>::FindChannelRoot(const uint32_t value, const uint32_t points) {
  // points - 1 is the last point index. Since we're going to fidn the cube
  // around this point by adding to the root, we need to be at least 1 less
  // than that.
  uint32_t index = std::min(points - 2, value / BlockSize(points));
  return std::make_pair(index, value - (index * BlockSize(points)));
}

template <uint32_t X, uint32_t Y, uint32_t Z>
constexpr uint32_t Lut3d<X, Y, Z>::BlockSize(uint32_t points) {
  return (kMaxColor + 1) / (points - 1);
}
