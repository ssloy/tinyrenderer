# A crude (but simple) approach to camera handling（粗暴但简单的相机处理方式）

## Rotating the camera（旋转相机）

At this stage, our renderer has no explicit concept of a camera:
depth (the \(z\)-coordinate) does not influence the size or shape of objects, and is largely ignored.
We do use the \(z\)-coordinate in the \(z\)-buffer to hide occluded surfaces, but this is a minor detail.
Ignoring depth leads to orthographic projection, where objects retain their dimensions regardless of distance from the viewer.
As before, our renderer outputs two images: `zbuffer.tga` and `framebuffer.tga`.

> 现阶段，我们的渲染器还没有显式的相机概念：深度（即 \(z\) 坐标）不影响物体的尺寸和形状，基本上被忽略。我们确实会利用 \(z\) 缓冲中的 \(z\) 坐标来隐藏被遮挡的表面，但这只是个次要细节。忽略深度得到的是正交投影：物体无论离观察者多远都保持同样的尺寸。和之前一样，渲染器输出两张图像：`zbuffer.tga` 和 `framebuffer.tga`。

![Image 1](https://haqr.eu/head-front.png)

To simulate a camera, we transform the scene rather than moving the camera itself.
For example, instead of rotating the camera to the left, we rotate the scene to the right.
This keeps the camera fixed conceptually, and shifts the viewpoint via model-view transformations.
This method simplifies camera handling and offers intuitive control over the visible scene.

> 为了模拟相机，我们变换的是场景，而不是移动相机本身。比如，与其把相机向左旋转，不如把场景向右旋转。这样相机在概念上保持不动，视点的移动则通过模型-视图变换来实现。这种方法简化了相机的处理，也让我们能直观地控制可见场景。

See [this commit](https://github.com/ssloy/tinyrenderer/commit/57760190f9f19005afe7031967c4e21dab3b9fd8):

> 参见[这个提交](https://github.com/ssloy/tinyrenderer/commit/57760190f9f19005afe7031967c4e21dab3b9fd8)：

Rotating the Object

```cpp
vec3 rot(vec3 v) {
    constexpr double a = M_PI/6;
    constexpr mat<3,3> Ry = {{{std::cos(a), 0, std::sin(a)}, {0,1,0}, {-std::sin(a), 0, std::cos(a)}}};
    return Ry*v;
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
        auto [ax, ay, az] = project(rot(model.vert(i, 0)));
        auto [bx, by, bz] = project(rot(model.vert(i, 1)));
        auto [cx, cy, cz] = project(rot(model.vert(i, 2)));
        TGAColor rnd;
        for (int c=0; c<3; c++) rnd[c] = std::rand()%255;
        triangle(ax, ay, az, bx, by, bz, cx, cy, cz, zbuffer, framebuffer, rnd);
    }

    framebuffer.write_tga_file("framebuffer.tga");
    zbuffer.write_tga_file("zbuffer.tga");
    return 0;
}
```

In this example, I apply the function `vec3 rot(vec3 v)` (lines 1–5) to each model vertex (lines 24–26) before projection.
This rotates each vertex by \(30^\circ\) around the \(y\)-axis to the right, giving the impression of rotating the camera to the left:

> 在这个例子里，我在投影之前对每个模型顶点（第 24–26 行）应用了函数 `vec3 rot(vec3 v)`（第 1–5 行）。它让每个顶点绕 \(y\) 轴向右旋转 \(30^\circ\)，给人的印象就是相机在向左转：

![Image 2](https://haqr.eu/head-rot.png)

Here, a constant [rotation matrix](https://en.wikipedia.org/wiki/Rotation_matrix#In_three_dimensions) is used.
Thanks to the previous homework, we now have basic vector and matrix operations available.
If you're not familiar with rotation matrices, don't worry - we'll soon explore alternative model-view transformations.
However, be sure to review basic vector math, as it will be essential.

> 这里用的是一个常量[旋转矩阵](https://en.wikipedia.org/wiki/Rotation_matrix#In_three_dimensions)。多亏了上一次作业，我们现在已经有基本的向量和矩阵运算可用。如果你不熟悉旋转矩阵，不用担心——我们很快就会探索其他形式的模型-视图变换。不过，请务必复习基本的向量数学，它会非常关键。

## Central projection（中心投影）

Orthographic projection is useful, but central (or perspective) projection offers more realism:
closer objects appear larger than distant ones.

> 正交投影很实用，但中心投影（或称透视投影）更真实：近处的物体看起来比远处的大。

Consider projecting a point \(P = (x, y, z)\) onto the plane \(z=0\) using a camera located at \(C = (0, 0, c)\) on the \(z\)-axis:

> 考虑把点 \(P = (x, y, z)\) 投影到平面 \(z=0\) 上，相机位于 \(z\) 轴上的 \(C = (0, 0, c)\) 处：

![Image 3](https://haqr.eu/persp-a.svg)

The projection point \(P'\) lies at the intersection of line \(CP\) and the screen plane \(z=0\).
Given \(P = (x, y, z)\) and camera parameter \(c\), we want to find \(P' = (x', y', 0)\).

> 投影点 \(P'\) 是直线 \(CP\) 与屏幕平面 \(z=0\) 的交点。已知 \(P = (x, y, z)\) 和相机参数 \(c\)，我们想求出 \(P' = (x', y', 0)\)。

Let’s first compute \(x'\) using the plane \(y = 0\):

> 先借助平面 \(y = 0\) 来计算 \(x'\)：

![Image 4](https://haqr.eu/persp-c.svg)

Using the [intercept theorem](https://en.wikipedia.org/wiki/Intercept_theorem):

> 根据[截线定理](https://en.wikipedia.org/wiki/Intercept_theorem)：

\[
\frac{x'}{c} = \frac{x}{c - z} \quad \Rightarrow \quad x' = \frac{x}{1 - \frac{z}{c}}.
\]

Similarly, in the plane \(x = 0\):

> 同理，在平面 \(x = 0\) 中：

![Image 5](https://haqr.eu/persp-b.svg)

\[
\frac{y'}{c} = \frac{y}{c - z} \quad \Rightarrow \quad y' = \frac{y}{1 - \frac{z}{c}}.
\]

As expected, both expressions are structurally similar.
So, just like rotation, we implement perspective projection by transforming vertices:
we replace each vertex \((x, y, z)\) with \((x, y, z) \cdot \frac{1}{1 - \frac{z}{c}}\).
We then apply orthographic projection as usual.
See lines 7–10 and 29–31:

> 不出所料，两个表达式在结构上是相似的。所以，和旋转一样，我们通过变换顶点来实现透视投影：把每个顶点 \((x, y, z)\) 替换为 \((x, y, z) \cdot \frac{1}{1 - \frac{z}{c}}\)。然后照常应用正交投影。参见第 7–10 行和第 29–31 行：

Central Projection

```cpp
vec3 rot(vec3 v) {
    constexpr double a = M_PI/6;
    constexpr mat<3,3> Ry = {{{std::cos(a), 0, std::sin(a)}, {0,1,0}, {-std::sin(a), 0, std::cos(a)}}};
    return Ry*v;
}

vec3 persp(vec3 v) {
    constexpr double c = 3.;
    return v / (1-v.z/c);
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
        auto [ax, ay, az] = project(persp(rot(model.vert(i, 0))));
        auto [bx, by, bz] = project(persp(rot(model.vert(i, 1))));
        auto [cx, cy, cz] = project(persp(rot(model.vert(i, 2))));
        TGAColor rnd;
        for (int c=0; c<3; c++) rnd[c] = std::rand()%255;
        triangle(ax, ay, az, bx, by, bz, cx, cy, cz, zbuffer, framebuffer, rnd);
    }

    framebuffer.write_tga_file("framebuffer.tga");
    zbuffer.write_tga_file("zbuffer.tga");
    return 0;
}
```

Here is the resulting image ([see commit](https://github.com/ssloy/tinyrenderer/commit/a34b13b6e19408e930c579641cb99da78309007d)):

> 这是渲染出的图像（[见这个提交](https://github.com/ssloy/tinyrenderer/commit/a34b13b6e19408e930c579641cb99da78309007d)）：

![Image 6](https://haqr.eu/head-persp.png)

Personally I find it much more convincing than the previous one made with an orthographic projection.

> 就我个人而言，它比之前那张用正交投影渲染的图像有说服力得多。

If this is unclear, consider the following 2D example.
Suppose we have a polygon with vertices:
\((2,0)\), \((0,2)\), \((-2, 2)\), \((-2,-2)\), and \((2,-2)\):

> 如果还不清楚，看看下面这个 2D 例子。假设有一个多边形，其顶点为：\((2,0)\)、\((0,2)\)、\((-2, 2)\)、\((-2,-2)\) 和 \((2,-2)\)：

![Image 7](https://haqr.eu/persp2-a.svg)

The camera is at \(C = (10, 0)\).
We want to project all vertices onto the green line.
Each vertex \((x, y)\) is transformed to:

> 相机位于 \(C = (10, 0)\)。我们想把所有顶点投影到绿线上。每个顶点 \((x, y)\) 都被变换为：

\[
\frac{(x, y)}{1 - \frac{x}{10}}.
\]

New coordinates:

> 新的坐标：

\[
\begin{array}{l}
\frac{(2, 0)}{1 - \frac{2}{10}} = \left(\frac{5}{2}, 0\right) \\
\frac{(0, 2)}{1 - 0} = (0, 2) \\
\frac{(-2, 2)}{1 + \frac{2}{10}} = \left(-\frac{5}{3}, \frac{5}{3}\right) \\
\frac{(-2, -2)}{1 + \frac{2}{10}} = \left(-\frac{5}{3}, -\frac{5}{3}\right) \\
\frac{(2, -2)}{1 - \frac{2}{10}} = \left(\frac{5}{2}, -\frac{5}{2}\right)
\end{array}
\]

Here is the deformed object:

> 这是变形后的物体：

![Image 8](https://haqr.eu/persp2-b.svg)

We can now apply an orthographic projection to these transformed points to get the desired central projection.

> 现在，对这些变换后的点应用正交投影，就能得到我们想要的中心投影。

## Homework assignment: find the bug（作业：找出 bug）

Everything works well - except with the Diablo model. The hand closest to the viewer is somewhat clipped:

> 一切都很顺利——除了 Diablo 模型。离观察者最近的那只手被裁掉了一部分：

![Image 9](https://haqr.eu/diablo-bug.png)

Your task is to understand why, and how to fix it.

> 你的任务是弄清楚为什么会这样，以及如何修复它。

**Spoiler Alert!（剧透警告！）**

The \(z\)-buffer is stored as an 8-bit grayscale image.

> \(z\) 缓冲是以 8 位灰度图像的形式存储的。

**Spoiler Alert #2!（剧透警告 #2！）**

The model was assumed to fit within the cube \([-1, 1]^3\), but this doesn't hold after rotation.

> 代码假定模型能装进立方体 \([-1, 1]^3\) 之内，但旋转之后这个假设就不再成立了。

**Spoiler Alert #3!（剧透警告 #3！）**

The hand's depth exceeds 1, resulting in values >255 after viewport transformation, causing integer overflow.
While storing the \(z\)-buffer as an image is helpful for debugging, it’s better to use a 2D array of floating-point values.
To visualize it, write a few lines of code to convert the depth array into an image.

> 这只手的深度超过了 1，经过视口变换后数值大于 255，导致整数溢出。虽然把 \(z\) 缓冲存成图像对调试很有帮助，但更好的做法是用二维浮点数组来存储。要把它可视化，写几行代码把深度数组转换成图像即可。

> 原文：[Lesson 5: A crude approach to camera handling](https://haqr.eu/tinyrenderer/camera-naive/)
