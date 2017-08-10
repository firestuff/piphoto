#pragma once

#include <arpa/inet.h>
#include <png.h>

#include <cassert>

#include "color.h"
#include "coord.h"

template <uint32_t X, uint32_t Y, uint32_t C>
class Image : public std::array<std::array<Color<C>, X>, Y> {
 public:
  constexpr const Color<C>& GetPixel(const Coord<2>& coord) const;

  void SetPixel(const Coord<2>& coord, const Color<C>& color);
  void DrawXLine(const Coord<2>& start, const Color<C>& color, uint32_t length);
  void DrawYLine(const Coord<2>& start, const Color<C>& color, uint32_t length);
  void DrawRectangle(const Coord<2>& start, const Color<C>& color, uint32_t x_length, uint32_t y_length);
  void DrawSquare(const Coord<2>& start, const Color<C>& color, uint32_t length);

  std::string ToPng();
};

template <uint32_t X, uint32_t Y, uint32_t C>
constexpr const Color<C>& Image<X, Y, C>::GetPixel(const Coord<2>& coord) const {
  return this->at(coord.at(1)).at(coord.at(0));
}

template <uint32_t X, uint32_t Y, uint32_t C>
void Image<X, Y, C>::SetPixel(const Coord<2>& coord, const Color<C>& color) {
  this->at(coord.at(1)).at(coord.at(0)) = color;
}

template <uint32_t X, uint32_t Y, uint32_t C>
void Image<X, Y, C>::DrawXLine(const Coord<2>& coord, const Color<C>& color, uint32_t length) {
  auto& row = this->at(coord.at(1));

  for (uint32_t x = coord.at(0); x < std::min(X, coord.at(0) + length); ++x) {
    row.at(x) = color;
  }
}

template <uint32_t X, uint32_t Y, uint32_t C>
void Image<X, Y, C>::DrawYLine(const Coord<2>& coord, const Color<C>& color, uint32_t length) {
  for (uint32_t y = coord.at(1); y <= std::min(Y, coord.at(1) + length); ++y) {
    SetPixel({{{coord.at(0), y}}}, color);
  }
}

template <uint32_t X, uint32_t Y, uint32_t C>
void Image<X, Y, C>::DrawRectangle(const Coord<2>& start, const Color<C>& color, uint32_t x_length, uint32_t y_length) {
  DrawXLine(start, color, x_length);
  DrawXLine({{{start.at(0), start.at(1) + y_length}}}, color, x_length);
  DrawYLine(start, color, y_length);
  DrawYLine({{{start.at(0) + x_length, start.at(1)}}}, color, y_length);
}

template <uint32_t X, uint32_t Y, uint32_t C>
void Image<X, Y, C>::DrawSquare(const Coord<2>& start, const Color<C>& color, uint32_t length) {
  DrawRectangle(start, color, length, length);
}

static inline void WriteCallback(png_structp png_ptr, png_bytep data, png_size_t length) {
  auto dest = static_cast<std::string*>(png_get_io_ptr(png_ptr));
  dest->append(reinterpret_cast<char*>(data), length);
}

template <uint32_t X, uint32_t Y, uint32_t C>
std::string Image<X, Y, C>::ToPng() {
  static_assert(C == 3);  // PNG only supports RGB

  std::string ret;

  auto png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  assert(png_ptr);
  auto info_ptr = png_create_info_struct(png_ptr);
  assert(info_ptr);

  png_set_write_fn(png_ptr, &ret, &WriteCallback, nullptr);
  png_set_IHDR(png_ptr, info_ptr, X, Y,
    16, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  png_write_info(png_ptr, info_ptr);
  for (auto& row : *this) {
    std::array<uint16_t, X * 3> out_row;
    for (uint32_t x = 0; x < X; ++x) {
      out_row[x * 3 + 0] = htons(static_cast<uint16_t>(row[x].at(0)));
      out_row[x * 3 + 1] = htons(static_cast<uint16_t>(row[x].at(1)));
      out_row[x * 3 + 2] = htons(static_cast<uint16_t>(row[x].at(2)));
    }
    png_write_row(png_ptr, reinterpret_cast<unsigned char*>(out_row.data()));
  }
  png_write_end(png_ptr, nullptr);

  png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
  png_destroy_write_struct(&png_ptr, &info_ptr);

  return ret;
}
