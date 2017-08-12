#pragma once

#include <cassert>
#include <experimental/string_view>

#include "color.h"
#include "image.h"

namespace std {
using string_view = experimental::string_view;
}

template <int32_t X, int32_t Y, int32_t C, int32_t D, int32_t A, int32_t P>
class PiRaw {
 public:
  PiRaw() = delete;
  PiRaw(const PiRaw&) = delete;
  PiRaw(PiRaw&&) = delete;

  static std::unique_ptr<Image<X / 2, Y / 2, C>> FromJpeg(const std::string_view& jpeg);
  static std::unique_ptr<Image<X / 2, Y / 2, C>> FromRaw(const std::string_view& raw);

 private:
  static constexpr int32_t kJpegHeaderBytes = 32768;
  static constexpr const char* kJpegHeaderMagic = "BRCM";
  static constexpr int32_t kPixelsPerChunk = 4;
  static constexpr int32_t kBitsPerByte = 8;

  static constexpr int32_t GetRawBytes();
  static constexpr int32_t GetRowBytes();
  static constexpr int32_t GetNumRows();
  static constexpr int32_t GetChunkBytes();

  static constexpr int32_t Align(int32_t val);

  typedef Array<int32_t, kPixelsPerChunk> Chunk;

  static constexpr Chunk GetChunk(const std::string_view& raw, const int32_t x_chunk, const int32_t y);
  static constexpr Color<C> CombineRaw(int32_t y0x0, int32_t y0x1, int32_t y1x0, int32_t y1x1);
};

typedef PiRaw<3280, 2464, 3, 10, 16, 2> PiRaw2;

template <int32_t X, int32_t Y, int32_t C, int32_t D, int32_t A, int32_t P>
typename std::unique_ptr<Image<X / 2, Y / 2, C>> PiRaw<X, Y, C, D, A, P>::FromJpeg(const std::string_view& jpeg) {
  static_assert(C == 3);

  size_t container_len = GetRawBytes() + kJpegHeaderBytes;
  assert(jpeg.substr(jpeg.size() - container_len, 4) == kJpegHeaderMagic);
  return FromRaw(jpeg.substr(jpeg.size() - GetRawBytes(), GetRawBytes()));
}

template <int32_t X, int32_t Y, int32_t C, int32_t D, int32_t A, int32_t P>
typename std::unique_ptr<Image<X / 2, Y / 2, C>> PiRaw<X, Y, C, D, A, P>::FromRaw(const std::string_view& raw) {
  static_assert(C == 3);
  static_assert(X % 2 == 0);
  static_assert(Y % 2 == 0);
  static_assert(kPixelsPerChunk == 4);

  assert(raw.size() == GetRawBytes());

  auto image = std::make_unique<Image<X / 2, Y / 2, C>>();

  for (int32_t y = 0, out_y = 0; y < Y; y += 2, ++out_y) {
    for (int32_t x_chunk = 0, out_x = 0; x_chunk < X / kPixelsPerChunk; ++x_chunk, out_x += kPixelsPerChunk / 2) {
      auto chunk1 = GetChunk(raw, x_chunk, y + 0);
      auto chunk2 = GetChunk(raw, x_chunk, y + 1);
      image->at(out_y).at(out_x + 0) = CombineRaw(chunk1.at(0), chunk1.at(1), chunk2.at(0), chunk2.at(1));
      image->at(out_y).at(out_x + 1) = CombineRaw(chunk1.at(2), chunk1.at(3), chunk2.at(2), chunk2.at(3));
    }
  }
  return image;
}

template <int32_t X, int32_t Y, int32_t C, int32_t D, int32_t A, int32_t P>
constexpr int32_t PiRaw<X, Y, C, D, A, P>::GetRawBytes() {
  return GetRowBytes() * GetNumRows();
}

template <int32_t X, int32_t Y, int32_t C, int32_t D, int32_t A, int32_t P>
constexpr int32_t PiRaw<X, Y, C, D, A, P>::GetRowBytes() {
  return Align(Align(X + P) * D / kBitsPerByte);
}

template <int32_t X, int32_t Y, int32_t C, int32_t D, int32_t A, int32_t P>
constexpr int32_t PiRaw<X, Y, C, D, A, P>::GetNumRows() {
  return Align(Y + P);
}

template <int32_t X, int32_t Y, int32_t C, int32_t D, int32_t A, int32_t P>
constexpr int32_t PiRaw<X, Y, C, D, A, P>::GetChunkBytes() {
  return D * kPixelsPerChunk / kBitsPerByte;
}

template <int32_t X, int32_t Y, int32_t C, int32_t D, int32_t A, int32_t P>
constexpr int32_t PiRaw<X, Y, C, D, A, P>::Align(int32_t val) {
  return (~(A - 1)) & ((val) + (A - 1));
}

template <int32_t X, int32_t Y, int32_t C, int32_t D, int32_t A, int32_t P>
constexpr typename PiRaw<X, Y, C, D, A, P>::Chunk PiRaw<X, Y, C, D, A, P>::GetChunk(const std::string_view& raw, const int32_t x_chunk, const int32_t y) {
  // Function is bit depth & layout specific
  static_assert(C == 3);
  static_assert(D == 10);

  size_t start = static_cast<size_t>(y * GetRowBytes() + x_chunk * GetChunkBytes());
  uint32_t high0 = static_cast<uint8_t>(raw.at(start + 0));
  uint32_t high1 = static_cast<uint8_t>(raw.at(start + 1));
  uint32_t high2 = static_cast<uint8_t>(raw.at(start + 2));
  uint32_t high3 = static_cast<uint8_t>(raw.at(start + 3));
  uint32_t packed_low = static_cast<uint8_t>(raw.at(start + 4));

  Chunk ret;
  ret.at(0) = static_cast<int32_t>(((high0 << 2) | ((packed_low >> 6) & 0b11)) << 6);
  ret.at(1) = static_cast<int32_t>(((high1 << 2) | ((packed_low >> 4) & 0b11)) << 6);
  ret.at(2) = static_cast<int32_t>(((high2 << 2) | ((packed_low >> 2) & 0b11)) << 6);
  ret.at(3) = static_cast<int32_t>(((high3 << 2) | ((packed_low >> 0) & 0b11)) << 6);
  return ret;
}

template <int32_t X, int32_t Y, int32_t C, int32_t D, int32_t A, int32_t P>
constexpr Color<C> PiRaw<X, Y, C, D, A, P>::CombineRaw(int32_t y0x0, int32_t y0x1, int32_t y1x0, int32_t y1x1) {
  // Function is bit layout specific
  static_assert(C == 3);

  Color<C> ret;
  ret.at(0) = static_cast<int32_t>(y1x1);
  ret.at(1) = static_cast<int32_t>((y0x1 + y1x0) / 2);
  ret.at(2) = static_cast<int32_t>(y0x0);
  return ret;
}
