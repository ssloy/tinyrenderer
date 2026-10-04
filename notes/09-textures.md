# More data!（更多数据！）

Let us continue our discussion about shading. How can we add more realism to our renders?
At this stage, I don’t want to introduce additional mathematics — instead, I want us to stop ignoring the data that is already available to us. We will continue to use the Phong reflection model that we [implemented earlier](./).

> 我们接着讨论着色。怎样才能让渲染画面更真实？
> 现阶段我不想引入额外的数学——我只想让大家别再无视手头已有的数据。我们会继续使用之前已经[实现过](./)的 Phong 反射模型。

Our goal today is to gradually improve the realism of our renderings without making major changes to the shader itself:

> 今天的目标是：在不对着色器本身做大改动的前提下，一步步提升渲染结果的真实感：

![Image 1](https://haqr.eu/teaser.jpg)

## Smooth shading（平滑着色）

### Curved geometry（曲面几何）

In the Phong reflection model, if the light direction and camera are fixed, the estimated light intensity depends only on the normal vector at the surface. In our previous implementation, the surface was composed of flat triangles, meaning each triangle had a single constant normal vector. The result was a very faceted look.

> 在 Phong 反射模型中，如果光照方向和相机固定，那么估算出的光强就只取决于表面处的法线向量。在之前的实现里，表面由一块块平面三角形拼成，也就是说每个三角形只有一个恒定不变的法线向量。结果就是画面呈现出非常明显的棱面感。

So, how can we achieve smoother renderings? The answer: provide more data.

> 那么，怎样才能得到更平滑的渲染效果？答案是：提供更多数据。

One naïve approach would be to define an extremely fine mesh, so fine that each triangle is smaller than a pixel on screen. While this would produce smooth results, it is computationally very expensive. A cheaper, smarter method is preferable.

> 一种天真的做法是定义极其精细的网格，精细到每个三角形在屏幕上都比一个像素还小。这样做确实能得到平滑的结果，但计算开销高昂得吓人。我们最好找一种更廉价、更聪明的办法。

Do you recall the exercise on [drawing straight line segments](../bresenham/)? Given two points \(P_0\) and \(P_1\), we can interpolate between them:

> 还记得[画直线段](../bresenham/)那次的练习吗？给定两个点 \(P_0\) 和 \(P_1\)，我们可以在它们之间做插值：

\[
P(t)= (1-t)\ P_0 + t\ P_1
\]

We vary the parameter \(t\) between 0 and 1, thus sweeping the segment.
Here is a small Python implementation for those who want to experiment again:

> 让参数 \(t\) 在 0 和 1 之间变化，就扫出了整条线段。
> 下面是一小段 Python 实现，给想再动手试试的朋友：

straight.py

```python
import numpy as np, matplotlib.pyplot as plt

P0, P1 = np.array([0,0]), np.array([3,1])
t = np.linspace(0,1,100)[:,None]
P = (1-t)*P0 + t*P1

plt.plot(*P.T)
plt.show()
```

Expected output:

> 预期输出：

![Image 2](https://haqr.eu/straight.png)

This process is simply [linear interpolation](https://en.wikipedia.org/wiki/Linear_interpolation) between \(P_0\) and \(P_1\). “Linear” means that \(P(t)\) is a polynomial of order 1.

> 这个过程不过是在 \(P_0\) 和 \(P_1\) 之间做[线性插值](https://en.wikipedia.org/wiki/Linear_interpolation)。“Linear”（线性）的意思是 \(P(t)\) 是一个 1 阶多项式。

But what if we use a higher-order polynomial?
For example, we can redefine \(P(t)\) as follows:

> 那要是换成更高阶的多项式呢？
> 比如，我们可以把 \(P(t)\) 重新定义成这样：

\[
P(t)= (1-t)^3\ P_0 + 3(1-t)^2 t\ P_1 + 3(1-t) t^2\ P_2 + t^3\ P_3
\]

Here we have four control points \(P_0, P_1, P_2, P_3\). Now \(P(t)\) is a cubic function of \(t\). As \(t\) varies from 0 to 1, the point sweeps out a curve, not a straight line. This is the principle of [Bézier curves](https://en.wikipedia.org/wiki/B%C3%A9zier_curve), widely used in [computer-aided design](https://en.wikipedia.org/wiki/Computer-aided_design).

> 这里有四个控制点 \(P_0, P_1, P_2, P_3\)。此时 \(P(t)\) 是 \(t\) 的三次函数。当 \(t\) 从 0 变到 1 时，点扫出的是一条曲线，而不是直线。这正是[贝塞尔曲线](https://en.wikipedia.org/wiki/B%C3%A9zier_curve)的原理，它在[计算机辅助设计](https://en.wikipedia.org/wiki/Computer-aided_design)中被广泛应用。

Python implementation and visualization:

> Python 实现与可视化：

bezier.py

```python
import numpy as np, matplotlib.pyplot as plt

# Bézier control points
P0, P1, P2, P3 = np.array([0,0]), np.array([1,1.5]), np.array([2,2]), np.array([3,1])

# Bézier curve
t = np.linspace(0,1,100)[:,None]
P = (1-t)**3*P0 + 3*(1-t)**2*t*P1 + 3*(1-t)*t**2*P2 + t**3*P3

# plot it
plt.plot(*P.T)
plt.show()
```

![Image 3](https://haqr.eu/bezier.png)

Another way to define the same curve is by specifying endpoint normal vectors instead of adding extra control points:

> 定义同一条曲线还有另一种思路：不增加额外的控制点，而是指定端点处的法线向量：

![Image 4](https://haqr.eu/hermite.png)

This is known as a [cubic Hermite spline](https://en.wikipedia.org/wiki/Cubic_Hermite_spline) (1). Here is a python code:

> 这就是所谓的[三次 Hermite 样条](https://en.wikipedia.org/wiki/Cubic_Hermite_spline)（1）。下面是 Python 代码：

1. Traditional Hermite interpolation uses tangent vectors instead of normals, but the idea is similar.

> 1. 传统的 Hermite 插值用的是切向量而不是法线，但思路是相似的。

hermite.py

```python
import numpy as np, matplotlib.pyplot as plt

# Hermite control points and normals
P0, P1, N0, N1 = np.array([0,0]), np.array([3,1]), np.array([-4.5,3]), np.array([3,3])

# Hermite curve
t = np.linspace(0,1,100)[:,None]
P = (2*t**3-3*t**2+1)*P0 + (-2*t**3+3*t**2)*P1 + (t**3-2*t**2+t)*(np.array([[0,1],[-1,0]])@N0) + (t**3-t**2)*(np.array([[0,1],[-1,0]])@N1)

# plot it
plt.plot(*P.T)
plt.show()
```

Back to our task: it is possible to define curved triangles if, in addition to vertex positions, we specify normal vectors at each vertex:

> 回到正题：除了顶点位置之外，如果我们再为每个顶点指定法线向量，就能定义弯曲的三角形：

![Image 5](https://haqr.eu/curved-triangle.png)

Fortunately, artists already provide this data. Until now, we only considered lines starting with `v` and `f` in the [Wavefront OBJ format](https://en.wikipedia.org/wiki/Wavefront_.obj_file). Now, we must also consider `vn` lines, which define 3D normal vectors:

> 幸运的是，美术师们早就准备好了这些数据。在此之前，我们只关心 [Wavefront OBJ 格式](https://en.wikipedia.org/wiki/Wavefront_.obj_file)里以 `v` 和 `f` 开头的行。现在，我们还得考虑定义 3D 法线向量的 `vn` 行：

```text
vn 0 1 0
```

This represents a vector aligned with the \(y\)-axis.
`f` lines (faces) now include references to both vertex and normal indices:

> 它表示一个沿 \(y\) 轴方向的向量。
> 而 `f` 行（面）现在同时引用了顶点索引和法线索引：

```text
f 6/4/1 3/5/3 7/6/5
```

Here, the first and last numbers of each slash-separated triplet correspond to vertex indices (from `v`) and normal indices (from `vn`). For example, this face uses vertices 6, 3, and 7, with normals 1, 3, and 5. Remember: OBJ indices start from 1, so you must subtract 1 when working with C++ arrays.

> 这里，每个用斜杠分隔的三元组中，第一个数和最后一个数分别对应顶点索引（来自 `v`）和法线索引（来自 `vn`）。例如，这个面使用的是顶点 6、3、7，法线 1、3、5。记住：OBJ 的索引从 1 开始，所以在 C++ 数组里使用时要记得减 1。

We’ll handle the remaining `vt` texture coordinate lines later.

> 剩下的 `vt` 纹理坐标行，我们稍后再处理。

Parsing `v`, `vn`, and `f` gives us data like this:

> 解析 `v`、`vn` 和 `f` 之后，我们得到的数据是这样的：

![Image 6](https://haqr.eu/normals.png)

There are vertices connected into triangles, and there are vectors specified per vertex.

> 有连成三角形的顶点，也有逐顶点指定的向量。

### A practical shortcut（一条实用的捷径）

While defining true curved geometry is possible, it’s expensive. Instead, let’s “cheat.” During rasterization, each fragment \(P\) is already computed as a barycentric interpolation of triangle vertices:

> 定义真正的曲面几何虽然可行，但代价太高。所以我们干脆“作弊”：光栅化时，每个片元 \(P\) 本来就是用三角形顶点的重心坐标插值算出来的：

\[
P = \alpha_0 P_0 + \alpha_1 P_1 + \alpha_2 P_2.
\]

Instead of computing the normal as \(\vec{n} = \overrightarrow{P_0P_1} \times \overrightarrow{P_0P_2}\), we interpolate the vertex normals:

> 与其用 \(\vec{n} = \overrightarrow{P_0P_1} \times \overrightarrow{P_0P_2}\) 来计算法线，我们改为对顶点法线做插值：

\[
\vec{n} = \alpha_0 \vec{n_0} + \alpha_1 \vec{n_1} + \alpha_2 \vec{n_2}.
\]

This gives varying shading across the triangle surface:

> 这样一来，三角形表面上的着色就产生了变化：

![Image 7](https://haqr.eu/interpolated-normal.png)

The geometry remains flat, but the varying shading tricks the eye into perceiving smoothness.
Since usually the curvature of the triangles is pretty low, this simple computational hack is enough to produce visually plausible results.

> 几何上它仍然是平的，但变化的着色骗过了眼睛，让人觉得表面是光滑的。
> 由于三角形的曲率通常都很低，这个小小的计算技巧就足以产生视觉上说得过去的结果。

### Homework 1: smooth shading（作业 1：平滑着色）

Parse per-vertex normals and update your shader to use them. The faceted look will disappear, and you’ll obtain smooth surfaces:

> 解析逐顶点法线，并修改着色器来使用它们。棱面感会消失，你会得到光滑的表面：

![Image 8](https://haqr.eu/phong.png)

Although the triangles are still flat (noticeable at the silhouette), the shading convinces the eye otherwise. [Here is my implementation](https://github.com/ssloy/tinyrenderer/commit/e42447a9bf2a64f4a33bcb755f9ff54d8787420b) for reference, but try implementing it yourself first.

> 尽管三角形仍然是平的（在轮廓边缘能看出来），但着色却让眼睛相信了它是光滑的。我的实现在[这里](https://github.com/ssloy/tinyrenderer/commit/e42447a9bf2a64f4a33bcb755f9ff54d8787420b)供参考，不过建议你先自己动手实现。

### Normal vector transformation caveat（法线向量变换的注意事项）

If you have implemented the [camera model](../camera/), then the vertices are transformed by the `ModelView` matrix.
Since normal vectors are also given in the model space, and the shading is done in camera space, we need to transform the normals along with the vertices.
It is tempting to apply `ModelView` matrix to the normals as well, and it will work most of the time. There is a caveat though.

> 如果你已经实现了[相机模型](../camera/)，那么顶点会被 `ModelView` 矩阵变换。
> 由于法线向量也是在模型空间中给出的，而着色是在相机空间中进行的，所以我们需要把法线和顶点一起变换。
> 人们很容易想当然地把 `ModelView` 矩阵也作用到法线上，大多数时候这都没问题。不过这里有个坑。

![Image 9](https://haqr.eu/mit.svg)

Imagine we have a 2D scene made of one segment with endpoints (0, 1) and (1, 0).
Then the tangent vector \(\vec t\) can be found as the difference between them:

> 想象一个 2D 场景，里面只有一条端点分别为 (0, 1) 和 (1, 0) 的线段。
> 那么切向量 \(\vec t\) 就是这两个端点之差：

$$
    \vec{t} := (1\ 0)^\top - (0\ 1)^\top = (1\ -1)^\top.
$$

Vector \(\vec{n} := (1\ 1)^\top\) is normal to the segment, since \(\vec t \perp \vec n\):

> 向量 \(\vec{n} := (1\ 1)^\top\) 垂直于这条线段，因为 \(\vec t \perp \vec n\)：

\[
\vec{t} \cdot \vec{n} = 1\cdot 1 + (-1)\cdot 1 = 0.
\]

I want to transform the geometry by the matrix \(M\) defined as follows:

> 我想用一个如下定义的矩阵 \(M\) 来变换这个几何体：

\[
M := \begin{pmatrix}1 & 0 \\ 0 & 2\end{pmatrix}.
\]

The transformed tangent \(\vec{t'}\) is easy to compute:

> 变换后的切向量 \(\vec{t'}\) 很好算：

\[
\vec{t'} = M\ \vec t = (1\ -2)^\top.
\]

Now let us check the vector \(\vec m\) that was obtained as the direct transformation of \(\vec n\) by \(M\):

> 再看看直接把 \(\vec n\) 用 \(M\) 变换得到的向量 \(\vec m\)：

\[
\vec{m} := M\ \vec n = (1\ 2)^\top.
\]

It turns out that \(\vec m\) is __not__ orthogonal to \(\vec{t'}\):

> 结果发现，\(\vec m\) __并不__与 \(\vec{t'}\) 正交：

\[
\vec{t'}\cdot\vec{m} = 1\cdot 1 + (-2)\cdot 2 \neq 0.
\]

In other words, __we do not need to transform normals, we need to compute normals to the transformed geometry.__
It is easy to do, let us define another vector \(\vec{n'}\) as follows:

> 换句话说，__我们不该去变换法线，而应该去计算变换后几何体的法线。__
> 这并不难，我们定义另一个向量 \(\vec{n'}\)：

\[
\vec{n'} := (M^{-1})^{\top} \vec{n}
\]

We can easily verify that \(\vec{n'} \perp \vec{t'}\):

> 很容易验证 \(\vec{n'} \perp \vec{t'}\)：

\[
\vec{n'} \cdot \vec{t'} = ((M^{-1})^\top \vec{n}) \cdot (M \vec{t}) =  \vec{n} \cdot (M^{-1} M \vec{t}) = \vec n \cdot \vec t = 0.
\]

In this line I have used the property of the dot product \((B^\top \vec{a}) \cdot \vec{b} = \vec{a} \cdot (B\vec{b})\).

> 这一行里我用到了点积的性质 \((B^\top \vec{a}) \cdot \vec{b} = \vec{a} \cdot (B\vec{b})\)。

Let us verify the reasoning on our example:

> 我们用刚才的例子来验证这个推理：

\[
(M^{-1})^\top = \begin{pmatrix}1 & 0 \\ 0 & \frac12\end{pmatrix},
\]

and since the vector \(\vec{n'}\) is defined as \(\vec{n'} := (M^{-1})^{\top} \vec{n}\),

> 而向量 \(\vec{n'}\) 的定义是 \(\vec{n'} := (M^{-1})^{\top} \vec{n}\)，所以

\[
\vec{n'} = \begin{pmatrix}1 & 0 \\ 0 & \frac12\end{pmatrix} \begin{pmatrix}1 \\ 1\end{pmatrix} = \begin{pmatrix}1 \\ \frac12 \end{pmatrix}.
\]

Let us check that \(\vec{n'}\perp \vec{t'}\):

> 再验证一下 \(\vec{n'}\perp \vec{t'}\)：

\[
    \vec{n'}\cdot \vec{t'} = 1 \cdot 1 + \frac12 \cdot (-2) = 0.
\]

It all check out. To sum up, if a model is transformed by a matrix \(M\), we can convert its normal vectors by the matrix \(M^{-\top}\).
[You can check this line of code](https://github.com/ssloy/tinyrenderer/blob/e42447a9bf2a64f4a33bcb755f9ff54d8787420b/main.cpp#L20).

> 全都对得上。总结一下：如果模型被矩阵 \(M\) 变换，那么它的法线向量可以用矩阵 \(M^{-\top}\) 来换算。
> [你可以看看这行代码](https://github.com/ssloy/tinyrenderer/blob/e42447a9bf2a64f4a33bcb755f9ff54d8787420b/main.cpp#L20)。

As I have said, pretty often all this is not necessary, and we can simply transform normals by \(M\). If \(M\) is orthogonal (for example, it is a simple rotation),
then \(M^{-1} = M^\top\), and therefore \(M = M^{-\top}\).
However, if your `ModelView` matrix contains some shearing or uneven scaling, then you need to correctly transform your normals.

> 正如我所说，很多时候这些都不必要，直接拿 \(M\) 变换法线就行。如果 \(M\) 是正交矩阵（比如只是一个简单的旋转），
> 那么 \(M^{-1} = M^\top\)，因此 \(M = M^{-\top}\)。
> 但如果你的 `ModelView` 矩阵里包含切变或者不均匀缩放，那就必须正确地变换法线。

## Textures（纹理）

By introducing per-vertex normals, we improved shading realism. But can we add even more detail? Yes: by storing additional information in textures.

> 通过引入逐顶点法线，我们提升了着色的真实感。但还能再加更多细节吗？能：把额外的信息存进纹理里。

OBJ files also contain `vt` lines, which define 2D coordinates (the [_UV space_](https://en.wikipedia.org/wiki/UV_mapping)). Each face (`f` line) links 3D vertices with 2D texture coordinates. Thus, for every triangle, we now have both its 3D geometry and a 2D mapping:

> OBJ 文件里还有 `vt` 行，用来定义 2D 坐标（即[_UV 空间_](https://en.wikipedia.org/wiki/UV_mapping)）。每个面（`f` 行）都把 3D 顶点与 2D 纹理坐标关联起来。这样一来，每个三角形既有了 3D 几何，又有了 2D 映射：

![Image 10](https://haqr.eu/textures.png)

The 2D space is commonly called the UV space.
The idea is to superpose the flattened 2D mesh onto an image.
When we rasterize a triangle with vertices \(P_0, P_1, P_2\), for every fragment \(P\) we find its barycentric coordinates \(\alpha_0, \alpha_1, \alpha_2\) so that
\(P = \sum \alpha_i P_i\).
We know that the same triangle has coordinates \(U_0, U_1, U_2\) in the UV space, we can therefore compute the UV counterpart \(U\) of the fragement \(P\):

> 这个 2D 空间通常被称为 UV 空间。
> 思路就是把摊平后的 2D 网格叠到一张图像上。
> 当我们光栅化一个顶点为 \(P_0, P_1, P_2\) 的三角形时，对每个片元 \(P\)，我们求出它的重心坐标 \(\alpha_0, \alpha_1, \alpha_2\)，使得
> \(P = \sum \alpha_i P_i\)。
> 同一个三角形在 UV 空间中也有坐标 \(U_0, U_1, U_2\)，因此可以算出片元 \(P\) 在 UV 空间中对应的点 \(U\)：

\[
    U := \alpha_0 U_0 + \alpha_1 U_1 + \alpha_2 U_2.
\]

This process is called UV mapping. Traditionally, it is used to assign fragment colors by sampling an image at \(U\):

> 这个过程叫做 UV 映射。传统上，它通过在 \(U\) 处对图像采样来为片元赋予颜色：

![Image 11](https://haqr.eu/uv.jpg)

Originally, textures stored only colors. Nowadays, they are used much more generally. For example, since a 3D normal vector has three components and a texture has three channels, we can encode normals inside images — a technique known as normal mapping.

> 最初，纹理只用来存颜色。如今它们的用途广泛得多。比如，3D 法线向量有三个分量，纹理有三个通道，于是我们可以把法线编码进图像里——这种技术就叫做法线贴图。

To do this, we remap color channels from  \([0\dots 255]\) into \([-1,1]\) vector components.

> 为此，我们把颜色通道从 \([0\dots 255]\) 重映射到 \([-1,1]\) 的向量分量上。

### Homework 2: normal mapping（作业 2：法线贴图）

For each model, I have prepared files with `_nm.tga` suffix (ignore `_nm_tangent.tga` files for a moment).
Sample normals from these maps and use them in shading to create highly detailed surfaces:

> 我为每个模型都准备了后缀为 `_nm.tga` 的文件（先暂时忽略 `_nm_tangent.tga` 文件）。
> 从这些贴图中采样法线，并在着色中使用，就能创造出细节极其丰富的表面：

![Image 12](https://haqr.eu/normalmap.jpg)

Note that with this approach, we entirely throw away the vertex normals coming from the `vn` lines in the .obj.
Do not worry, they will be useful for tangent space normal mapping.

> 注意，这种做法会把 .obj 文件中 `vn` 行给出的顶点法线完全丢弃。
> 别担心，等到做切线空间法线贴图时，它们就会派上用场。

### Color channel caveat（颜色通道的注意事项）

We need to map RGB channels is mapped to \(xyz\) coordinates, but recall that `TGAColor` structure encodes the channels in the BGRA order.

> 我们需要把 RGB 通道映射到 \(xyz\) 坐标，但别忘了，`TGAColor` 结构是以 BGRA 顺序对通道进行编码的。

For reference, [here is my commit](https://github.com/ssloy/tinyrenderer/commit/f68629a190055e702493bcfa7d7f91427b2be722).

> 供参考，[这是我的提交](https://github.com/ssloy/tinyrenderer/commit/f68629a190055e702493bcfa7d7f91427b2be722)。

### Homework 3: more textures!（作业 3：更多纹理！）

I have ignored colors for as long as I could, but this time it would be unreasonable to avoid them.
You can load colors from the textures with `_diff.tga` suffix.
But there are much more kinds of textures: there are specular textures (`_spec.tga` suffix) that provide the specular intensity weight in the Phong reflection model,
there are incadecence textures (`_glow.tga` suffix), there are [subsurface scattering](https://en.wikipedia.org/wiki/Subsurface_scattering) textures and many, many others that I won’t cover in this tutorial.
Here I used diffuse color + specular textures, try to reproduce the image:

> 颜色我一路能忽略就忽略，但这一次再回避就说不过去了。
> 你可以从后缀为 `_diff.tga` 的纹理中加载颜色。
> 但纹理的种类远不止这些：有镜面反射纹理（`_spec.tga` 后缀），为 Phong 反射模型提供镜面反射强度权重；
> 有自发光纹理（`_glow.tga` 后缀）；还有[次表面散射](https://en.wikipedia.org/wiki/Subsurface_scattering)纹理，以及许许多多本教程不会涉及的其他纹理。
> 这里我用的是漫反射颜色 + 镜面反射纹理，试着复现这张图：

![Image 13](https://haqr.eu/colors.jpg)

---

That concludes this chapter. With just a bit of additional data (per-vertex normals and few textures) we can dramatically increase realism without fundamentally changing our rendering pipeline.

> 本章就到这里。只需一点点额外的数据（逐顶点法线和几张纹理），我们就能大幅提升真实感，而无需从根本上改动渲染管线。

> 原文：[Lesson 8: More data!](https://haqr.eu/tinyrenderer/textures/)
