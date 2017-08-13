#pragma once

#include <arpa/inet.h>
#include <png.h>

#include <cassert>

#include "array.h"
#include "color.h"
#include "coord.h"

template <int32_t X, int32_t Y, class C>
class Image : public Array<Array<C, X>, Y> {
 public:
  constexpr const C& GetPixel(const Coord<2>& coord) const;

  void SetPixel(const Coord<2>& coord, const C& color);
  void DrawXLine(const Coord<2>& start, const C& color, int32_t length);
  void DrawYLine(const Coord<2>& start, const C& color, int32_t length);
  void DrawRectangle(const Coord<2>& start, const C& color, int32_t x_length, int32_t y_length);
  void DrawSquare(const Coord<2>& start, const C& color, int32_t length);

  std::string ToPng();
};

template <int32_t X, int32_t Y, class C>
constexpr const C& Image<X, Y, C>::GetPixel(const Coord<2>& coord) const {
  return this->at(coord.at(1)).at(coord.at(0));
}

template <int32_t X, int32_t Y, class C>
void Image<X, Y, C>::SetPixel(const Coord<2>& coord, const C& color) {
  if (coord.at(0) >= X || coord.at(1) >= Y) {
    return;
  }
  this->at(coord.at(1)).at(coord.at(0)) = color;
}

template <int32_t X, int32_t Y, class C>
void Image<X, Y, C>::DrawXLine(const Coord<2>& coord, const C& color, int32_t length) {
  for (int32_t x = coord.at(0); x <= coord.at(0) + length; ++x) {
    SetPixel({{{{x, coord.at(1)}}}}, color);
  }
}

template <int32_t X, int32_t Y, class C>
void Image<X, Y, C>::DrawYLine(const Coord<2>& coord, const C& color, int32_t length) {
  for (int32_t y = coord.at(1); y <= coord.at(1) + length; ++y) {
    SetPixel({{{{coord.at(0), y}}}}, color);
  }
}

template <int32_t X, int32_t Y, class C>
void Image<X, Y, C>::DrawRectangle(const Coord<2>& start, const C& color, int32_t x_length, int32_t y_length) {
  DrawXLine(start, color, x_length);
  DrawXLine({{{{start.at(0), start.at(1) + y_length}}}}, color, x_length);
  DrawYLine(start, color, y_length);
  DrawYLine({{{{start.at(0) + x_length, start.at(1)}}}}, color, y_length);
}

template <int32_t X, int32_t Y, class C>
void Image<X, Y, C>::DrawSquare(const Coord<2>& start, const C& color, int32_t length) {
  DrawRectangle(start, color, length, length);
}

static inline void WriteCallback(png_structp png_ptr, png_bytep data, png_size_t length) {
  auto dest = static_cast<std::string*>(png_get_io_ptr(png_ptr));
  dest->append(reinterpret_cast<char*>(data), length);
}

template <int32_t X, int32_t Y, class C>
std::string Image<X, Y, C>::ToPng() {
  // TODO: specialize this to RgbColor

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
    Array<uint16_t, X * 3> out_row;
    for (int32_t x = 0; x < X; ++x) {
      out_row.at(x * 3 + 0) = htons(static_cast<uint16_t>(row.at(x).at(0)));
      out_row.at(x * 3 + 1) = htons(static_cast<uint16_t>(row.at(x).at(1)));
      out_row.at(x * 3 + 2) = htons(static_cast<uint16_t>(row.at(x).at(2)));
    }
    png_write_row(png_ptr, reinterpret_cast<unsigned char*>(out_row.data()));
  }
  png_write_end(png_ptr, nullptr);

  png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
  png_destroy_write_struct(&png_ptr, &info_ptr);

  return ret;
}
