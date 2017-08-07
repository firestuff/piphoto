#pragma once

#include <arpa/inet.h>
#include <png.h>

#include <cassert>

#include "color.h"
#include "coord.h"

template <uint32_t X, uint32_t Y>
class Image : public std::array<std::array<Color, X>, Y> {
 public:
  constexpr const Color& GetPixel(const Coord& coord) const;

  void SetPixel(const Coord& coord, const Color& color);
  void DrawXLine(const Coord& start, const Color& color, uint32_t length);
  void DrawYLine(const Coord& start, const Color& color, uint32_t length);
  void DrawRectangle(const Coord& start, const Color& color, uint32_t x_length, uint32_t y_length);
  void DrawSquare(const Coord& start, const Color& color, uint32_t length);

  std::string ToPng();
};

template <uint32_t X, uint32_t Y>
constexpr const Color& Image<X, Y>::GetPixel(const Coord& coord) const {
  return this->at(coord.y).at(coord.x);
}

template <uint32_t X, uint32_t Y>
void Image<X, Y>::SetPixel(const Coord& coord, const Color& color) {
  this->at(coord.y).at(coord.x) = color;
}

template <uint32_t X, uint32_t Y>
void Image<X, Y>::DrawXLine(const Coord& coord, const Color& color, uint32_t length) {
  auto& row = this->at(coord.y);

  for (uint32_t x = coord.x; x < std::min(X, coord.x + length); ++x) {
    row.at(x) = color;
  }
}

template <uint32_t X, uint32_t Y>
void Image<X, Y>::DrawYLine(const Coord& coord, const Color& color, uint32_t length) {
  for (uint32_t y = coord.y; y <= std::min(Y, coord.y + length); ++y) {
    SetPixel({coord.x, y}, color);
  }
}

template <uint32_t X, uint32_t Y>
void Image<X, Y>::DrawRectangle(const Coord& start, const Color& color, uint32_t x_length, uint32_t y_length) {
  DrawXLine(start, color, x_length);
  DrawXLine({start.x, start.y + y_length}, color, x_length);
  DrawYLine(start, color, y_length);
  DrawYLine({start.x + x_length, start.y}, color, y_length);
}

template <uint32_t X, uint32_t Y>
void Image<X, Y>::DrawSquare(const Coord& start, const Color& color, uint32_t length) {
  DrawRectangle(start, color, length, length);
}

static void WriteCallback(png_structp png_ptr, png_bytep data, png_size_t length) {
  auto dest = static_cast<std::string*>(png_get_io_ptr(png_ptr));
  dest->append(reinterpret_cast<char*>(data), length);
}

template <uint32_t X, uint32_t Y>
std::string Image<X, Y>::ToPng() {
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
