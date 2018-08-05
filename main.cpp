#include <vector>
#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

void line2(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
  for (float t = 0.; t < 1.; t += .1) {
    int x = x0 * (1. - t) + x1 * t;
    int y = y0 * (1. - t) + y1 * t;
    image.set(x, y, color);
  }
}

void line(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor& color) {
  bool is_range_y = false;

  // We want to do work on a range that is longer than the domain so each pixel has a value.
  if (std::abs(y1 - y0) > std::abs(x1 - x0)) {
    is_range_y = true;
    std::swap(x0, y0);
    std::swap(x1, y1);
  }

  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  int range = x1 - x0;

  // If points are same exit.
  if (range == 0) {
    image.set(x0, y0, color);
    return;
  }

  // x0 is range_start, x1 is range_end, y0 is domain_start, y1 is domain_end.
  float range_inverse = 1.0f / range;
  float domain = y1 - y0;
  float domain_range_inverse = domain * range_inverse;

  // If range is 1, we have (range + 1) or two pixels.
  for (int i = 0; i <= range; i++) {
    int domain_closest = y0 + i * domain_range_inverse;
    if (is_range_y) {
      image.set(domain_closest, x0 + i, color);
    } else {
      image.set(x0 + i, domain_closest, color);
    }
  }
}

int main() {
  TGAImage image(500, 500, TGAImage::RGB);
  std::vector<int> start = {250, 250};
  std::vector<int> end = {80, 41};
  std::vector<int> x_series = {end[0], end[1], -end[1], -end[0], -end[0], -end[1], end[1], end[0]};
  std::vector<int> y_series = {end[1], end[0], end[0], end[1], -end[1], -end[0], -end[0], -end[1]};

  // This is for profiling purposes.
  for (int j = 0; j < 1e6; j++) {
    for (size_t i = 0; i < x_series.size(); i++) {
      line(
          start[0],
          start[1],
          (start[0] + x_series[i]),
          (start[1] + y_series[i]),
          image,
          TGAColor(0xff - (i * 20), 0xff - (i * 20), 0xff - (i * 20), 0xff));
    }
  }

  image.flip_vertically();  // i want to have the origin at the left bottom corner of the image
  image.write_tga_file("output.tga");
  return 0;
}
