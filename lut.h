#pragma once

#include "array.h"
#include "color.h"
#include "coord.h"
#include "image.h"

class LutBase {
 public:
  LutBase() = default;
  LutBase(const LutBase&) = default;
  virtual ~LutBase();

  // TODO: Allow other color dimensions
  virtual Color<3> MapColor(const Color<3>& in) const = 0;

  template <int32_t X, int32_t Y, class C>
  std::unique_ptr<Image<X, Y, C>> MapImage(const Image<X, Y, C>& in) const;

 protected:
  static constexpr std::pair<int32_t, int32_t> FindChannelRoot(int32_t value, int32_t points);
  static constexpr int32_t BlockSize(int32_t points);
};

template <int32_t X, int32_t Y, class C>
std::unique_ptr<Image<X, Y, C>> LutBase::MapImage(const Image<X, Y, C>& in) const {
  auto out = std::make_unique<Image<X, Y, C>>();

  for (int32_t y = 0; y < Y; ++y) {
    for (int32_t x = 0; x < X; ++x) {
      Coord<2> coord = {{{{x, y}}}};
      out->SetPixel(coord, MapColor(in.GetPixel(coord)));
    }
  }

  return out;
}

constexpr int32_t LutBase::BlockSize(int32_t points) {
  return (kMaxColor + 1) / (points - 1);
}

constexpr std::pair<int32_t, int32_t> LutBase::FindChannelRoot(int32_t value, int32_t points) {
  // points - 1 is the last point index. Since we're going to find the region
  // around this point by adding to the root, we need to be at least 1 less
  // than that.
  int32_t index = std::min(points - 2, value / BlockSize(points));
  return std::make_pair(index, value - (index * BlockSize(points)));
}


template <int32_t X>
class Lut1d : public Array<Color<3>, X>, public LutBase {
 public:
  static Lut1d<X> Identity();

  Color<3> MapColor(const Color<3>& in) const override;
};

typedef Lut1d<2> MinimalLut1d;

template <int32_t X>
Lut1d<X> Lut1d<X>::Identity() {
  Lut1d<X> ret;

  Color<3> color;
  for (int32_t x = 0; x < X; ++x) {
    color.at(0) = color.at(1) = color.at(2) = std::min(kMaxColor, BlockSize(X) * x);
    ret.at(x) = color;
  }

  return ret;
}

template <int32_t X>
Color<3> Lut1d<X>::MapColor(const Color<3>& in) const {
  Color<3> ret;

  for (int32_t c = 0; c < 3; ++c) {
    const auto root_rem = FindChannelRoot(in.at(c), X);
    const auto& root = root_rem.first;
    const auto& rem = root_rem.second;
    ret.at(c) = Interpolate(
      this->at(root + 0).at(c),
      this->at(root + 1).at(c),
      rem, BlockSize(X));
  }

  return ret;
}


template <int32_t X, int32_t Y, int32_t Z>
class Lut3d : public Array<Array<Array<Color<3>, X>, Y>, Z>, public LutBase {
 public:
  static Lut3d<X, Y, Z> Identity();

  Color<3> MapColor(const Color<3>& in) const override;

 private:
  // Return value is (root_indices, remainders)
  constexpr static std::pair<Coord<3>, Coord<3>> FindRoot(const Color<3>& in);
};

typedef Lut3d<2, 2, 2> MinimalLut3d;

template <int32_t X, int32_t Y, int32_t Z>
Lut3d<X, Y, Z> Lut3d<X, Y, Z>::Identity() {
  Lut3d<X, Y, Z> ret;

  Color<3> color;
  for (int32_t x = 0; x < X; ++x) {
    auto& rect = ret.at(x);
    color.at(0) = std::min(kMaxColor, BlockSize(X) * x);

    for (int32_t y = 0; y < Y; ++y) {
      auto& row = rect.at(y);
      color.at(1) = std::min(kMaxColor, BlockSize(Y) * y);

      for (int32_t z = 0; z < Z; ++z) {
        color.at(2) = std::min(kMaxColor, BlockSize(Z) * z);
        row.at(z) = color;
      }
    }
  }

  return ret;
}

template <int32_t X, int32_t Y, int32_t Z>
Color<3> Lut3d<X, Y, Z>::MapColor(const Color<3>& in) const {
  const auto root_rem = FindRoot(in);
  const auto& root = root_rem.first;
  const auto& rem = root_rem.second;

  // https://en.wikipedia.org/wiki/Trilinear_interpolation
  auto inter00 =
    this->at(root.at(0) + 0).at(root.at(1) + 0).at(root.at(2) + 0).Interpolate(
    this->at(root.at(0) + 1).at(root.at(1) + 0).at(root.at(2) + 0),
    rem.at(0), BlockSize(X));

  auto inter01 =
    this->at(root.at(0) + 0).at(root.at(1) + 0).at(root.at(2) + 1).Interpolate(
    this->at(root.at(0) + 1).at(root.at(1) + 0).at(root.at(2) + 1),
    rem.at(0), BlockSize(X));

  auto inter10 =
    this->at(root.at(0) + 0).at(root.at(1) + 1).at(root.at(2) + 0).Interpolate(
    this->at(root.at(0) + 1).at(root.at(1) + 1).at(root.at(2) + 0),
    rem.at(0), BlockSize(X));

  auto inter11 =
    this->at(root.at(0) + 0).at(root.at(1) + 1).at(root.at(2) + 1).Interpolate(
    this->at(root.at(0) + 1).at(root.at(1) + 1).at(root.at(2) + 1),
    rem.at(0), BlockSize(X));

  auto inter0 = inter00.Interpolate(inter10, rem.at(1), BlockSize(Y));
  auto inter1 = inter01.Interpolate(inter11, rem.at(1), BlockSize(Y));

  return inter0.Interpolate(inter1, rem.at(2), BlockSize(Z)).Crop();
}

template <int32_t X, int32_t Y, int32_t Z>
constexpr std::pair<Coord<3>, Coord<3>> Lut3d<X, Y, Z>::FindRoot(const Color<3>& in) {
  auto root_x = FindChannelRoot(in.at(0), X);
  auto root_y = FindChannelRoot(in.at(1), Y);
  auto root_z = FindChannelRoot(in.at(2), Z);
  return {
    {{{{root_x.first, root_y.first, root_z.first}}}},
    {{{{root_x.second, root_y.second, root_z.second}}}},
  };
}
