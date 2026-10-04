# Shading（着色）

## The rendering pipeline（渲染管线）

[The modern rendering pipeline](https://www.khronos.org/opengl/wiki/Rendering_Pipeline_Overview) can be represented as follows:

> [现代渲染管线](https://www.khronos.org/opengl/wiki/Rendering_Pipeline_Overview)可以用下图来表示：

![Image 3](https://haqr.eu/tinyrenderer/shading/pipeline.svg)

It consists of the following main stages:

> 它由以下几个主要阶段组成：

- Vertex processing: processes individual vertices.
- Primitive assembly: connects vertices to form primitives.
- Rasterization: converts each primitive into a set of fragments.
- Fragment processing: processes individual fragments.
- Output merging: combines the fragments of all primitives (in 3D space) into a 2D color pixel for the display.

> - 顶点处理（Vertex processing）：逐个处理各个顶点。
> - 图元装配（Primitive assembly）：把顶点连接起来组成图元。
> - 光栅化（Rasterization）：把每个图元转换成一组片元。
> - 片段处理（Fragment processing）：逐个处理各个片元。
> - 输出合并（Output merging）：把（3D 空间中）所有图元的片元合并成显示器上的一个 2D 彩色像素。

Note the distinction between fragments and pixels.
Pixels are the dots on the display. A pixel is 2-dimensional, with a (x, y) position and a color value.
A fragment is 3-dimensional, with a (x, y, z) position. The (x, y) are aligned with the 2D pixel-grid. The z-value (not grid-aligned) denotes its depth.
In addition to that, fragment processing stage assigns other attributes to the fragments such as color and alpha, that are to be combined in the output merging stage.

> 注意区分片元（fragment）和像素（pixel）。像素是显示器上的点：像素是 2D 的，有一个 (x, y) 位置和一个颜色值。片元是 3D 的，有一个 (x, y, z) 位置：其中的 (x, y) 与 2D 像素网格对齐，而 z 值（并不与网格对齐）表示它的深度。除此之外，片段处理阶段还会给片元赋予颜色、alpha 等其他属性，这些属性会在输出合并阶段被合并。

Vertex processing and fragment processing are programmable whereas the rest of the pipeline is hardwired in the GPU.

> 顶点处理和片段处理是可编程的，而管线的其余部分则固化在 GPU 之中。

## Refactoring（重构）

The time has come for a small refactoring ([check the commit](https://github.com/ssloy/tinyrenderer/commit/637223c98ae9610f7d9ff2c200e29aa837a162d5)), here I split `main.cpp` in two.
The idea is to mimic the rendering pipeline, I have extracted the parts of the code that the programmer cannot modify.
Roughly speaking, it corresponds to the binary of the OpenGL library and the hardwired GPU rasterization.

> 是时候做一次小重构了（[看看对应的提交](https://github.com/ssloy/tinyrenderer/commit/637223c98ae9610f7d9ff2c200e29aa837a162d5)），这里我把 `main.cpp` 拆成了两部分。思路是模仿渲染管线的结构：我把程序员无法修改的那部分代码抽了出来，粗略地说，它们相当于 OpenGL 库的二进制文件加上 GPU 中固化的光栅化部分。

Here are the files that we will no longer modify:

> 以下是我们不会再修改的文件：

our_gl.h

```cpp
#include "tgaimage.h"
#include "geometry.h"

void lookat(const vec3 eye, const vec3 center, const vec3 up);
void init_perspective(const double f);
void init_viewport(const int x, const int y, const int w, const int h);
void init_zbuffer(const int width, const int height);

struct IShader {
    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const = 0;
};

typedef vec4 Triangle[3]; // a triangle primitive is made of three ordered points
void rasterize(const Triangle &clip, const IShader &shader, TGAImage &framebuffer);
```

our_gl.cpp

```cpp
#include <algorithm>
#include "our_gl.h"

mat<4,4> ModelView, Viewport, Perspective; // "OpenGL" state matrices
std::vector<double> zbuffer;               // depth buffer

void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    vec3 n = normalized(eye-center);
    vec3 l = normalized(cross(up,n));
    vec3 m = normalized(cross(n, l));
    ModelView = mat<4,4>{{{l.x,l.y,l.z,0}, {m.x,m.y,m.z,0}, {n.x,n.y,n.z,0}, {0,0,0,1}}} *
                mat<4,4>{{{1,0,0,-center.x}, {0,1,0,-center.y}, {0,0,1,-center.z}, {0,0,0,1}}};
}

void init_perspective(const double f) {
    Perspective = {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0, -1/f,1}}};
}

void init_viewport(const int x, const int y, const int w, const int h) {
    Viewport = {{{w/2., 0, 0, x+w/2.}, {0, h/2., 0, y+h/2.}, {0,0,1,0}, {0,0,0,1}}};
}

void init_zbuffer(const int width, const int height) {
    zbuffer = std::vector(width*height, -1000.);
}

void rasterize(const Triangle &clip, const IShader &shader, TGAImage &framebuffer) {
    vec4 ndc[3]    = { clip[0]/clip[0].w, clip[1]/clip[1].w, clip[2]/clip[2].w };                // normalized device coordinates
    vec2 screen[3] = { (Viewport*ndc[0]).xy(), (Viewport*ndc[1]).xy(), (Viewport*ndc[2]).xy() }; // screen coordinates

    mat<3,3> ABC = {{ {screen[0].x, screen[0].y, 1.}, {screen[1].x, screen[1].y, 1.}, {screen[2].x, screen[2].y, 1.} }};
    if (ABC.det()<1) return; // backface culling + discarding triangles that cover less than a pixel

    auto [bbminx,bbmaxx] = std::minmax({screen[0].x, screen[1].x, screen[2].x}); // bounding box for the triangle
    auto [bbminy,bbmaxy] = std::minmax({screen[0].y, screen[1].y, screen[2].y}); // defined by its top left and bottom right corners
#pragma omp parallel for
    for (int x=std::max<int>(bbminx, 0); x<=std::min<int>(bbmaxx, framebuffer.width()-1); x++) {         // clip the bounding box by the screen
        for (int y=std::max<int>(bbminy, 0); y<=std::min<int>(bbmaxy, framebuffer.height()-1); y++) {
            vec3 bc = ABC.invert_transpose() * vec3{static_cast<double>(x), static_cast<double>(y), 1.}; // barycentric coordinates of {x,y} w.r.t the triangle
            if (bc.x<0 || bc.y<0 || bc.z<0) continue;                                                    // negative barycentric coordinate => the pixel is outside the triangle
            double z = bc * vec3{ ndc[0].z, ndc[1].z, ndc[2].z };  // linear interpolation of the depth
            if (z <= zbuffer[x+y*framebuffer.width()]) continue;   // discard fragments that are too deep w.r.t the z-buffer
            auto [discard, color] = shader.fragment(bc);
            if (discard) continue;                                 // fragment shader can discard current fragment
            zbuffer[x+y*framebuffer.width()] = z;                  // update the z-buffer
            framebuffer.set(x, y, color);                          // update the framebuffer
        }
    }
}
```

I have isolated three state matrices `ModelView`, `Viewport` and `Perspective` (and helper functions to initialize them), as well as the `zbuffer` and triangle rasterization routine.
Note the abstract class `IShader`.
Each time the rasterizer draws a pixel in the framebuffer, it calls the virtual function `fragment` to decide the color of the pixel.
Therefore, the programmer must define a shader object and pass it to the `rasterize` function.

> 我把三个状态矩阵 `ModelView`、`Viewport` 和 `Perspective`（以及用来初始化它们的辅助函数）隔离了出来，还有 `zbuffer` 和三角形光栅化例程。注意那个抽象类 `IShader`：每当光栅化器要在帧缓冲里画一个像素时，它都会调用虚函数 `fragment` 来决定该像素的颜色。因此，程序员必须定义一个着色器对象，并把它传给 `rasterize` 函数。

Here is what is left of `main.cpp`:

> 下面是 `main.cpp` 中剩下的内容：

main.cpp

```cpp
#include <cstdlib>
#include "our_gl.h"
#include "model.h"

extern mat<4,4> ModelView, Perspective; // "OpenGL" state matrices and
extern std::vector<double> zbuffer;     // the depth buffer

struct RandomShader : IShader {
    const Model &model;
    TGAColor color = {};
    vec3 tri[3];  // triangle in eye coordinates

    RandomShader(const Model &m) : model(m) {
    }

    virtual vec4 vertex(const int face, const int vert) {
        vec3 v = model.vert(face, vert);                          // current vertex in object coordinates
        vec4 gl_Position = ModelView * vec4{v.x, v.y, v.z, 1.};
        tri[vert] = gl_Position.xyz();                            // in eye coordinates
        return Perspective * gl_Position;                         // in clip coordinates
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        return {false, color};                                    // do not discard the pixel
    }
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    constexpr int width  = 800;      // output image size
    constexpr int height = 800;
    constexpr vec3    eye{-1, 0, 2}; // camera position
    constexpr vec3 center{ 0, 0, 0}; // camera direction
    constexpr vec3     up{ 0, 1, 0}; // camera up vector

    lookat(eye, center, up);                                   // build the ModelView   matrix
    init_perspective(norm(eye-center));                        // build the Perspective matrix
    init_viewport(width/16, height/16, width*7/8, height*7/8); // build the Viewport    matrix
    init_zbuffer(width, height);
    TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});

    for (int m=1; m<argc; m++) {                    // iterate through all input objects
        Model model(argv[m]);                       // load the data
        RandomShader shader(model);
        for (int f=0; f<model.nfaces(); f++) {      // iterate through all facets
            shader.color = { std::rand()%255, std::rand()%255, std::rand()%255, 255 };
            Triangle clip = { shader.vertex(f, 0),  // assemble the primitive
                              shader.vertex(f, 1),
                              shader.vertex(f, 2) };
            rasterize(clip, shader, framebuffer);   // rasterize the primitive
        }
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
```

In fact, our `main()` function follows closely the rendering pipeline, let us check it:

> 事实上，我们的 `main()` 函数已经紧紧跟随渲染管线的结构，我们来检查一下：

- We declare few global constants: screen dimensions, camera position etc (lines 34-38).
- Then all the state variables are initialized (lines 40-44)
- and, finally, we loop over all triangles rasterizing them (lines 49-55).

> - 我们声明了几个全局常量：屏幕尺寸、相机位置等（第 34-38 行）。
> - 然后初始化所有状态变量（第 40-44 行）。
> - 最后，我们遍历所有三角形并把它们逐个光栅化（第 49-55 行）。

Let's focus on the last part. The primitive assembly (lines 51-53) calls the vertex shader to position vertices in the space.
Then the primitive is passed to the rasterizer (line 54), who calls the fragment shader to decide color of individual fragments.

> 让我们把注意力放在最后一部分。图元装配（第 51-53 行）会调用顶点着色器，把顶点定位到空间中的正确位置；随后图元被交给光栅化器（第 54 行），由它调用片段着色器来决定各个片元的颜色。

Here, the fragment shader is quite rudimentary
and the final result is the same as in the previous chapter (after all, we did nothing but refactor the code!):

> 这里片段着色器还非常简陋，最终结果和上一章完全相同（毕竟我们除了重构代码什么都没做！）：

![Image 4](https://haqr.eu/tinyrenderer/shading/random.png)

## Phong reflection model（Phong 反射模型）

Now, let's talk about actual shading, i.e. lights and shadows. After all, programmable stages of the rendering pipeline are called shaders because approximation of the illumination is their main purpose.
For the moment I ignore colors, let us say that the scene is painted gray, but I am very interested in illumination intensity.
I want you to produce such an image by the end of this chapter:

> 现在，我们来谈谈真正的着色，也就是光照和阴影。毕竟渲染管线中可编程的阶段之所以叫"着色器"（shader），就是因为它们的主要用途是近似计算光照。这里我先不管颜色——就假定场景被涂成灰色——但我非常关心光照强度。我希望你在本章结束时能渲染出这样一幅图像：

![Image 5](https://haqr.eu/tinyrenderer/shading/phong-full.png)

In computer graphics, we often "cheat" for efficiency: computing global illumination is very expensive (we will see it a bit later), and we want to do cheap tricks that achieve the most visually plausible effects.
The Phong reflection model is one such trick, a simple but effective way to approximate how light interacts with a surface in computer graphics.
It assumes that the perceived brightness at a point on a surface is the sum of three components: __ambient__, __diffuse__, and __specular__ reflection, each scaled by a weighting factor to control its influence.

> 在计算机图形学中，为了效率我们经常"作弊"：计算全局光照的代价非常高（稍后我们就会看到），而我们希望用一些廉价的技巧达到视觉上尽可能可信的效果。Phong 反射模型就是这样一种技巧，它简单而有效，用来近似计算机图形学中光与表面的相互作用。它假定表面上某一点的可感知亮度由三个分量相加而成：**环境光**、**漫反射**和**镜面反射**，每个分量再乘上一个权重系数以控制其影响。

- The __ambient term__ models indirect light — the constant background illumination present even in shadowed areas.
- The __diffuse term__ represents light scattered equally in all directions from rough surfaces.
- The __specular term__ simulates bright highlights on shiny surfaces caused by mirror-like reflection.

> - **环境光项**模拟间接光照——即使在阴影区域也依然存在的那份恒定背景照明。
> - **漫反射项**表示粗糙表面向各个方向均匀散射的光。
> - **镜面反射项**模拟光滑表面上由类镜面反射造成的明亮高光。

If we store each term in its own image, we can obtain the final result as a simple (weighted) sum:

> 如果我们将每一项分别存成一张图像，最终结果就是对它们做一个简单的（加权）求和：

![Image 6](https://haqr.eu/tinyrenderer/shading/phong-reflectance.png)

By adjusting these weights, artists can control how "matte" or "shiny" a surface appears.

> 通过调整这些权重，美术人员就能控制表面看起来有多"哑光"或多"闪亮"。

### The practical recipe（实用配方）

Here's a nuts-and-bolts walkthrough of computing Phong lighting on a triangulated surface.
For the moment we consider surfaces made of flat triangles, so the illumination is constant per triangle.

> 下面具体地走一遍在三角形网格表面上计算 Phong 光照的过程。目前我们只考虑由平面三角形构成的表面，因此光照在每个三角形上是常量。

As mentioned earlier, the ambient term is a constant fill light, it doesn't depend on geometry.
Let's focus on the diffuse (Lambert) term.
It models matte scattering, i.e. received energy depends on how much the surface faces the light.

> 如前所述，环境光项是一份恒定的补光，与几何形状无关。我们重点来看漫反射（Lambert）项：它模拟的是哑光散射，也就是说，表面接收到的能量取决于它正对光源的程度。

Imagine a solar panel. The sun is very distant, so all the light rays can be considered parallel.
See this image:

> 想象一块太阳能板。太阳非常遥远，因此所有光线都可以看作平行光。请看下图：

![Image 7](https://haqr.eu/tinyrenderer/shading/diffuse.svg)

The energy output of the panel depends on how many sun rays enter in the corridor between the dashed lines.
For a 1D unit length panel, the width of the corridor can be computed as \(\cos \alpha\): it is 1 meter when the panel is orthogonal to the sun rays and it is zero when the panel is aligned with the sun rays.

> 太阳能板的输出能量取决于有多少条阳光进入两条虚线之间的走廊。对于一块一维、长度为 1 的板子，这条走廊的宽度可以按 \(\cos \alpha\) 计算：当板面与阳光垂直时宽度为 1 米，当板面与阳光平行时宽度为 0。

Here's a quick question for you: cosine function gives values between -1 and 1. What do we do with negative length?

> 给你出个小问题：余弦函数的取值在 -1 到 1 之间，那遇到"负的长度"该怎么办？

Spoiler Alert!

> 剧透警告！

Imagine a solar panel perfectly orthogonal to the sun rays, but with solar cells facing the ground and not the sky. In this case it won't produce any energy at all, right?
And it is exactly the case of negative \(\cos \alpha\). So the diffuse term is to be computed as \(|\cos \alpha|\) for two-sided facets or \(\max (0, \cos \alpha)\) for one-sided facets.

> 想象一块完全垂直于阳光的太阳能板，但它的电池面朝地面而不是天空。这种情况下它根本不会产生任何能量，对吧？而这个情形正是 \(\cos \alpha\) 为负的情况。所以漫反射项的计算方式是：双面面片取 \(|\cos \alpha|\)，单面面片取 \(\max (0, \cos \alpha)\)。

One question remains for the diffuse lighting: how to compute \(\cos \alpha\)?
Imagine we have a 3D triangle described by three points \(A\), \(B\) and \(C\).
Then its normal \(\vec n\) (the unit length vector orthogonal to the triangle) can be computed as the cross product between the edges:

> 漫反射还有一个问题没有解决：\(\cos \alpha\) 怎么算？假设有一个由三个点 \(A\)、\(B\)、\(C\) 描述的 3D 三角形，那么它的法线 \(\vec n\)（垂直于该三角形、长度为 1 的向量）可以用两条边的叉积来计算：

\[
\vec n=\frac{\overrightarrow{AB} \times \overrightarrow{AC} }{||\overrightarrow{AB} \times \overrightarrow{AC}||}.
\]

If \(\vec l\) is the (unit length) direction towards the sun, then \(\cos \alpha = \vec n \cdot \vec l\) and the diffuse term can be found as \(\max(0, \vec n \cdot \vec l).\)
Here is an illustration:

> 如果 \(\vec l\) 是指向太阳的（单位长度）方向，那么 \(\cos \alpha = \vec n \cdot \vec l\)，漫反射项即可由 \(\max(0, \vec n \cdot \vec l)\) 求出。图示如下：

![Image 8](https://haqr.eu/tinyrenderer/shading/vectors1.svg)

The last step is to find out how to compute the specular term.
Recall: this term simulates bright highlights on shiny surfaces caused by mirror-like reflection.
Its intensity depends on the angle between the viewer's direction and the reflection direction of the light.
Here is an illustration:

> 最后一步是研究镜面反射项该怎么算。回忆一下：这一项模拟光滑表面上由类镜面反射产生的明亮高光，其强度取决于观察者方向与光线反射方向之间的夹角。图示如下：

![Image 9](https://haqr.eu/tinyrenderer/shading/vectors2.svg)

Here are the vector \(\vec v\) is the direction from the fragment to the camera, and the \(\vec r\) is the reflection of the sun light.
Quick exercise: given vectors \(\vec n\) and \(\vec l\), find vector \(\vec r\).

> 这里向量 \(\vec v\) 是从片元指向相机的方向，\(\vec r\) 是太阳光的反射方向。小练习：已知向量 \(\vec n\) 和 \(\vec l\)，求向量 \(\vec r\)。

Spoiler Alert!

> 剧透警告！

If \(\vec n\) and \(\vec l\) are normalized, then \(\vec r = 2\vec n (\vec n \cdot \vec l ) - \vec l\).

> 如果 \(\vec n\) 和 \(\vec l\) 都已经归一化，那么 \(\vec r = 2\vec n (\vec n \cdot \vec l ) - \vec l\)。

A perfect mirror appears brightest if the viewer direction \(\vec v\) exactly matched \(\vec r\).
For non-perfectly-polished materials, the highlight should fade away as \(\vec v\) moves away from \(\vec r\).

> 对于完美镜面，当观察方向 \(\vec v\) 与 \(\vec r\) 完全重合时看起来最亮。而对于并非完美抛光的材质，随着 \(\vec v\) 偏离 \(\vec r\)，高光应当逐渐衰减。

The dot product \(\vec r \cdot \vec v\) is \(\cos \beta\), where \(\beta\) is the angle between the viewing direction and the perfect reflection direction.
To compute the specular term, we can simply raise this cosine to a shininess exponent \(e\), i.e. the specular term can be found as \((\max(0, \cos \beta))^e\).
Recall that all numbers inferior to 1 will decrease when we raise them into power.
Let's plot \(\cos^e \beta\) for different values of \(e\):

> 点积 \(\vec r \cdot \vec v\) 就是 \(\cos \beta\)，其中 \(\beta\) 是观察方向与完美反射方向之间的夹角。要计算镜面反射项，只需把这个余弦值取"光泽度"指数 \(e\) 次幂，也就是说镜面反射项可由 \((\max(0, \cos \beta))^e\) 求得。回想一下，所有小于 1 的数在乘方之后都会变小。下面画出不同 \(e\) 值下的 \(\cos^e \beta\) 曲线：

![Image 10](https://haqr.eu/tinyrenderer/shading/exponent.png)

This power-law falloff is a mathematical hack giving narrow, tight highlight (polished metal, glossy plastic) for large exponents \(e\).
For smaller \(e\), it corresponds to broad, soft highlight (matte or worn surfaces).

> 这种幂律衰减是一种数学上的小把戏：指数 \(e\) 较大时得到又窄又锐的高光（抛光金属、亮面塑料），\(e\) 较小时则对应又宽又柔和的高光（哑光或磨损的表面）。

## Homework assignment（课后作业）

You have all the tools you need to implement Phong reflection model, try to do so, play with the coefficients. Can you replicate my results?
Normally you need to tweak the fragment shader only.
For reference, [here is my attempt](https://github.com/ssloy/tinyrenderer/commit/27f4fd811aa3a62e478910e195abb9c6c6318f97).

> 实现 Phong 反射模型所需的工具你已经全部具备了，动手做一遍吧，把各个系数都调一调。你能复现我的结果吗？通常你只需要修改片段着色器。作为参考，[这是我的一份实现](https://github.com/ssloy/tinyrenderer/commit/27f4fd811aa3a62e478910e195abb9c6c6318f97)。

Next time we will talk about textures and normal mapping. Stay tuned!

> 下次我们聊聊纹理和法线贴图，敬请期待！

> 原文：[Lesson 7: Shading](https://haqr.eu/tinyrenderer/shading/)
