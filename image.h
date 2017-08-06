#pragma once

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
