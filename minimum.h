#pragma once

template <typename I, typename O>
struct Range {
  I start;
  I end;
  I testpoint;
  O testpoint_value;
};

// Find the minimum value of a callback within a range, using a given
// parallelism.
//
// Deterministic for a given parallelism, but not guaranteed to be correct.
// Since it does a non-exhaustive search, can be fooled by distributions with
// multiple peaks, especially those with the minimum in a narrow valley and
// other wider valleys.
template <typename I, typename O, uint32_t P>
I FindPossibleMinimum(I min, I max, std::function<O(I)> callback) {
  if (min == max) {
    return min;
  }

  std::array<Range<I, O>, P> ranges;

  const I step = ((max - min) / P) + 1;
  const I offset = step / 2;
  for (uint32_t i = 0; i < P; ++i) {
    auto& range = ranges[i];
    range.start = std::min(max, min + i * step);
    range.end = std::min(max, range.start + (step - 1));
    range.testpoint = range.start + offset;
  }

  // TODO: threads
  for (auto& range : ranges) {
    range.testpoint_value = callback(range.testpoint);
  }

  const auto& min_range = *std::min_element(ranges.begin(), ranges.end(), [](const Range<I, O>& a, const Range<I, O>& b) {
    return a.testpoint_value < b.testpoint_value;
  });

  if (step == 1) {
    return min_range.testpoint;
  } else {
    return FindPossibleMinimum<I, O, P>(min_range.start, min_range.end, callback);
  }
}
