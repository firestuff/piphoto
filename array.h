#pragma once

#include <array>
#include <cassert>

template <class T, int32_t N>
struct Array : std::array<T, N> {
  using reference = typename std::array<T, N>::reference;
  using const_reference = typename std::array<T, N>::const_reference;
  using size_type = typename std::array<T, N>::size_type;

  constexpr int32_t ssize() const;

  constexpr reference at(int32_t pos);
  constexpr const_reference at(int32_t pos) const;
};

template <class T, int32_t N>
constexpr int32_t Array<T, N>::ssize() const {
  auto size = this->size();
  assert(size < INT32_MAX);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
  return size;
#pragma clang diagnostic pop
}

template <class T, int32_t N>
constexpr typename Array<T, N>::reference Array<T, N>::at(int32_t pos) {
  assert(pos >= 0);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
  return std::array<T, N>::at(static_cast<size_type>(pos));
#pragma clang diagnostic pop
}

template <class T, int32_t N>
constexpr typename Array<T, N>::const_reference Array<T, N>::at(int32_t pos) const {
  assert(pos >= 0);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
  return std::array<T, N>::at(static_cast<size_type>(pos));
#pragma clang diagnostic pop
}
