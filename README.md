# Software rendering in 500 lines of bare C++（500 行裸 C++ 实现软件渲染）

The code itself is of little interest. Check the course notes:

> 代码本身没什么可看的，重点在课程笔记：

1. [Introduction](https://haqr.eu/tinyrenderer/)
2. [Bresenham’s line drawing algorithm](https://haqr.eu/tinyrenderer/bresenham/)
3. [Triangle rasterization](https://haqr.eu/tinyrenderer/rasterization/)
4. [Primer on barycentric coordinates](https://haqr.eu/tinyrenderer/barycentric/)
5. [Hidden faces removal](https://haqr.eu/tinyrenderer/z-buffer/)
6. [A crude (but simple) approach to camera handling](https://haqr.eu/tinyrenderer/camera-naive/)
7. [Better camera handling](https://haqr.eu/tinyrenderer/camera/)
8. [Shading](https://haqr.eu/tinyrenderer/shading/)
9. [More data!](https://haqr.eu/tinyrenderer/textures/)
10. [Tangent space normal mapping](https://haqr.eu/tinyrenderer/tangent/)
11. [Shadow mapping](https://haqr.eu/tinyrenderer/shadow/)
12. [Indirect lighting](https://haqr.eu/tinyrenderer/ssao/)
13. [Bonus: toon shading](https://haqr.eu/tinyrenderer/toon/)
14. [Afterword](https://haqr.eu/tinyrenderer/afterword/)

> 1. [引言](https://haqr.eu/tinyrenderer/)
> 2. [布雷森汉姆画线算法](https://haqr.eu/tinyrenderer/bresenham/)
> 3. [三角形光栅化](https://haqr.eu/tinyrenderer/rasterization/)
> 4. [重心坐标入门](https://haqr.eu/tinyrenderer/barycentric/)
> 5. [隐藏面消除](https://haqr.eu/tinyrenderer/z-buffer/)
> 6. [粗暴但简单的相机处理方式](https://haqr.eu/tinyrenderer/camera-naive/)
> 7. [更好的相机处理](https://haqr.eu/tinyrenderer/camera/)
> 8. [着色](https://haqr.eu/tinyrenderer/shading/)
> 9. [更多数据！](https://haqr.eu/tinyrenderer/textures/)
> 10. [切线空间法线贴图](https://haqr.eu/tinyrenderer/tangent/)
> 11. [阴影贴图](https://haqr.eu/tinyrenderer/shadow/)
> 12. [间接光照](https://haqr.eu/tinyrenderer/ssao/)
> 13. [附赠：卡通着色](https://haqr.eu/tinyrenderer/toon/)
> 14. [后记](https://haqr.eu/tinyrenderer/afterword/)

In this series of articles, I aim to demonstrate how OpenGL, Vulkan, Metal, and DirectX work by writing a simplified clone from scratch.
Surprisingly, many people struggle with the initial hurdle of learning a 3D graphics API.
To help with this, I have prepared a short series of lectures, after which my students are able to produce quite capable renderers.

> 在这一系列文章中，我的目标是从零写一个简化版渲染器，来演示 OpenGL、Vulkan、Metal 和 DirectX 的工作原理。
> 令人意外的是，很多人在学习 3D 图形 API 时卡在了最初的门槛上。
> 为了帮助他们，我准备了这个简短的系列课程。学完这些课程之后，我的学生们都能写出相当能打的渲染器。

The task is as follows: using no third-party libraries (especially graphics-related ones), we will generate an image like this:

> 任务是这样的：不使用任何第三方库（尤其是图形相关的库），生成下面这样的图像：

![](https://haqr.eu/tinyrenderer/home/africanhead.png)

_Warning: This is a training material that loosely follows the structure of modern 3D graphics libraries.
It is a **software renderer**.
**I do not intend to show how to write GPU applications — I want to show how they work.**
I firmly believe that understanding this is essential for writing efficient applications using 3D libraries._

> _警告：这是一份教学材料，大体上按照现代 3D 图形库的结构来组织。
> 它是一个**软件渲染器**。
> **我并不打算教你如何编写 GPU 应用——我想展示的是它们的工作原理。**
> 我坚信，理解这一点对于用 3D 库编写高效应用至关重要。_

## The starting point（起点）

The final code consists of about 500 lines.
My students typically require 10 to 20 hours of programming to start producing such renderers.
The input is a 3D model composed of a triangulated mesh and textures.
The output is a rendering.
There is no graphical interface, the program simply generates an image.

> 最终代码大约 500 行。
> 我的学生们通常需要 10 到 20 小时的编程练习，才能开始做出这样的渲染器。
> 输入是一个由三角网格和纹理组成的 3D 模型。
> 输出是一张渲染图。
> 没有图形界面，程序只负责生成图像。

To minimize external dependencies, I provide my students with a single class for handling [TGA](http://en.wikipedia.org/wiki/Truevision_TGA) files —
one of the simplest formats supporting RGB, RGBA, and grayscale images.
This serves as our foundation for image manipulation.
At the beginning, the only available functionality (besides loading and saving images) is the ability to set the color of a single pixel.

> 为了尽量少依赖外部库，我只给我的学生们提供一个处理 [TGA](http://en.wikipedia.org/wiki/Truevision_TGA) 文件的类——
> TGA 是最简单的图像格式之一，支持 RGB、RGBA 和灰度图。
> 这就是我们进行图像操作的基础。
> 一开始，除了加载和保存图像，唯一可用的功能就是设置单个像素的颜色。

There are no built-in functions for drawing line segments or triangles — we will implement all of this manually.
While I provide my own source code, written alongside my students, I do not recommend using it directly, as doing the work yourself is essential to understanding the concepts.
The complete code is available on [github](https://github.com/ssloy/tinyrenderer), and you can find the initial source code I provide to my students [here](https://github.com/ssloy/tinyrenderer/tree/706b2dfecff65daeb93de568ee2c2bd87f277860).
Behold, here is the starting point:

> 没有任何现成的画线段或三角形的函数——这些都要靠我们自己手写实现。
> 虽然我提供了自己的源码（和学生们一起写的），但我并不建议直接拿来用，因为亲自动手做一遍才是理解概念的关键。
> 完整代码在 [github](https://github.com/ssloy/tinyrenderer) 上，我提供给学生的初始源码在[这里](https://github.com/ssloy/tinyrenderer/tree/706b2dfecff65daeb93de568ee2c2bd87f277860)。
> 看好了，这就是起点：

```cpp
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

> 它生成一张 64x64 的图像 `framebuffer.tga`，这里我放大了方便看清：

![](https://haqr.eu/tinyrenderer/bresenham/bresenham0.png)


## Teaser: few examples made with the renderer（预告：用这个渲染器做出的几个例子）

![](https://haqr.eu/tinyrenderer/home/demon.png)

![](https://haqr.eu/tinyrenderer/home/diablo-glow.png)

![](https://haqr.eu/tinyrenderer/home/boggie.png)

![](https://haqr.eu/tinyrenderer/home/diablo-ssao.png)

## Compilation（编译）

```sh
git clone https://github.com/ssloy/tinyrenderer.git &&
cd tinyrenderer &&
cmake -Bbuild &&
cmake --build build -j &&
build/tinyrenderer obj/diablo3_pose/diablo3_pose.obj obj/floor.obj
```
The rendered image is saved to `framebuffer.tga`.

> 渲染出的图像保存为 `framebuffer.tga`。
