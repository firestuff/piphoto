#include <iostream>

#include "colorchecker.h"
#include "lut.h"
#include "piraw.h"
#include "util.h"

int main() {
  auto image = PiRaw2::FromJpeg(ReadFile("test.jpg"));
  auto lut = MinimalLut3d::Identity();
  auto image2 = lut->MapImage(*image);
  std::cout << "Score: " << ScoreImage(*image2) << std::endl;
  WriteFile("test.png", HighlightClosest(*image2)->ToPng());
}
