#pragma once

template <typename T>
constexpr T AbsDiff(T a, T b) {
  return (a > b) ? (a - b) : (b - a);
}

template <typename T>
constexpr T Interpolate(T val0, T val1, T mul, T div) {
  if (val1 > val0) {
    return val0 + ((mul * (val1 - val0)) / div);
  } else {
    return val0 - ((mul * (val0 - val1)) / div);
  }
}
