#include <iostream>

#include "colorchecker.h"
#include "lut.h"
#include "piraw.h"
#include "util.h"

int main() {
  auto image = PiRaw2::FromJpeg(ReadFile("test.jpg"));
  WriteFile("start.png", HighlightClosest(*image)->ToPng());

  auto lut = MinimalLut1d::Identity();
  std::cout << "Initial error: " << ScoreLut(*image, lut) << std::endl;

  int32_t diff = 1;
  while (diff) {
    diff = OptimizeLut(*image, &lut);
    std::cout << "diff=" << diff << " error=" << ScoreLut(*image, lut) << std::endl;
    WriteFile("inter.png", HighlightClosest(*lut.MapImage(*image))->ToPng());
  }

  WriteFile("test.png", HighlightClosest(*lut.MapImage(*image))->ToPng());
}
