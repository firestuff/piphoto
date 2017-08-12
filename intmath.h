#pragma once

template <typename T>
constexpr T AbsDiff(T a, T b) {
  return std::abs(b - a);
}

template <typename T>
constexpr T Interpolate(T val0, T val1, T mul, T div) {
  return val0 + static_cast<int32_t>((static_cast<int64_t>(mul) * (val1 - val0)) / div);
}
