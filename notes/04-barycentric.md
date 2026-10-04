# Primer on barycentric coordinates（重心坐标入门）

## 1D barycentric coordinates（一维重心坐标）

### A bit of math（一点数学）

Now it is the time to recall the note I made on the barycentric coordinates when we rasterized segments. Just as regular coordinates give the position of a point with respect to a chosen basis, barycentric coordinates express a point's position relative to a given set of reference points. Let's first consider the simplest case: a __1D segment__ defined by two values \( A \in \mathbb R\) and \( B \in \mathbb R \). Given a point \( P \) on the segment between \( A \) and \( B \), we can express its position as a weighted combination:

\[
P = \alpha A + \beta B
\]

> 现在该回顾一下我们在光栅化线段时留下的那条关于重心坐标的笔记了。普通坐标给出的是点相对于某个选定基底的位置，而重心坐标表达的是点相对于一组给定参照点的位置。先从最简单的情形说起：用两个值 \( A \in \mathbb R\) 和 \( B \in \mathbb R \) 定义的 __1D 线段__。给定线段上位于 \( A \) 与 \( B \) 之间的点 \( P \)，我们可以把它的位置表示成加权组合：

The term "barycentric" comes from the Greek word _barys_ (meaning heavy), as these coordinates originally arose in __center of mass computations__. If we place a weight of \(\alpha\) kg to the point \(A\) and \(\beta\) kg to the point \(B\), then the barycenter of the system will be situated in the point \(P\). Note that the barycenter would not move if we put weights \(13 \alpha\) and \(13 \beta\) instead of \(\alpha\) and \(\beta\). The usual way to eliminate this redundancy is to distribute \(1\) kg in total, meaning that

\[
\alpha + \beta = 1.
\]

> “重心”（barycentric）一词源自希腊语 _barys_（意为“重”），因为这类坐标最初诞生于 __质心计算__。如果在点 \(A\) 上挂一个重 \(\alpha\) kg 的砝码、在点 \(B\) 上挂一个重 \(\beta\) kg 的砝码，那么整个系统的重心就会落在点 \(P\)。注意，如果把权重换成 \(13 \alpha\) 和 \(13 \beta\)（而不是 \(\alpha\) 和 \(\beta\)），重心并不会移动。消除这种冗余的惯常做法是让总重量为 \(1\) kg，也就是说

Under this constraint \(\alpha\) and \(\beta\) are unique for each choice of point \(P\) and the reference segment \([A, B]\). How do we find \(\alpha\) and \(\beta\) for given \(P\), \(A\) and \(B\)? Well, we have a system of two equations with two unknowns:

