#include <iostream>

#include "colorchecker.h"
#include "lut.h"
#include "piraw.h"
#include "util.h"

int main() {
  auto image = PiRaw2::FromJpeg(ReadFile("test.jpg"));
  auto lut = MinimalLut3d::Identity();
  auto image2 = lut->MapImage(*image);
  HighlightClosest(image2.get());
  WriteFile("test.png", image2->ToPng());
}
