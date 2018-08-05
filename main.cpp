#include <string>
#include <vector>
#include "model.h"
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
  // Keep in mind y1 - y0 < x1 - x0.
  // Therefore, (y1 - y0) / (x1 - x0) < 1.
  float domain_range_inverse = (y1 - y0) * (1.0f / range);

  // If range is 1, we have (range + 1) or two pixels.
  float domain_closest = y0;
  for (int i = x0; i <= x1; i++) {
    if (is_range_y) {
      image.set(domain_closest, i, color);
    } else {
      image.set(i, domain_closest, color);
    }
    domain_closest += domain_range_inverse;
  }
}

void renderLines() {
  const int width = 500;
  const int height = 500;

  TGAImage image(width, height, TGAImage::RGB);
  std::vector<int> start = {250, 250};
  std::vector<int> end = {80, 41};
  std::vector<int> x_series = {end[0], end[1], -end[1], -end[0], -end[0], -end[1], end[1], end[0]};
  std::vector<int> y_series = {end[1], end[0], end[0], end[1], -end[1], -end[0], -end[0], -end[1]};

  // This is for profiling purposes.
  for (int j = 0; j < 1e5; j++) {
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
}

void renderModel() {
  std::string kModelFileName = "obj/african_head/african_head.obj";
  std::unique_ptr<Model> model = std::make_unique<Model>(kModelFileName.c_str());
  const int width = 800;
  const int height = 600;

  TGAImage image(width, height, TGAImage::RGB);
  for (int i = 0; i < model->nfaces(); i++) {
    std::vector<int> face = model->face(i);
    for (int j = 0; j < 3; j++) {
      Vec3f v0 = model->vert(face[j]);
      Vec3f v1 = model->vert(face[(j + 1) % 3]);
      int x0 = (v0.x + 1.) * width / 2.;
      int y0 = (v0.y + 1.) * height / 2.;
      int x1 = (v1.x + 1.) * width / 2.;
      int y1 = (v1.y + 1.) * height / 2.;
      line(x0, y0, x1, y1, image, white);
    }
  }

  image.flip_vertically();  // i want to have the origin at the left bottom corner of the image
  image.write_tga_file("output2.tga");
}

int main() {
  renderLines();
  renderModel();
  return 0;
}