\[
\left\{
\begin{array}{l}
    P = \alpha A + \beta B\\
    1~ = \alpha + \beta
\end{array}\right.
\]

> 在这个约束下，对于每一组选定的点 \(P\) 和参考线段 \([A, B]\)，\(\alpha\) 和 \(\beta\) 都是唯一的。那么给定 \(P\)、\(A\) 和 \(B\)，要怎样求出 \(\alpha\) 和 \(\beta\) 呢？很简单，我们有一个两个未知数、两个方程的方程组：

Solving the system, we get following equations:

\[
\alpha = \frac{B - P}{B - A}, \quad \beta = 1-\alpha = \frac{P - A}{B - A}.
\]

> 解这个方程组，就得到如下公式：

### Example 1（示例 1）

These weights tell us how much of \( A \) and \( B \) contribute to the position of \( P \). To take an example, let us imagine a one meter-long rod. Put 750g to the left end (point \(A\)), and 250g to the right end (point \(B\)). To make the rod balance on a needle, we need to adapt the lengths of the levers. In this case, the rod is balanced if \(P\) is 25cm from the left endpoint, and therefore, 75cm from the right.

> 这两个权重告诉我们：\( A \) 和 \( B \) 各自为 \( P \) 的位置贡献了多少。举个例子，想象一根一米长的杆子：左端（点 \(A\)）挂 750g，右端（点 \(B\)）挂 250g。想让杆子在针尖上保持平衡，就得调节两侧力臂的长度。在这个例子里，当 \(P\) 距左端 25cm、也就是距右端 75cm 时，杆子刚好平衡。

![Image 1](https://haqr.eu/1d.png)

Note that we can have __negative weights__, meaning that they do not weigh down, but rather pull up. Let us sum up:

> 注意，权重也可以是__负数__，这表示它们不是往下压，而是往上拉。归纳一下：

- If \( P = A \), then \( \alpha = 1 \) and \( \beta = 0 \).
- If \( P = B \), then \( \alpha = 0 \) and \( \beta = 1 \).
- If both coordinates are __strictly between 0 and 1__, \( P \) is strictly inside the segment.
- If \( P \) is outside \( [A, B] \), then one coordinate is __negative__.

> - 若 \( P = A \)，则 \( \alpha = 1 \)、\( \beta = 0 \)。
> - 若 \( P = B \)，则 \( \alpha = 0 \)、\( \beta = 1 \)。
> - 若两个坐标都__严格介于 0 和 1 之间__，则 \( P \) 严格位于线段内部。
> - 若 \( P \) 在线段 \( [A, B] \) 之外，则其中一个坐标为__负数__。

### Example 2（示例 2）

Recall our [second attempt](../bresenham/#second-attempt-different-sampling-strategy) on rasterizing line segments? Here is the code:

> 还记得我们对线段光栅化的[第二次尝试](../bresenham/#second-attempt-different-sampling-strategy)吗？代码如下：

```cpp
void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    for (int x=ax; x<=bx; x++) {
        float t = (x-ax) / static_cast<float>(bx-ax);
        int y = std::round( (1-t) * ay + t * by );
        framebuffer.set(x, y, color);
    }
}
```

If we call `line(17, 4, 43, 59, framebuffer, white)`, we obtain the following image:

> 如果调用 `line(17, 4, 43, 59, framebuffer, white)`，会得到下面的图像：

![Image 2](https://haqr.eu/line.png)

Let us take a closer look to what is happening. We iterate (line 2) through the interval \(x\in[a_x, b_x]\). For each value \(x\) we determine (line 3) its barycentric coordinates \((1-t, t)\) with respect to the interval \([a_x, b_x]\). Finally, using these barycentric coordinates with respect to __another__ interval \([a_y, b_y]\), we compute interpolated value of \(y\). Therefore, in this approach \(y\) is a __linear interpolation__ over the segment \([a_x, b_x]\) between two samples \(a_y\) and \(b_y\).

> 我们仔细看看其中发生了什么。代码在第 2 行遍历区间 \(x\in[a_x, b_x]\)。对每个 \(x\) 值，第 3 行求出它相对于区间 \([a_x, b_x]\) 的重心坐标 \((1-t, t)\)。最后，把这组重心坐标用到__另一个__区间 \([a_y, b_y]\) 上，算出 \(y\) 的插值。因此在这种做法里，\(y\) 是在两个采样点 \(a_y\) 和 \(b_y\) 之间、沿线段 \([a_x, b_x]\) 的__线性插值__。

The ability to interpolate various things over a given domain is an extremely powerful tool, remember it, we will make a heavy use of it.

> 能在给定定义域上对各种各样的量做插值，是一种极其强大的工具，记住这一点，后面我们会大量用到它。

## 2D barycentric coordinates（二维重心坐标）

In 1D we used two points to compute their barycenter. In 2D we'd need a triangle. Given a triangle with vertices \( A, B, C \), any point \( P \) inside the triangle can be written as:

\[
P = \alpha A + \beta B + \gamma C,
\]

> 一维情形下，我们用两个点来计算它们的重心；到了二维，就需要一个三角形。给定顶点为 \( A, B, C \) 的三角形，其内部的任意点 \( P \) 都可以写成：

where the barycentric coordinates \( \alpha, \beta, \gamma \) satisfy:

\[
\alpha + \beta + \gamma = 1.
\]

> 其中重心坐标 \( \alpha, \beta, \gamma \) 满足：

We can compute the weights \(\alpha\), \(\beta\) and \(\gamma\) for given 2D points \(P\), \(A\), \(B\) and \(C\) in the same manner we did in the previous example. We have three unknowns with three equations linking them:

\[
\left\{
\begin{array}{ll}
    \alpha A_x + \beta B_x + \gamma C_x &= P_x\\
    \alpha A_y + \beta B_y + \gamma C_y &= P_y\\
    \alpha + \beta + \gamma &= 1
\end{array}\right.
\]

> 对于给定的二维点 \(P\)、\(A\)、\(B\) 和 \(C\)，我们可以用与上一个例子相同的方式计算权重 \(\alpha\)、\(\beta\) 和 \(\gamma\)。这里有三个未知数，由三个方程把它们联系起来：

How can we solve the system?

> 这个方程组该怎么解呢？

### A bit of matrix calculus (optional)（一点矩阵运算，选读）

For a better readability, we can rewrite the same equation in the matrix form:

\[
\begin{pmatrix}
    A_x & B_x & C_x\\
    A_y & B_y & C_y\\
    1 & 1 & 1
\end{pmatrix}
\begin{pmatrix}\alpha \\ \beta \\ \gamma\end{pmatrix}
=
\begin{pmatrix}P_x \\ P_y \\ 1\end{pmatrix}
\]

> 为了更易读，我们可以把同一个方程改写成矩阵形式：

Then we have:

\[
\begin{align*}
\begin{pmatrix}\alpha \\ \beta \\ \gamma\end{pmatrix}
&=
\begin{pmatrix}
    A_x & B_x & C_x\\
    A_y & B_y & C_y\\
    1 & 1 & 1
\end{pmatrix}^{-1}
\begin{pmatrix}P_x \\ P_y \\ 1\end{pmatrix}\\
&=
\frac{
\small
\text{adj}
\begin{pmatrix}
    A_x & B_x & C_x\\
    A_y & B_y & C_y\\
    1 & 1 & 1
\end{pmatrix}
\begin{pmatrix}P_x \\ P_y \\ 1\end{pmatrix}
}{\small
\begin{vmatrix}
    A_x & B_x & C_x\\
    A_y & B_y & C_y\\
    1 & 1 & 1
\end{vmatrix}
}
\\
&=
\frac{\small
\begin{pmatrix}
B_y - C_y & C_x - B_x &  B_x C_y - B_y C_x\\
C_y - A_y & A_x - C_x &  A_y C_x - A_x C_y\\
A_y - B_y & B_x - A_x &  A_x B_y - A_y B_x
\end{pmatrix}
\begin{pmatrix}P_x \\ P_y \\ 1\end{pmatrix}
}{\small
\begin{vmatrix}
    A_x & B_x & C_x\\
    A_y & B_y & C_y\\
    1 & 1 & 1
\end{vmatrix}
}\\
&=
\frac{\small
\begin{pmatrix}
\begin{vmatrix}
    P_x & B_x & C_x\\
    P_y & B_y & C_y\\
    1 & 1 & 1
\end{vmatrix}
&
\begin{vmatrix}
    P_x & C_x & A_x\\
    P_y & C_y & A_y\\
    1 & 1 & 1
\end{vmatrix}
&
\begin{vmatrix}
    P_x & A_x & B_x\\
    P_y & A_y & B_y\\
    1 & 1 & 1
\end{vmatrix}
\end{pmatrix}^\top
}{\small
\begin{vmatrix}
    A_x & B_x & C_x\\
    A_y & B_y & C_y\\
    1 & 1 & 1
\end{vmatrix}
}
\end{align*}
\]

> 于是有：

It turns out that to find the weights \(\alpha, \beta, \gamma\), we need to compute determinants of four matrices that of a very particular form. These determinants give twice the __signed area__ of the triangle formed by the points in the matrices.

> 可以发现，为了求出权重 \(\alpha, \beta, \gamma\)，我们需要计算四个矩阵的行列式，而这些矩阵的形式非常特殊。这些行列式的值，等于由矩阵中各点构成的三角形__有符号面积__的两倍。

### Practical computation（实际计算）

Then the coordinate \( \alpha \) is given by the ratio of the sub-triangle \( PBC \) to the total triangle \( ABC \):

\[
\alpha = \frac{ \, \text{Area}(PBC) \, }{ \, \text{Area}(ABC) \, }.
\]

> 于是，坐标 \( \alpha \) 由于三角形 \( PBC \) 与整个三角形 \( ABC \) 的面积之比给出：

Similarly,

\[
\beta = \frac{ \, \text{Area}(PCA) \, }{ \, \text{Area}(ABC) \, }, \quad \gamma = \frac{ \, \text{Area}(PAB) \, }{ \, \text{Area}(ABC) \, }.
\]

> 同理，

The [shoelace formula](https://en.wikipedia.org/wiki/Shoelace_formula) allows to efficiently compute these areas:

\[
\text{Area}(ABC) = \frac{1}{2} \left( \small(B_y-A_y)(B_x+A_x) + (C_y-B_y)(C_x+B_x) + (A_y-C_y)(A_x+C_x) \right).
\]

> [鞋带公式](https://en.wikipedia.org/wiki/Shoelace_formula)可以高效地计算这些面积：

In fact, it is just the above determinants written explicitly. Please, do pay attention that the order of point matters, as the formula gives the __signed__ area.

> 其实，它不过就是把上面的行列式展开写了出来。请注意，点的顺序是有影响的，因为该公式给出的是__有符号__面积。

The interpretation of the weights is very similar to the 1D case:

> 这些权重的含义与一维情形非常相似：

- If \( \alpha, \beta, \gamma \) are all __between 0 and 1__, \( P \) is __inside__ the triangle.
- If any coordinate is __negative__, \( P \) is __outside__ the triangle.
- If one coordinate is exactly __zero__, \( P \) lies __on an edge__.
- If two coordinates are __zero__, \( P \) is exactly on a vertex.

> - 若 \( \alpha, \beta, \gamma \) 都__介于 0 和 1 之间__，则 \( P \) 在三角形__内部__。
> - 若任一坐标为__负数__，则 \( P \) 在三角形__外部__。
> - 若某一个坐标恰为__零__，则 \( P \) __落在一条边上__。
> - 若两个坐标都为__零__，则 \( P \) 恰好位于一个顶点上。

### Example 1（示例 1）

Let me illustrate all this with the following example:

> 下面用一个例子来说明这一切：

![Image 3](https://haqr.eu/2d_b.png)

Here I took a [Pythagorean triangle](https://en.wikipedia.org/wiki/Pythagorean_triple) with sides 12:16:20. I chose points \(A, B, C\) to have coordinates \((16, 0), (0, 12), (0,0)\), respectively. Then I took point \(P = (4,3)\).

\[
\text{Area}(ABC)=
\frac12
\begin{vmatrix}
16 & 0  &0\\
0  & 12 &0\\
1  & 1  &1
\end{vmatrix} = 96,
\quad
\text{Area}(PBC)=
\frac12
\begin{vmatrix}
 4 & 0  &0\\
 3 & 12 &0\\
 1 & 1  &1
\end{vmatrix} = 24
\]
\[
\text{Area}(PCA)=
\frac12
\begin{vmatrix}
 4 & 0 & 16\\
 3 & 0 & 0 \\
 1 & 1 & 1
\end{vmatrix} = 24,
\quad
\text{Area}(PAB)=
\frac12
\begin{vmatrix}
 4 & 16 & 0\\
 3 & 0  & 12 \\
 1 & 1  &1
\end{vmatrix} = 48
\]

> 这里我取了一个三边之比为 12:16:20 的[勾股三角形](https://en.wikipedia.org/wiki/Pythagorean_triple)，令点 \(A, B, C\) 的坐标分别为 \((16, 0), (0, 12), (0,0)\)，然后取点 \(P = (4,3)\)。

By dividing these areas, we obtain the barycentric coordinates:

$$
\alpha=\frac14,\quad \beta=\frac14,\quad\gamma=\frac12
$$

> 把这些面积相除，就得到重心坐标：

Note that we can compute \(\text{Area}(PCA)\) more easily. We know that the area of the triangle is half of the base (16/2) multiplied by its height (3). Therefore, \(\text{Area}(PCA)\) is constant for every position of the point \(P\) that does not affect the height. The green dashed line indicates possible positions of \(P\) for \(\beta=\frac14\). The isoline \(\alpha=\frac14\) is red and \(\gamma=\frac12\) is blue. Naturally, they meet in the only possible point \(\alpha+\beta+\gamma = 1\).

> 注意，\(\text{Area}(PCA)\) 还可以算得更简单：我们知道三角形面积等于底的一半（16/2）乘以它的高（3）。因此，只要点 \(P\) 的位置不影响这条高，\(\text{Area}(PCA)\) 就是常数。绿色虚线标出了 \(\beta=\frac14\) 时 \(P\) 所有可能的位置。\(\alpha=\frac14\) 的等值线是红色的，\(\gamma=\frac12\) 的等值线是蓝色的。很自然，它们在唯一满足 \(\alpha+\beta+\gamma = 1\) 的那个点相遇。

Practice makes perfect, I am not afraid of repetition: the point \(P\) is the barycenter of triangle \(ABC\) with weights \(\alpha, \beta, \gamma\) appied to the vertices:

> 熟能生巧，我不怕重复：点 \(P\) 就是把权重 \(\alpha, \beta, \gamma\) 施加到三角形 \(ABC\) 各顶点后整个系统的重心：

![Image 4](https://haqr.eu/2d.png)

### Example 2（示例 2）

As I have already mentioned, barycentric coordinates are fundamental in rasterization because they allow efficient interpolation of attributes such as colors, depth values, and texture coordinates across triangles in 3D rendering. Let me modify a little bit the triangle rasterization function we made in the [previous lesson](../rasterization/).

> 正如前面提到的，重心坐标在光栅化中是基础性的，因为它们让我们能在 3D 渲染中沿着三角形高效地插值颜色、深度值、纹理坐标等属性。下面把我们[上一课](../rasterization/)写的三角形光栅化函数稍微改一下。

Linear interpolation over a triangle

```cpp
#include <cmath>
#include "tgaimage.h"

double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5*((by-ay)*(bx+ax) + (cy-by)*(cx+bx) + (ay-cy)*(ax+cx));
}

void triangle(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage &framebuffer) {
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
            framebuffer.set(x, y, {z});
        }
    }
}

int main(int argc, char** argv) {
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::GRAYSCALE);

    int ax = 17, ay =  4, az =  13;
    int bx = 55, by = 39, bz = 128;
    int cx = 23, cy = 59, cz = 255;

    triangle(ax, ay, az, bx, by, bz, cx, cy, cz, framebuffer);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
```

In addition to the triangle vertices, I pass to the function three integers `az`, `bz`, `cz`, and I interpret them as colors of the vertices in a grayscale rasterization. Then for each pixel with coordinates `alpha`,`beta`,`gamma`, I interpolate its color as the weighed sum `z = alpha * az + beta * bz + gamma * cz`, obtaining the following image:

> 除了三角形的顶点之外，我还给函数传入了三个整数 `az`、`bz`、`cz`，在灰度光栅化中把它们当作顶点的颜色。然后对每个像素，按它的坐标 `alpha`,`beta`,`gamma` 把颜色插值成加权和 `z = alpha * az + beta * bz + gamma * cz`，得到下面的图像：

![Image 5](https://haqr.eu/triangle.png)

## Homework assignment（作业）

Play around with the code, for example, draw full-colored triangles instead of grayscale:

> 动手玩玩这段代码吧，比如画彩色三角形代替灰度三角形：

![Image 6](https://haqr.eu/rgb.png)

Or draw a "wireframe" triangle without using `line(...)` function:

> 或者不使用 `line(...)` 函数，画一个“线框”三角形：

![Image 7](https://haqr.eu/fatwire.png)

> 原文：[Lesson 3: Primer on barycentric coordinates](https://haqr.eu/tinyrenderer/barycentric/)
