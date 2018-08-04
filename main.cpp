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

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
  int range_start;
  int range_end;
  int domain_start;
  int domain_end;
  bool x_y_inverted = false;

  // We want to do work on a range that is longer than the domain so each pixel has a value.
  if (abs(x1 - x0) > abs(y1 - y0)) {
    if (x0 < x1) {
        range_start = x0;
        range_end = x1;
        domain_start = y0;
        domain_end = y1;
    } else {
        range_start = x1;
        range_end = x0;
        domain_start = y1;
        domain_end = y0;
    }
  } else {
    x_y_inverted = true;
    if (y0 < y1) {
        range_start = y0;
        range_end = y1;
        domain_start = x0;
        domain_end = x1;
    } else {
        range_start = y1;
        range_end = y0;
        domain_start = x1;
        domain_end = x0;
    }
  }

  // TODO: check not inf.
  float range_inverse = (1.0f / static_cast<float>(range_end - range_start));
  float domain = static_cast<float>(domain_end - domain_start);
  float domain_range_inverse = domain * range_inverse;

  for (int range_current = range_start; range_current < range_end; range_current++) {
    int domain_delta = static_cast<int>(static_cast<float>(range_current - range_start) * domain_range_inverse);
    int domain_closest = domain_start + domain_delta;

    if (!x_y_inverted) {
        image.set(range_current, domain_closest, color);
    } else {
        image.set(domain_closest, range_current, color);
    }
  }

}

int main() {
  TGAImage image(500, 500, TGAImage::RGB);
  std::vector<int> start = {250, 250};
  std::vector<int> end = {80, 41};
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
