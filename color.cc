#include "color.h"

RgbColor::RgbColor(const Color<3>& src)
    : Color<3>(src) {}

RgbColor::operator HsvColor() const {
  constexpr int32_t kSection = kNumColors / 6;
  int32_t max = *std::max_element(this->begin(), this->end());
  int32_t min = *std::min_element(this->begin(), this->end());
  int32_t delta = max - min;

  if (delta == 0) {
    return {{{{{0x0000, 0x0000, max}}}}};
  }

  HsvColor ret = {{{{{0, 0, max}}}}};

  if (max == this->at(0)) {
    ret.at(0) = (kSection * (this->at(1) - this->at(2))) / delta + (0 * kSection);
  } else if (max == this->at(1)) {
    ret.at(0) = (kSection * (this->at(2) - this->at(0))) / delta + (2 * kSection);
  } else {
    assert(max == this->at(2));
    ret.at(0) = (kSection * (this->at(0) - this->at(1))) / delta + (4 * kSection);
  }

  if (ret.at(0) < 0) {
    ret.at(0) += kNumColors;
  }

  if (max > 0) {
    ret.at(1) = static_cast<int32_t>(static_cast<int64_t>(delta) * kMaxColor / max);
  }

  return ret;
}


HsvColor::HsvColor(const Color<3>& src)
    : Color<3>(src) {}
