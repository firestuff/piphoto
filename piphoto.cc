#include <iostream>

#include "color.h"
#include "colorchecker.h"
#include "colors.h"
#include "coord.h"
#include "image.h"
#include "lut.h"
#include "piraw.h"
#include "util.h"

int main() {
  auto image = PiRaw2::FromJpeg(ReadFile("test.jpg"));

  auto lut = ColorCheckerLut3d::Identity();
  auto image2 = lut->MapImage(*image);

  auto closest = ColorCheckerClosest(*image2);
  for (uint32_t cc = 0; cc < kColorCheckerSrgb.size(); ++cc) {
    const auto& coord = closest.at(cc);
    const auto& color = kColorCheckerSrgb.at(cc);
    std::cout << cc << ": " << coord << " difference=" << color.Difference(image2->GetPixel(coord)) << std::endl;
    image2->DrawSquare({std::max(5U, coord.x) - 5, std::max(5U, coord.y) - 5}, kBlack, 10);
    image2->DrawSquare({std::max(6U, coord.x) - 6, std::max(6U, coord.y) - 6}, color, 12);
    image2->DrawSquare({std::max(7U, coord.x) - 7, std::max(7U, coord.y) - 7}, color, 14);
    image2->DrawSquare({std::max(8U, coord.x) - 8, std::max(8U, coord.y) - 8}, color, 16);
    image2->DrawSquare({std::max(9U, coord.x) - 9, std::max(9U, coord.y) - 9}, kWhite, 18);
  }
  WriteFile("test.png", image2->ToPng());
}
