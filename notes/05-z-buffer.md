# Hidden faces removal（隐藏面消除）

## Revisit the back-face culling（重温背面剔除）

Let us put aside barycentric coordinates for a moment and return to the [rasterization with back-face culling](https://haqr.eu/tinyrenderer/rasterization/#putting-all-together-back-face-culling). Recall that the idea is to loop through all the triangles in the input model and discard the back-facing ones. Check line 2 in the following pseudocode:

> 我们先把重心坐标放一放，回到带背面剔除的光栅化上来。回忆一下，当时的思路是遍历输入模型中的所有三角形，把背朝镜头的那些丢弃掉。请看下面伪代码的第 2 行：

```
for each triangle t:
    if t is not back-facing:
        for each pixel p that t covers:
            paint the pixel
```

This [commit](https://github.com/ssloy/tinyrenderer/tree/ab559b1325df38266b420c6236a33794bd7e4c65) from the rasterization lesson was used to draw the following image. Unfortunately, the image has some artifacts - check, for example, the right hip of the demon. Clearly, it is a mess:

> 下面的图片就是用光栅化那一课的这个 [commit](https://github.com/ssloy/tinyrenderer/tree/ab559b1325df38266b420c6236a33794bd7e4c65) 绘制出来的。很遗憾，图片有些瑕疵——比如看看恶魔的右髋部，明摆着，一团糟：

![Image 3](https://haqr.eu/tinyrenderer/z-buffer/diablo-culling-hip.png)

How can we eliminate all the hidden surfaces?

> 我们怎样才能消除所有隐藏面呢？

## Painter's algorithm（画家算法）

In theory, we could simply draw all the triangles without discarding any. If we do it properly in rear-to-front order, the front facets will overwrite the back ones. This technique is called the [painter's algorithm](http://en.wikipedia.org/wiki/Painter%27s_algorithm). Here is the pseudocode:

> 理论上很简单：一个都不丢弃，把所有三角形统统画出来。只要老老实实按从后到前的顺序绘制，靠前的面就会覆盖掉靠后的面。这种技术叫做[画家算法](http://en.wikipedia.org/wiki/Painter%27s_algorithm)。伪代码如下：

```
sort triangles by depth
for each triangle t:
    for each pixel p that t covers:
        paint the pixel
```

Here is an animation of the process. Note that no triangle was actually discarded; I simply painted them all in the rear-to-front order:

> 下面是这个过程的动画。注意，其实并没有丢弃任何三角形；我只是按从后到前的顺序把它们全部画了一遍：

![Image 4](https://haqr.eu/tinyrenderer/z-buffer/painter.gif)

And here is the final render:

> 最终渲染结果如下：

![Image 5](https://haqr.eu/tinyrenderer/z-buffer/diablo-painter.png)

To produce this image, I did not modify the rendering part. At the moment, I use an orthogonal projection to the screen, so I simply sorted the triangles by their maximum z-coordinate during the loading of the model. Check lines 35–55 of the [following listing](https://github.com/ssloy/tinyrenderer/commit/52c7f8898e1a71c412f89240d83e4b354a3e76bc).

> 为了生成这张图，我没有改动渲染部分。目前我使用的是到屏幕的正交投影，所以干脆在加载模型时按三角形的最大 z 坐标做了排序。请查看下面清单的第 35–55 行。

> __Warning__: Do not spend too much time trying to understand the code. You can just trust that it reorders the triangles by their max z-coordinate. Anyway, this code will eventually end up in the trash bin.

> **警告**：不要花太多时间去试图理解这段代码。你尽可以相信它会按三角形的最大 z 坐标重新排序。总之，这段代码最终会被扔进垃圾桶。

**Sort triangles by maximum depth**

```c++
#include <fstream>
#include <sstream>
#include "model.h"
#include <algorithm>

Model::Model(const std::string filename) {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            vec3 v;
            for (int i : {0,1,2}) iss >> v[i];
            verts.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            int f,t,n, cnt = 0;
            iss >> trash;
            while (iss >> f >> trash >> t >> trash >> n) {
                facet_vrt.push_back(--f);
                cnt++;
            }
            if (3!=cnt) {
                std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
                return;
            }
        }
    }
    std::cerr << "# v# " << nverts() << " f# "  << nfaces() << std::endl;

    std::vector<int> idx(nfaces());    // permutation, a map from new to old facet indices
    for (int i = 0 ; i<nfaces() ; i++) // we start with the identity
        idx[i] = i;

    std::sort(idx.begin(), idx.end(),
              [&](const int& a, const int& b) { // given two triangles, compare their min z coordinate
                  float aminz = std::min(vert(a, 0).z,
                                         std::min(vert(a, 1).z,
                                                  vert(a, 2).z));
                  float bminz = std::min(vert(b, 0).z,
                                         std::min(vert(b, 1).z,
                                                  vert(b, 2).z));
                  return aminz < bminz;
              });

    std::vector<int> facet_vrt2(nfaces()*3); // allocate an array to store permutated facets
    for (int i=0; i<nfaces(); i++)           // for each (new) facet
        for (int j=0; j<3; j++)              // copy its three vertices from the old array
            facet_vrt2[i*3+j] = facet_vrt[idx[i]*3+j];

    facet_vrt = facet_vrt2;                  // store the sorted triangles
}

int Model::nverts() const { return verts.size(); }
int Model::nfaces() const { return facet_vrt.size()/3; }

vec3 Model::vert(const int i) const {
    return verts[i];
}

vec3 Model::vert(const int iface, const int nthvert) const {
    return verts[facet_vrt[iface*3+nthvert]];
}
```

Unfortunately, this approach comes with a high computational cost: for each camera movement, we need to re-sort the entire scene. And then there are dynamic scenes... But that is not even the main problem. The key issue is that it is not always possible to determine the correct order. Consider, for example, this scene:

> 可惜这个做法的计算代价很高：相机每动一次，就得把整个场景重新排序。更别提还有动态场景……但这还不是主要问题。关键在于，我们并不总能确定正确的顺序。比如看看这个场景：

![Image 6](https://haqr.eu/tinyrenderer/z-buffer/painter.svg)

There are ways to circumvent these drawbacks. The problem lies in the fact that triangles are not ideal primitives for global ordering. Instead of handling visibility in a per-triangle manner, it would be better to handle it per pixel.

> 这些缺陷确实有办法绕开。问题的根源在于，三角形并不是做全局排序的理想图元。与其按三角形来处理可见性，不如按像素来处理。

## Depth interpolation（深度插值）

But first, let us throw away this sorting approach and return to our starting point. We simply iterate through all triangles and paint all the pixels in each triangle. Since the triangles are not correctly sorted, visual artifacts will appear.

> 不过首先，让我们丢掉排序这套做法，回到起点：简单地遍历所有三角形，把每个三角形覆盖的全部像素都画出来。由于三角形没有正确排序，画面就会出现瑕疵。

In addition to painting pixels in the frame buffer, I propose creating a second grayscale image of the same dimensions. In this image, we will paint the depth of the triangles, just as we did in the [last listing of the barycentric coordinates lesson](https://haqr.eu/tinyrenderer/barycentric/#example-2_1). Black pixels will represent distant triangles, and white pixels will represent the closest ones.

> 除了往帧缓冲里画像素，我建议再创建一张同样尺寸的灰度图。在这张图里，我们把三角形的深度也画进去，就像重心坐标那一课最后一个清单里做的那样。黑色像素代表远处的三角形，白色像素代表最近的三角形。

Here is the pseudocode:

> 伪代码如下：

```
for each triangle t:
    for each pixel p that t covers:
        compute its depth z
        update the depth buffer with z
        paint the pixel
```

You can find the complete code [here](https://github.com/ssloy/tinyrenderer/commit/af1c138fd86cd9d46813ee492e97127d434a9242). Note that I have removed back-face culling from the pseudocode. However, for consistency in the tutorial's images, I kept it in the code. In the approach we are developing, back-face culling is not mandatory; it can be seen as an optimization.

> 完整代码在[这里](https://github.com/ssloy/tinyrenderer/commit/af1c138fd86cd9d46813ee492e97127d434a9242)。注意，我在伪代码里去掉了背面剔除；但为了和教程里的图片保持一致，代码里仍然保留着它。在我们正在开发的这套方案中，背面剔除并非必需，完全可以把它看作一种优化。

**Depth interpolation**

```c++
#include <cmath>
#include <tuple>
#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

constexpr int width  = 800;
constexpr int height = 800;

double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5*((by-ay)*(bx+ax) + (cy-by)*(cx+bx) + (ay-cy)*(ax+cx));
}

void triangle(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage &zbuffer, TGAImage &framebuffer, TGAColor color) {
    int bbminx = std::min(std::min(ax, bx), cx); // bounding box for the triangle
    int bbminy = std::min(std::min(ay, by), cy); // defined by its top left and bottom right corners
    int bbmaxx = std::max(std::max(ax, bx), cx);
    int bbmaxy = std::max(std::max(ay, by), cy);
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area<1) return; // backface culling + discarding triangles that cover less than a pixel

#pragma omp parallel for
    for (int x=bbminx; x<=bbmaxx; x++) {
        for (int y=bbminy; y<=bbmaxy; y++) {
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta  = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha<0 || beta<0 || gamma<0) continue; // negative barycentric coordinate => the pixel is outside the triangle

            unsigned char z = static_cast<unsigned char>(alpha * az + beta * bz + gamma * cz);
            zbuffer.set(x, y, {z});
            framebuffer.set(x, y, color);
        }
    }
}

std::tuple<int,int,int> project(vec3 v) { // First of all, (x,y) is an orthogonal projection of the vector (x,y,z).
    return { (v.x + 1.) *  width/2,       // Second, since the input models are scaled to have fit in the [-1,1]^3 world coordinates,
             (v.y + 1.) * height/2,       // we want to shift the vector (x,y) and then scale it to span the entire screen.
             (v.z + 1.) *   255./2 };
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    Model model(argv[1]);
    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage     zbuffer(width, height, TGAImage::GRAYSCALE);

    for (int i=0; i<model.nfaces(); i++) { // iterate through all triangles
        auto [ax, ay, az] = project(model.vert(i, 0));
        auto [bx, by, bz] = project(model.vert(i, 1));
        auto [cx, cy, cz] = project(model.vert(i, 2));
        TGAColor rnd;
        for (int c=0; c<3; c++) rnd[c] = std::rand()%255;
        triangle(ax, ay, az, bx, by, bz, cx, cy, cz, zbuffer, framebuffer, rnd);
    }

    framebuffer.write_tga_file("framebuffer.tga");
    zbuffer.write_tga_file("zbuffer.tga");
    return 0;
}
```

Check the highlighted lines. This code produces two images, namely `framebuffer.tga` and `zbuffer.tga`. Note line 39: the `project` function now produces a triplet. The depth values are mapped to the range \([0, 255]\) (recall that my input models are normalized to fit in the \([-1, 1]^3\) cube).

> 请留意高亮的行。这段代码会生成两张图片，即 `framebuffer.tga` 和 `zbuffer.tga`。注意第 39 行：`project` 函数现在返回一个三元组。深度值被映射到 \([0, 255]\) 区间（回忆一下，我的输入模型都被归一化到 \([-1, 1]^3\) 的立方体内）。

Here are the resulting images. Note the discontinuities on the demon’s right hip, matching the mess in the framebuffer:

> 生成的图片如下。注意恶魔右髋部那些不连续的地方，与帧缓冲里的混乱刚好对应：

![Image 7](https://haqr.eu/tinyrenderer/z-buffer/diablo-zbuffer-bad.png)

## Per-pixel painter's algorithm (a.k.a. z-buffer)（逐像素的画家算法，又称 z 缓冲）

Now that we have a depth buffer, a single line will suffice to eliminate all the artifacts - check line 4:

> 现在有了深度缓冲，只要一行代码就能消除所有瑕疵——看第 4 行：

```
for each triangle t:
    for each pixel p that t covers:
        compute its depth z
        if depth buffer at p < z:
            update the depth buffer with z
            paint the pixel
```

Our depth buffer allows us to track the depth of each pixel we paint. If, for some triangle `t`, the depth of some pixel `p` is inferior to the one we have already painted, we can safely discard it.
Otherwise, we paint the pixel and update the depth buffer. So, at the expense of keeping a rather small buffer, we can keep the track of the frontmost surface being drawn.

> 深度缓冲让我们能记录所画每个像素的深度。如果对某个三角形 `t`，某个像素 `p` 的深度比我们已经画过的那个更小，就可以放心地丢弃它。
> 否则，我们就把这个像素画上，并更新深度缓冲。也就是说，只需额外维护一个相当小的缓冲，我们就能始终跟踪正在绘制的最靠前的表面。

Here is the update to the previous code - check line 30:

> 对之前的代码做如下更新——看第 30 行：

**Z-buffer hidden surfaces removal**

```c++
#include <cmath>
#include <tuple>
#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

constexpr int width  = 800;
constexpr int height = 800;

double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5*((by-ay)*(bx+ax) + (cy-by)*(cx+bx) + (ay-cy)*(ax+cx));
}

void triangle(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage &zbuffer, TGAImage &framebuffer, TGAColor color) {
    int bbminx = std::min(std::min(ax, bx), cx); // bounding box for the triangle
    int bbminy = std::min(std::min(ay, by), cy); // defined by its top left and bottom right corners
    int bbmaxx = std::max(std::max(ax, bx), cx);
    int bbmaxy = std::max(std::max(ay, by), cy);
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area<1) return; // backface culling + discarding triangles that cover less than a pixel

#pragma omp parallel for
    for (int x=bbminx; x<=bbmaxx; x++) {
        for (int y=bbminy; y<=bbmaxy; y++) {
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta  = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha<0 || beta<0 || gamma<0) continue; // negative barycentric coordinate => the pixel is outside the triangle

            unsigned char z = static_cast<unsigned char>(alpha * az + beta * bz + gamma * cz);
            if (z <= zbuffer.get(x, y)[0]) continue;
            zbuffer.set(x, y, {z});
            framebuffer.set(x, y, color);
        }
    }
}

std::tuple<int,int,int> project(vec3 v) { // First of all, (x,y) is an orthogonal projection of the vector (x,y,z).
    return { (v.x + 1.) *  width/2,       // Second, since the input models are scaled to have fit in the [-1,1]^3 world coordinates,
             (v.y + 1.) * height/2,       // we want to shift the vector (x,y) and then scale it to span the entire screen.
             (v.z + 1.) *   255./2 };
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    Model model(argv[1]);
    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage     zbuffer(width, height, TGAImage::GRAYSCALE);

    for (int i=0; i<model.nfaces(); i++) { // iterate through all triangles
        auto [ax, ay, az] = project(model.vert(i, 0));
        auto [bx, by, bz] = project(model.vert(i, 1));
        auto [cx, cy, cz] = project(model.vert(i, 2));
        TGAColor rnd;
        for (int c=0; c<3; c++) rnd[c] = std::rand()%255;
        triangle(ax, ay, az, bx, by, bz, cx, cy, cz, zbuffer, framebuffer, rnd);
    }

    framebuffer.write_tga_file("framebuffer.tga");
    zbuffer.write_tga_file("zbuffer.tga");
    return 0;
}
```

And here is the result. You can see that the demon’s right paw is lighter than the left one, it appears darker since it is behind the demon:

> 结果如下。可以看到恶魔的右爪比左爪更亮；左爪显得更暗，因为它位于恶魔身后：

![Image 8](https://haqr.eu/tinyrenderer/z-buffer/diablo-zbuffer.png)

As usual, you can find the complete code [here](https://github.com/ssloy/tinyrenderer/commit/f1dd24ec9418922fcfa3d6e7ab13ffc8c1c18577).
In this second edition of the tutorial, I’ll postpone the shading of the model a bit. First, we will dive into some mathematics necessary for camera handling.

> 和往常一样，完整代码在[这里](https://github.com/ssloy/tinyrenderer/commit/f1dd24ec9418922fcfa3d6e7ab13ffc8c1c18577)。
> 在本教程的第二版中，我会把模型的着色稍稍推后。我们要先深入一些处理相机所必需的数学。

## Homework assignment（课后作业）

Right now there is a very rudimentrary implementation of `vec3` class in the repository:

> 目前仓库里的 `vec3` 类是一个非常简陋的实现：

**Rudimentary vec3 class**

```c++
#pragma once
#include <cmath>
#include <cassert>
#include <iostream>

template<int n> struct vec {
    double data[n] = {0};
    double& operator[](const int i)       { assert(i>=0 && i<n); return data[i]; }
    double  operator[](const int i) const { assert(i>=0 && i<n); return data[i]; }
};

template<int n> std::ostream& operator<<(std::ostream& out, const vec<n>& v) {
    for (int i=0; i<n; i++) out << v[i] << " ";
    return out;
}

template<> struct vec<3> {
    double x = 0, y = 0, z = 0;
    double& operator[](const int i)       { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
    double  operator[](const int i) const { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
};

typedef vec<3> vec3;
```

For the moment `vec3` is only used to store triplets of `double`, but very soon we will need to add and subtract vectors, compute dot products and so on.
So, here is a homework assignment: implement `vec2`, `vec3` and `vec4` classes with basic vector operations.
Moreover, we will need small matrices (up to 4x4 maximum) and basic operations (mainly, multiplication, transposition and inversion).
It is handy to be able to invert any square matrix, but 3x3 inversion may suffice if you do not wish implement a generic inversion.

> 目前 `vec3` 只用来存放 `double` 三元组，但很快我们就要做向量加减、点积之类的运算。
> 所以，课后作业来了：实现 `vec2`、`vec3` 和 `vec4` 类，支持基本的向量运算。
> 此外，我们还会需要小矩阵（最大 4x4 即可）和基本运算（主要是乘法、转置和求逆）。
> 能对任意方阵求逆会很方便，但如果你不想实现通用的求逆，3x3 求逆也够用了。

> 原文：[Lesson 4: Hidden faces removal](https://haqr.eu/tinyrenderer/z-buffer/)
