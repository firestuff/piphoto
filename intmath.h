#pragma once

template <typename T>
constexpr T AbsDiff(T a, T b) {
  return (a > b) ? (a - b) : (b - a);
}
