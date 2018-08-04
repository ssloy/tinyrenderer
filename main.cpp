#include <vector>
#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
  for (float t = 0.; t < 1.; t += .1) {
    int x = x0 * (1. - t) + x1 * t;
    int y = y0 * (1. - t) + y1 * t;
    image.set(x, y, color);
  }
}

int main() {
  TGAImage image(100, 100, TGAImage::RGB);
  std::vector<int> start = {50, 50};
  std::vector<int> end = {40, 10};
  std::vector<int> x_series = {end[0], end[0], -end[0], -end[0], end[1], end[1], -end[1], -end[1]};
  std::vector<int> y_series = {end[1], -end[1], end[1], -end[1], end[0], -end[0], end[0], -end[0]};

  for (size_t i = 0; i < x_series.size(); i++) {
    line(
        start[0],
        start[1],
        start[0] + x_series[i],
        start[1] + y_series[i],
        image,
        TGAColor(0xff, 0xff, 0xff, 0xff));
  }

  image.flip_vertically();  // i want to have the origin at the left bottom corner of the image
  image.write_tga_file("output.tga");
  return 0;
}
