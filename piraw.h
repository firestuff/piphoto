#pragma once

#include <cassert>
#include <experimental/string_view>

#include "color.h"
#include "image.h"

namespace std {
using string_view = experimental::string_view;
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
class PiRaw {
 public:
  PiRaw() = delete;
  PiRaw(const PiRaw&) = delete;
  PiRaw(PiRaw&&) = delete;

  static std::unique_ptr<Image<X / 2, Y / 2>> FromJpeg(const std::string_view& jpeg);
  static std::unique_ptr<Image<X / 2, Y / 2>> FromRaw(const std::string_view& raw);

 private:
  static constexpr uint32_t kJpegHeaderBytes = 32768;
  static constexpr const char* kJpegHeaderMagic = "BRCM";
  static constexpr uint32_t kPixelsPerChunk = 4;
  static constexpr uint32_t kBitsPerByte = 8;

  static constexpr uint32_t GetRawBytes();
  static constexpr uint32_t GetRowBytes();
  static constexpr uint32_t GetNumRows();
  static constexpr uint32_t GetChunkBytes();

  static constexpr uint32_t Align(uint32_t val);

  typedef std::array<uint32_t, kPixelsPerChunk> Chunk;

  static constexpr Chunk GetChunk(const std::string_view& raw, const uint32_t x_chunk, const uint32_t y);
  static constexpr Color CombineRaw(uint32_t y0x0, uint32_t y0x1, uint32_t y1x0, uint32_t y1x1);
};

typedef PiRaw<3280, 2464, 10, 16, 2> PiRaw2;

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
typename std::unique_ptr<Image<X / 2, Y / 2>> PiRaw<X, Y, D, A, P>::FromJpeg(const std::string_view& jpeg) {
  auto container_len = GetRawBytes() + kJpegHeaderBytes;
  assert(jpeg.substr(jpeg.size() - container_len, 4) == kJpegHeaderMagic);
  return FromRaw(jpeg.substr(jpeg.size() - GetRawBytes(), GetRawBytes()));
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
typename std::unique_ptr<Image<X / 2, Y / 2>> PiRaw<X, Y, D, A, P>::FromRaw(const std::string_view& raw) {
  static_assert(X % 2 == 0);
  static_assert(Y % 2 == 0);
  static_assert(kPixelsPerChunk == 4);

  assert(raw.size() == GetRawBytes());

  auto image = std::make_unique<Image<X / 2, Y / 2>>();

  for (uint32_t y = 0, out_y = 0; y < Y; y += 2, ++out_y) {
    for (uint32_t x_chunk = 0, out_x = 0; x_chunk < X / kPixelsPerChunk; ++x_chunk, out_x += kPixelsPerChunk / 2) {
      auto chunk1 = GetChunk(raw, x_chunk, y + 0);
      auto chunk2 = GetChunk(raw, x_chunk, y + 1);
      image->at(out_y).at(out_x + 0) = CombineRaw(chunk1.at(0), chunk1.at(1), chunk2.at(0), chunk2.at(1));
      image->at(out_y).at(out_x + 1) = CombineRaw(chunk1.at(2), chunk1.at(3), chunk2.at(2), chunk2.at(3));
    }
  }
  return image;
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
constexpr uint32_t PiRaw<X, Y, D, A, P>::GetRawBytes() {
  return GetRowBytes() * GetNumRows();
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
constexpr uint32_t PiRaw<X, Y, D, A, P>::GetRowBytes() {
  return Align(Align(X + P) * D / kBitsPerByte);
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
constexpr uint32_t PiRaw<X, Y, D, A, P>::GetNumRows() {
  return Align(Y + P);
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
constexpr uint32_t PiRaw<X, Y, D, A, P>::GetChunkBytes() {
  return D * kPixelsPerChunk / kBitsPerByte;
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
constexpr uint32_t PiRaw<X, Y, D, A, P>::Align(uint32_t val) {
  return (~(A - 1)) & ((val) + (A - 1));
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
constexpr typename PiRaw<X, Y, D, A, P>::Chunk PiRaw<X, Y, D, A, P>::GetChunk(const std::string_view& raw, const uint32_t x_chunk, const uint32_t y) {
  // Function is bit depth & layout specific
  static_assert(D == 10);

  auto start = y * GetRowBytes() + x_chunk * GetChunkBytes();
  auto high0 = static_cast<uint32_t>(raw.at(start + 0));
  auto high1 = static_cast<uint32_t>(raw.at(start + 1));
  auto high2 = static_cast<uint32_t>(raw.at(start + 2));
  auto high3 = static_cast<uint32_t>(raw.at(start + 3));
  auto packed_low = static_cast<uint32_t>(raw.at(start + 4));

  Chunk ret;
  ret.at(0) = ((high0 << 2) | ((packed_low >> 6) & 0b11)) << 6;
  ret.at(1) = ((high1 << 2) | ((packed_low >> 4) & 0b11)) << 6;
  ret.at(2) = ((high2 << 2) | ((packed_low >> 2) & 0b11)) << 6;
  ret.at(3) = ((high3 << 2) | ((packed_low >> 0) & 0b11)) << 6;
  return ret;
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
constexpr Color PiRaw<X, Y, D, A, P>::CombineRaw(uint32_t y0x0, uint32_t y0x1, uint32_t y1x0, uint32_t y1x1) {
  // Function is bit layout specific
  Color ret;
  ret.r = y1x1;
  ret.g = (y0x1 + y1x0) / 2;
  ret.b = y0x0;
  return ret;
}
