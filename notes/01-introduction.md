# Software rendering in 500 lines of bare C++（500 行裸 C++ 实现软件渲染）

In this series of articles, I aim to demonstrate how OpenGL, Vulkan, Metal, and DirectX work by writing a simplified clone from scratch.
Surprisingly, many people struggle with the initial hurdle of learning a 3D graphics API.
To help with this, I have prepared a short series of lectures, after which my students are able to produce quite capable renderers.

> 在这一系列文章中，我将通过从零编写一个简化版的克隆实现，来演示 OpenGL、Vulkan、Metal 和 DirectX 是如何工作的。
> 出人意料的是，很多人在学习 3D 图形 API 的第一道坎上就卡住了。
> 为此，我准备了一系列简短的课程，学完之后，我的学生就能做出相当像样的渲染器。

The task is as follows: using no third-party libraries (especially graphics-related ones), we will generate an image like this:

> 任务是这样的：不使用任何第三方库（尤其是与图形相关的库），我们要生成这样一张图片：

![Image 1](https://haqr.eu/home/africanhead.png)

_Warning: This is a training material that loosely follows the structure of modern 3D graphics libraries.
It is a __software renderer__.
__I do not intend to show how to write GPU applications — I want to show how they work.__
I firmly believe that understanding this is essential for writing efficient applications using 3D libraries._

> 警告：这是一份训练材料，它大致沿用了现代 3D 图形库的结构。
> 它是一个__软件渲染器__。
> __我无意展示如何编写 GPU 应用——我想展示的是它们如何工作。__
> 我坚信，理解这一点对于用 3D 图形库写出高效的应用来说至关重要。

## The starting point（起点）

The final code consists of about 500 lines.
My students typically require 10 to 20 hours of programming to start producing such renderers.
The input is a 3D model composed of a triangulated mesh and textures.
The output is a rendereding.
There is no graphical interface, the program simply generates an image.

> 最终的代码大约 500 行。
> 我的学生通常需要编程 10 到 20 个小时，才能做出这样的渲染器。
> 输入是一个由三角网格（mesh）和纹理组成的 3D 模型。
> 输出是一张渲染图。
> 没有图形界面，程序只是简单地生成一张图片。

To minimize external dependencies, I provide my students with a single class for handling [TGA](http://en.wikipedia.org/wiki/Truevision_TGA) files —
one of the simplest formats supporting RGB, RGBA, and grayscale images.
This serves as our foundation for image manipulation.
At the beginning, the only available functionality (besides loading and saving images) is the ability to set the color of a single pixel.

> 为了尽量减少外部依赖，我给学生提供了一个处理 [TGA](http://en.wikipedia.org/wiki/Truevision_TGA) 文件的类——
> TGA 是最简单的图像格式之一，支持 RGB、RGBA 和灰度图像。
> 它将作为我们处理图像的基础。
> 一开始，除了加载和保存图像之外，唯一可用的功能就是设置单个像素的颜色。

There are no built-in functions for drawing line segments or triangles — we will implement all of this manually.
While I provide my own source code, written alongside my students, I do not recommend using it directly, as doing the work yourself is essential to understanding the concepts.
The complete code is available on [github](https://github.com/ssloy/tinyrenderer), and you can find the initial source code I provide to my students [here](https://github.com/ssloy/tinyrenderer/tree/706b2dfecff65daeb93de568ee2c2bd87f277860).
Behold, here is the starting point:

> 没有任何现成的线段或三角形绘制函数——这些我们都会亲手实现。
> 虽然我会提供自己和学生一起写出的源码，但不建议直接拿来用，因为亲自动手才是理解这些概念的关键。
> 完整代码可以在 [github](https://github.com/ssloy/tinyrenderer) 上找到，我给学生提供的初始源码在[这里](https://github.com/ssloy/tinyrenderer/tree/706b2dfecff65daeb93de568ee2c2bd87f277860)。
> 瞧，这就是起点：

main.cpp

```
#include "tgaimage.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

int main(int argc, char** argv) {
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    int ax =  7, ay =  3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    framebuffer.set(ax, ay, white);
    framebuffer.set(bx, by, white);
    framebuffer.set(cx, cy, white);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
```

It produces the 64x64 image `framebuffer.tga`, here I scaled it for better readability:

> 它会生成 64x64 的图像 `framebuffer.tga`，为了看得更清楚，我把它放大了：

![Image 2](https://haqr.eu/bresenham/bresenham0.png)

## Compilation（编译）

```
git clone https://github.com/ssloy/tinyrenderer.git &&
cd tinyrenderer &&
cmake -Bbuild &&
cmake --build build -j &&
build/tinyrenderer obj/diablo3_pose/diablo3_pose.obj obj/floor.obj
```

The rendered image is saved to `framebuffer.tga`.

> 渲染得到的图像会保存到 `framebuffer.tga`。

## Teaser: few examples made with the renderer（先睹为快：用这个渲染器做出的几个示例）

![Image 3](https://haqr.eu/home/demon.png)

![Image 4](https://haqr.eu/home/diablo-glow.png)

![Image 5](https://haqr.eu/home/boggie.png)

![Image 6](https://haqr.eu/home/diablo-ssao.png)

> 原文：[Introduction](https://haqr.eu/tinyrenderer/)
