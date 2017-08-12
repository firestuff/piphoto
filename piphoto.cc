#include <iostream>

#include "colorchecker.h"
#include "lut.h"
#include "piraw.h"
#include "util.h"

int main() {
  auto image = PiRaw2::FromJpeg(ReadFile("test.jpg"));
  WriteFile("start.png", HighlightClosest(*image)->ToPng());
  std::cout << "Initial error: " << ScoreImage(*image) << std::endl;

  auto lut = MinimalLut3d::Identity();
  uint32_t diff = 1;
  while (diff) {
    diff = OptimizeLut<4>(*image, &lut);
    std::cout << "diff=" << diff << " error=" << ScoreImage(*lut.MapImage(*image)) << std::endl;
    WriteFile("inter.png", HighlightClosest(*lut.MapImage(*image))->ToPng());
  }

  WriteFile("test.png", HighlightClosest(*lut.MapImage(*image))->ToPng());
}
