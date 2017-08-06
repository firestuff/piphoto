#pragma once

#include <arpa/inet.h>
#include <png.h>

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
  PiRaw(std::unique_ptr<Image<X, Y>>);
  static PiRaw FromJpeg(const std::string_view& jpeg);
  static PiRaw FromRaw(const std::string_view& raw);

  std::string ToPng();

  Image<X, Y>* GetImage();
  const Image<X, Y>& GetImage() const;

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

  static Chunk GetChunk(const std::string_view& raw, const uint32_t x_chunk, const uint32_t y);
  static Color CombineRaw(uint32_t y0x0, uint32_t y0x1, uint32_t y1x0, uint32_t y1x1);

  std::unique_ptr<Image<X, Y>> image_;
};

typedef PiRaw<3280, 2464, 10, 16, 2> PiRaw2;

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
PiRaw<X, Y, D, A, P>::PiRaw(std::unique_ptr<Image<X, Y>> image)
    : image_(std::move(image)) {}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
PiRaw<X, Y, D, A, P> PiRaw<X, Y, D, A, P>::FromJpeg(const std::string_view& jpeg) {
  auto container_len = GetRawBytes() + kJpegHeaderBytes;
  assert(jpeg.substr(jpeg.size() - container_len, 4) == kJpegHeaderMagic);
  return FromRaw(jpeg.substr(jpeg.size() - GetRawBytes(), GetRawBytes()));
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
PiRaw<X, Y, D, A, P> PiRaw<X, Y, D, A, P>::FromRaw(const std::string_view& raw) {
  static_assert(X % 2 == 0);
  static_assert(Y % 2 == 0);
  static_assert(kPixelsPerChunk == 4);

  assert(raw.size() == GetRawBytes());

  auto image = std::make_unique<Image<X, Y>>();

  for (uint32_t y = 0, out_y = 0; y < Y; y += 2, ++out_y) {
    for (uint32_t x_chunk = 0, out_x = 0; x_chunk < X / kPixelsPerChunk; ++x_chunk, out_x += kPixelsPerChunk / 2) {
      auto chunk1 = GetChunk(raw, x_chunk, y + 0);
      auto chunk2 = GetChunk(raw, x_chunk, y + 1);
      image->at(out_y).at(out_x + 0) = CombineRaw(chunk1.at(0), chunk1.at(1), chunk2.at(0), chunk2.at(1));
      image->at(out_y).at(out_x + 1) = CombineRaw(chunk1.at(2), chunk1.at(3), chunk2.at(2), chunk2.at(3));
    }
  }
  return PiRaw<X, Y, D, A, P>(std::move(image));
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
typename PiRaw<X, Y, D, A, P>::Chunk PiRaw<X, Y, D, A, P>::GetChunk(const std::string_view& raw, const uint32_t x_chunk, const uint32_t y) {
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
Color PiRaw<X, Y, D, A, P>::CombineRaw(uint32_t y0x0, uint32_t y0x1, uint32_t y1x0, uint32_t y1x1) {
  // Function is bit layout specific
  Color ret;
  ret.r = y1x1;
  ret.g = (y0x1 + y1x0) / 2;
  ret.b = y0x0;
  return ret;
}

static void WriteCallback(png_structp png_ptr, png_bytep data, png_size_t length) {
  auto dest = static_cast<std::string*>(png_get_io_ptr(png_ptr));
  dest->append(reinterpret_cast<char*>(data), length);
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
std::string PiRaw<X, Y, D, A, P>::ToPng() {
  std::string ret;

  auto png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  assert(png_ptr);
  auto info_ptr = png_create_info_struct(png_ptr);
  assert(info_ptr);

  png_set_write_fn(png_ptr, &ret, &WriteCallback, nullptr);
  png_set_IHDR(png_ptr, info_ptr, X / 2, Y / 2,
    16, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  png_write_info(png_ptr, info_ptr);
  for (auto& row : *image_) {
    std::array<uint16_t, X * 3> out_row;
    for (uint32_t x = 0; x < X; ++x) {
      out_row[x * 3 + 0] = htons(static_cast<uint16_t>(row[x].r));
      out_row[x * 3 + 1] = htons(static_cast<uint16_t>(row[x].g));
      out_row[x * 3 + 2] = htons(static_cast<uint16_t>(row[x].b));
    }
    png_write_row(png_ptr, reinterpret_cast<unsigned char*>(out_row.data()));
  }
  png_write_end(png_ptr, nullptr);

  png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
  png_destroy_write_struct(&png_ptr, &info_ptr);

  return ret;
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
Image<X, Y>* PiRaw<X, Y, D, A, P>::GetImage() {
  return image_.get();
}

template <uint32_t X, uint32_t Y, uint32_t D, uint32_t A, uint32_t P>
const Image<X, Y>& PiRaw<X, Y, D, A, P>::GetImage() const {
  return *image_;
}
