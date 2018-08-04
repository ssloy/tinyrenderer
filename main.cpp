#include <vector>
#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
  int x_range = x1 - x0;
  int y_range = y1 - y0;

  int range_start;
  int range_end;

  bool x_range_larger = false;
  if (abs(x_range) > abs(y_range)) {
    x_range_larger = true;
    range_start = x0;
    range_end = x1;
  } else {
  }

  for (float t = 0.; t < 1.; t += .1) {
    int x = x0 * (1. - t) + x1 * t;
    int y = y0 * (1. - t) + y1 * t;
    image.set(x, y, color);
  }
}

int main() {
  TGAImage image(500, 500, TGAImage::RGB);
  std::vector<int> start = {250, 250};
  std::vector<int> end = {80, 40};
  std::vector<int> x_series = {end[0], end[1], -end[1], -end[0], -end[0], -end[1], end[1], end[0]};
  std::vector<int> y_series = {end[1], end[0], end[0], end[1], -end[1], -end[0], -end[0], -end[1]};

  for (size_t i = 0; i < x_series.size(); i++) {
    line(
        start[0],
        start[1],
        (start[0] + x_series[i]),
        (start[1] + y_series[i]),
        image,
        TGAColor(0xff - (i * 20), 0xff - (i * 20), 0xff - (i * 20), 0xff));
  }

  image.flip_vertically();  // i want to have the origin at the left bottom corner of the image
  image.write_tga_file("output.tga");
  return 0;
}
