# Better camera handling（更好的相机处理）

## Chain of coordinate transformations（坐标变换链）

In the [previous chapter](https://haqr.eu/tinyrenderer/camera-naive/), we chained three different transformations to the 3D object to simulate a camera:

> 在[上一章](https://haqr.eu/tinyrenderer/camera-naive/)里，我们把三种不同的变换依次串联到 3D 物体上，以此来模拟相机：

```cpp
auto [ax, ay, az] = project(persp(rot(model.vert(i, 0))));
auto [bx, by, bz] = project(persp(rot(model.vert(i, 1))));
auto [cx, cy, cz] = project(persp(rot(model.vert(i, 2))));
```

Such a transformation chain is standard practice, and usually, it's even longer. Typically:

> 这样的变换链是标准做法，而且通常还会更长。典型的流程是这样的：

1. Objects (e.g., characters) are modeled in their own local frame (object coordinates).
2. They're then placed into a scene (world coordinates).
3. The scene is expressed in the camera frame (eye coordinates).
4. We [deform the scene](https://haqr.eu/tinyrenderer/camera-naive/) to apply a perspective transformation (clip coordinates).
5. Finally, to render on screen, we convert clip coordinates to screen coordinates.

> 1. 物体（比如角色）在各自的局部坐标系（物体坐标）中建模。
> 2. 然后它们被放进场景中（世界坐标）。
> 3. 场景再被表示到相机坐标系下（眼坐标）。
> 4. 我们对[场景做变形](https://haqr.eu/tinyrenderer/camera-naive/)，以施加透视变换（裁剪坐标）。
> 5. 最后，为了在屏幕上渲染，我们把裁剪坐标转换为屏幕坐标。

In total, we use five coordinate systems connected by four transformations. When a 3D vertex `v` is read from a `.obj` file, the rendering pipeline applies this transformation chain:

> 总共算下来，一共是五个坐标系，由四个变换连接起来。当从 `.obj` 文件中读入一个 3D 顶点 `v` 时，渲染管线会施加这样一条变换链：

```cpp
Viewport(Perspective(View(Model(v)))).
```

In the code above, we see three transformations (with `Model` omitted) handled by different functions. Our goal is to unify and compose them. Instead of applying three separate transformations to each vertex, we aim to pre-compose them into one, allowing for faster batch processing of geometry.

> 在上面的代码里，我们看到由不同函数处理的三种变换（`Model` 被省略了）。我们的目标是把它们统一起来，组合成一个。与其对每个顶点分别施加三次变换，我们希望把它们预先合成为一个变换，从而更快地批量处理几何数据。

Ultimately, we want to position, orient, and scale objects in a scene using composable transformations. Let’s examine two classes of transformations that support this goal.

> 归根结底，我们希望用可组合的变换来摆放、定向和缩放场景中的物体。下面就来看看支持这一目标的两类变换。

---

## Linear transformations（线性变换）

In computer graphics, everything you see - models, characters, cameras - is made of __points and vectors__.
To animate, move, rotate, or resize them, we apply linear and affine transformations to their coordinates.

> 在计算机图形学里，你眼前的一切——模型、角色、相机——都是由**点和向量**构成的。要让它们动起来、平移、旋转或缩放，我们就得对它们的坐标施加线性变换和仿射变换。

A linear transformation is a mathematical way to reshape space using just multiplication — no bending, no shifting.
In 1D, this is easy to imagine: multiplying a number by 2 stretches the space, and multiplying by -1 flips it.
For example, the transformation \(x \mapsto 2x\) doubles \(x\), and \(x \mapsto -x\) reflects it across zero.

> 线性变换是一种只用乘法就能重塑空间的数学手段——不弯曲，不平移。在一维中很容易想象：把一个数乘以 2 会把空间拉伸，乘以 -1 则把它翻转。例如，变换 \(x \mapsto 2x\) 会把 \(x\) 翻倍，而 \(x \mapsto -x\) 则把它关于零做镜像反射。

In 2D or 3D the same idea applies, but now we multiply a __vector__ by a __matrix__ instead of a single number.
This matrix controls how space is scaled, rotated, or sheared.
Formally, in \(\mathbb R^n\), a linear transformation can be written as \(\vec x \mapsto A\vec x\),
where \(\vec x\) is a vector (or a point), and \(A\) is a \(n\times n\) matrix.

> 在 2D 或 3D 中道理相同，只不过现在我们要用一个**矩阵**去乘一个**向量**，而不是乘单个数字。这个矩阵控制着空间如何缩放、旋转或错切。严格地说，在 \(\mathbb R^n\) 中，线性变换可以写成 \(\vec x \mapsto A\vec x\)，其中 \(\vec x\) 是一个向量（或点），而 \(A\) 是一个 \(n\times n\) 矩阵。

In 2D \(A\) has for entries: \(A = \begin{bmatrix}a&b\\c&d\end{bmatrix}\), so any 2D point \(\begin{bmatrix}x\\y\end{bmatrix}\) is mapped to \(\begin{bmatrix}ax+by\\cx+dy\end{bmatrix}\), since
\(\begin{bmatrix}a&b\\c&d\end{bmatrix}\begin{bmatrix}x\\y\end{bmatrix} = \begin{bmatrix}ax+by\\cx+dy\end{bmatrix}.\)

> 在 2D 中，\(A\) 的元素为：\(A = \begin{bmatrix}a&b\\c&d\end{bmatrix}\)，因此任意 2D 点 \(\begin{bmatrix}x\\y\end{bmatrix}\) 都会被映射到 \(\begin{bmatrix}ax+by\\cx+dy\end{bmatrix}\)，因为 \(\begin{bmatrix}a&b\\c&d\end{bmatrix}\begin{bmatrix}x\\y\end{bmatrix} = \begin{bmatrix}ax+by\\cx+dy\end{bmatrix}.\)"

Let us see few examples.

> 来看几个例子。

---

### Identity（恒等变换）

The simplest 2D transformation is the identity:

> 最简单的 2D 变换是恒等变换：

\[
\begin{bmatrix}1&0\\0&1\end{bmatrix}
\begin{bmatrix}x\\y\end{bmatrix} = \begin{bmatrix}x\\y\end{bmatrix}
\]

Here is an illustration: I took a colored 2D plane (an image), and applied the identity transformation to every point of the plane.
Without any surprise the image is unchanged.

> 来看个示例：我取了一张彩色的 2D 平面（一张图像），对平面上的每个点施加恒等变换。不出所料，图像纹丝不动。

![Image 3](https://haqr.eu/tinyrenderer/camera/identity.png)

---

### Scaling（缩放）

A scaling matrix stretches objects wider or taller.
For example, \(\begin{bmatrix} 2 & 0 \\ 0 & 1/2 \end{bmatrix}\) stretches the x-axis by 2 and squishes the y-axis by \(1/2\).
In fact, this matrix performs simple 1D multiplication to the coordinates independently one from another.

> 缩放矩阵把物体横向或纵向拉伸。例如 \(\begin{bmatrix} 2 & 0 \\ 0 & 1/2 \end{bmatrix}\) 会把 x 轴拉伸为 2 倍，把 y 轴压缩为 \(1/2\)。实际上，这个矩阵只是对各个坐标互相独立地做简单的 1D 乘法。

![Image 4](https://haqr.eu/tinyrenderer/camera/scaling.png)

---

### Rotation（旋转）

A 2D rotation matrix \(\begin{bmatrix} \cos\theta & -\sin\theta \\ \sin\theta & \cos\theta \end{bmatrix}\) rotates a point counterclockwise by angle \(\theta\) around the orgin.
If we take \(\theta = 90°\), the matrix becomes \(\begin{bmatrix} 0 & -1 \\ 1 & 0 \end{bmatrix}\),
so any 2D point \(\begin{bmatrix}x\\y\end{bmatrix}\) is mapped to \(\begin{bmatrix}-y\\x\end{bmatrix}\), as it can be seen in this illustration:

> 2D 旋转矩阵 \(\begin{bmatrix} \cos\theta & -\sin\theta \\ \sin\theta & \cos\theta \end{bmatrix}\) 会把点绕原点逆时针旋转角度 \(\theta\)。如果取 \(\theta = 90°\)，矩阵就变成 \(\begin{bmatrix} 0 & -1 \\ 1 & 0 \end{bmatrix}\)，于是任意 2D 点 \(\begin{bmatrix}x\\y\end{bmatrix}\) 都被映射到 \(\begin{bmatrix}-y\\x\end{bmatrix}\)，如下图所示：

![Image 5](https://haqr.eu/tinyrenderer/camera/rotation.png)

---

### Shear（错切）

A shear matrix slants shapes like melting jello.
For example, \(\begin{bmatrix} 1 & 1 \\ 0 & 1 \end{bmatrix}\)
slants shapes horizontally, like tilting a rectangle into a parallelogram.

> 错切矩阵会把形状歪斜，就像融化的果冻。例如 \(\begin{bmatrix} 1 & 1 \\ 0 & 1 \end{bmatrix}\) 会沿水平方向把形状推歪，就像把矩形斜拉成平行四边形。

![Image 6](https://haqr.eu/tinyrenderer/camera/shear.png)

---

These transformations are called linear because they preserve straight lines and keep the origin fixed.
Linear transformations have compact representation: a transformation like rotation or scaling in 3D can be represented with a small square matrix.
Moreover, we have the composability we were looking for.
Suppose we have three different transformations \(A_1, A_2\) and \(A_3\) to be chained on every vertex \(\vec x_i\) of the scene (i.e. to compute \(A_3 \times A_2 \times A_1 \times \vec x_i\)).
Note that the multiplication is associative, i.e. \(A_3 \times (A_2 \times (A_1 \times \vec x_i)) = ((A_3 \times A_2) \times A_1) \times \vec x_i\),
therefore, we can precompute \(A = A_3\times A_2 \times A_1\) and apply the matrix \(A\) directly to the vertices by computing the product \(A\times \vec x_i\).

> 这些变换之所以称为线性的，是因为它们保持直线仍是直线，并且让原点保持不动。线性变换的表示很紧凑：3D 中的旋转或缩放这类变换可以用一个小方阵来表示。而且，我们想要的可组合性也具备了。假设有三个不同的变换 \(A_1, A_2\) 和 \(A_3\)，要依次作用到场景的每个顶点 \(\vec x_i\) 上（也就是计算 \(A_3 \times A_2 \times A_1 \times \vec x_i\)）。注意乘法满足结合律，即 \(A_3 \times (A_2 \times (A_1 \times \vec x_i)) = ((A_3 \times A_2) \times A_1) \times \vec x_i\)，因此我们可以预先算出 \(A = A_3\times A_2 \times A_1\)，然后直接对顶点计算乘积 \(A\times \vec x_i\) 即可。

---

## Affine transformations（仿射变换）

Note that linear transformations always map the origin to the origin, since \(A \vec 0 = \vec 0\).
But in graphics, we often need to move (translate) things - hence affine transformations are more general and more useful.

> 注意，线性变换总是把原点映射到原点，因为 \(A \vec 0 = \vec 0\)。但在图形学中，我们经常需要移动（平移）物体——所以更一般、也更有用的仿射变换登场了。

Formally, in \(\mathbb R^n\), an affine transformation \(T\) has form \(T(\vec x)=A\vec x+\vec b\),
where \(\vec x\) is a vector (or a point), \(A\) is a \(n\times n\) matrix representing a linear transformation (like rotation, scaling or shear),
and \(\vec b\) is a translation vector.

> 严格地说，在 \(\mathbb R^n\) 中，仿射变换 \(T\) 的形式为 \(T(\vec x)=A\vec x+\vec b\)，其中 \(\vec x\) 是一个向量（或点），\(A\) 是表示线性变换（如旋转、缩放或错切）的 \(n\times n\) 矩阵，而 \(\vec b\) 是平移向量。

In 2D it looks like this:

> 在 2D 中它长这样：

\[
\begin{bmatrix}x\\y\end{bmatrix} \quad \mapsto \quad \begin{bmatrix}a&b\\c&d\end{bmatrix}\begin{bmatrix}x\\y\end{bmatrix} + \begin{bmatrix}e\\f\end{bmatrix} = \begin{bmatrix}ax+by+e\\cx+dy+f\end{bmatrix}
\]

This expression is really cool.
We can rotate, scale, shear and translate.
However, let us recall that we are interested in composing multiple transformations.
Here is what a composition of two transformations looks like (remember, we need to compose dozens of those):

> 这个表达式非常棒：旋转、缩放、错切、平移都能做。但是别忘了，我们感兴趣的是把多个变换组合起来。下面是两个变换复合起来的样子（记住，我们需要复合几十个这样的变换）：

\[
\begin{bmatrix}x\\y\end{bmatrix} \quad \mapsto \quad \begin{bmatrix}a_2&b_2\\c_2&d_2\end{bmatrix}
\left(\begin{bmatrix}a_1&b_1\\c_1&d_1\end{bmatrix}\begin{bmatrix}x\\y\end{bmatrix} + \begin{bmatrix}e_1\\f_1\end{bmatrix}\right)
+\begin{bmatrix}e_2\\f_2\end{bmatrix}
\]

It is starting to look ugly even for a single composition, add more and things get even worse.

> 仅仅复合一次就已经开始变丑了，再多叠几个更是惨不忍睹。

---

## Homogeneous Coordinates（齐次坐标）

Okay, now it is the time for the black magic. Imagine that I add one column and one row to our transformation matrix (thus making it 3x3 for a 2D transformation and 4x4 for 3D) and append one coordinate always equal to 1 to our vector to be transformed:

> 好，现在是时候亮出黑魔法了。想象一下，我给变换矩阵加上一列和一行（于是 2D 变换的矩阵变成 3x3，3D 的变成 4x4），并给待变换的向量末尾追加一个恒等于 1 的坐标：

\[
\begin{bmatrix}
T(\vec{x}) \\
1
\end{bmatrix}
=
\begin{bmatrix}
A & \vec{b} \\
\vec{0}^\top & 1
\end{bmatrix}
\begin{bmatrix}
\vec{x} \\
1
\end{bmatrix}
\]

Let us write it down explicitly for 2D:

> 把它在 2D 下明确地写出来：

\[
\begin{bmatrix}a&b&e\\c&d&f\\0&0&1\end{bmatrix}\begin{bmatrix}x\\y\\1\end{bmatrix} = \begin{bmatrix}ax+by+e\\cx+dy+f\\1\end{bmatrix}
\]

This matrix captures the entire transformation and is easy to store and manipulate.
If we multiply this matrix and the vector augmented by 1 we get another vector with 1 in the last component, but the other two components have exactly the shape we would like! Magic.

> 这个矩阵完整地刻画了整个变换，而且便于存储和操作。用它乘上补了一个 1 的向量，得到的新向量最后一个分量还是 1，而另外两个分量恰好就是我们想要的形式！神奇。

In fact, the idea is really simple. Translations are not linear in the 2D space. So we embed our 2D into 3D space (by simply adding 1 for the 3rd component).
It means that our 2D space is the plane \(z=1\) in the 3D space. Then we perform a linear 3D transformation and project the result onto our 2D physical plane.
How do we project 3D back onto the 2D plane? Simply by dividing by the third component:

> 其实这个想法非常简单。平移在 2D 空间中不是线性的，所以我们把 2D 空间嵌入到 3D 空间中（第 3 个分量直接补 1）。也就是说，我们的 2D 空间就是 3D 空间中的平面 \(z=1\)。然后我们执行一个线性的 3D 变换，再把结果投影回 2D 物理平面。怎么把 3D 投影回 2D 平面？除以第三个分量就行了：

\[
\begin{bmatrix}x\\ y\\ z\end{bmatrix} \quad \mapsto \quad \begin{bmatrix}x/z \\ y/z\end{bmatrix}
\]

In the above example the 3rd component is always 1 independently from \(A\) and \(\vec b\), but very soon we will meet other cases.
In fact, we have just used __homogeneous coordinates__.
They are called _homogeneous_ because of the way they use ratios of coordinates that remain consistent under scaling - a key idea from projective geometry.

> 在上面的例子里，第 3 个分量始终是 1，与 \(A\) 和 \(\vec b\) 无关；但很快我们就会遇到别的情况。事实上，我们刚刚用到的就是**齐次坐标**。它之所以称为「齐次」，是因为它利用坐标之间的比例关系——这些比例在缩放之下保持一致——这正是射影几何中的一个关键思想。

In the process I described, a point \((x, y)\) in Cartesian coordinates becomes \((x, y, 1)\) in homogeneous coordinates.
More generally, a 2D point \((x, y)\) is represented as:

> 在我描述的流程里，笛卡尔坐标中的点 \((x, y)\) 在齐次坐标中变成 \((x, y, 1)\)。更一般地，2D 点 \((x, y)\) 表示为：

\[
(x, y) \quad \mapsto \quad (wx, wy, w)
\]

where \(w \ne 0\), and the point in Cartesian coordinates is recovered by:

> 其中 \(w \ne 0\)，要还原出笛卡尔坐标的点，只需：

\[
\left(\frac{wx}{w}, \frac{wy}{w}\right) = (x, y)
\]

So any scalar multiple of \((x, y, 1)\) — like \((2x, 2y, 2)\), or \((5x, 5y, 5)\) — represents the same point in the plane. That's the essence of homogeneity different triples represent the same point because they are all scalar multiples.

> 所以 \((x, y, 1)\) 的任意标量倍数——比如 \((2x, 2y, 2)\) 或 \((5x, 5y, 5)\)——都表示平面上的同一个点。这就是齐次性的本质：不同的三元组表示同一个点，因为它们互为标量倍数。

In computer graphics, we extend affine transformations into matrix form using homogeneous coordinates allowing all transformations (including translation) to be expressed as a single matrix.
This is why we often use 3×3 matrices in 2D and 4×4 matrices in 3D graphics.
While being strange at the first sight, the approach is very straightforward, and it offers two serious advantages:

> 在计算机图形学中，我们用齐次坐标把仿射变换扩展成矩阵形式，让所有变换（包括平移）都能用单个矩阵来表达。这就是为什么 2D 图形学中常用 3×3 矩阵，3D 中常用 4×4 矩阵。这个做法乍看很奇怪，其实非常直白，而且带来两个非常重要的好处：

1. __Translation becomes matrix multiplication.__
   Affine transformations (like translation) are not linear in Cartesian coordinates but are linear in homogeneous coordinates, thus allowing for the composability we were looking for.
2. __Enables perspective projection.__
   Homogeneous coordinates naturally handle perspective division, which is key for realistic rendering of 3D scenes (we will revisit it shortly).
   Therefore, all camera handling (rotation, scaling, translation, projection) can be done within the same consistent framework.

> 1. **平移变成了矩阵乘法。** 仿射变换（比如平移）在笛卡尔坐标下不是线性的，但在齐次坐标下是线性的，于是我们想要的可组合性就到手了。
> 2. **让透视投影成为可能。** 齐次坐标能自然地处理透视除法，这对 3D 场景的逼真渲染至关重要（我们很快就会再提到它）。因此，所有相机操作（旋转、缩放、平移、投影）都可以在同一个一致的框架内完成。

---

## Putting it all together（整合起来）

Recall that in the previous chapter we have chained three different transformations `project`, `persp` and `rot`:

> 回想一下，上一章里我们串联了三种变换：`project`、`persp` 和 `rot`：

```cpp
auto [ax, ay, az] = project(persp(rot(model.vert(i, 0))));
auto [bx, by, bz] = project(persp(rot(model.vert(i, 1))));
auto [cx, cy, cz] = project(persp(rot(model.vert(i, 2))));
```

Let us review them one at a time and see how we can replace it with a matrix version.
Basically, we want to compute three \(4\times 4\) matrices `Viewport`, `Perspective` and `ModelView`:

> 我们逐个检视它们，看看如何换成矩阵版本。基本上，我们要算出三个 \(4\times 4\) 矩阵：`Viewport`、`Perspective` 和 `ModelView`：

```cpp
mat<4,4> ModelView, Viewport, Perspective;
```

### Viewport（视口）

The easiest one is the viewport transform that scales the scene to the size of the screen. Last time it was defined as follows:

> 最简单的是视口变换，它把场景缩放到屏幕大小。上次它是这样定义的：

```cpp
std::tuple<int,int,int> project(vec3 v) { // First of all, (x,y) is an orthogonal projection of the vector (x,y,z).
    return { (v.x + 1.) *  width/2,       // Second, since the input models are scaled to have fit in the [-1,1]^3 world coordinates,
             (v.y + 1.) * height/2,       // we want to shift the vector (x,y) and then scale it to span the entire screen.
             (v.z + 1.) *   255./2 };
}
```

In fact, it is a by-hand computation of the following matrix multiplication:

> 事实上，它就是下面这个矩阵乘法的手工展开：

\[
\begin{bmatrix}\frac{\text{width}}{2}&0&0&\frac{\text{width}}{2} \\ 0&\frac{\text{height}}{2}&0&\frac{\text{height}}{2}\\ 0&0&\frac{255}{2}&\frac{255}{2}\\ 0 &0 &0&1\end{bmatrix}
\begin{bmatrix}v_x \\ v_y \\ v_z \\ 1\end{bmatrix}
\]

Once again, this function maps \([-1..1]\) range in clip coordinates (re-check the transformation chain in the beginning of this chaper) to the entire screen.
In addition to that, it maps \([-1..1]\) range of \(z\) coordinates to \([0..255]\) that I used for debugging of the \(z\)-buffer.
For the rest of the series I'll use following viewport matrix:

> 再说一遍，这个函数把裁剪坐标中的 \([-1..1]\) 范围（请回看本章开头的变换链）映射到整个屏幕。除此之外，它还把我用来调试 \(z\) 缓冲的 \(z\) 坐标从 \([-1..1]\) 范围映射到 \([0..255]\)。在本系列后续内容中，我会使用下面这个视口矩阵：

\[
\begin{bmatrix}\frac{w}{2}&0&0&x+\frac{w}{2} \\ 0&\frac{h}{2}&0&y+\frac{h}{2}\\ 0&0&1&0\\ 0 &0 &0&1\end{bmatrix}
\]

I do not need anymore to re-map \(z\)-values to grayscale colors, so I leave it them as is.
In addition to that, \([-1..1] \times [-1..1]\) range in clip coordinates is not mapped to the entire screen anymore,
but rather to a window described by its top left corner \((x,y)\) and \((w,h)\) being the size of the window.
In my code, I setup the \(4\times 4\) `Viewport` matrix with the following function:

> 我不再需要把 \(z\) 值重新映射成灰度颜色了，所以就保持原样。此外，裁剪坐标中的 \([-1..1] \times [-1..1]\) 范围也不再映射到整个屏幕，而是映射到一个窗口——窗口由左上角 \((x,y)\) 和尺寸 \((w,h)\) 来描述。在我的代码里，我用下面这个函数来设置 \(4\times 4\) 的 `Viewport` 矩阵：

```cpp
void viewport(const int x, const int y, const int w, const int h) {
    Viewport = {{{w/2., 0, 0, x+w/2.}, {0, h/2., 0, y+h/2.}, {0,0,1,0}, {0,0,0,1}}};
}
```

### Perspective deformation（透视变形）

In the last chapter we saw that we can decompose a central projection into a perspective deformation of the space followed by an orthographic projection.
For a camera with focal length \(f\) located on the \(z\)-axis, the perspective deformation can be computed as follows:

> 上一章我们看到，可以把中心投影分解为：先对空间做透视变形，再进行正交投影。对于位于 \(z\) 轴上、焦距为 \(f\) 的相机，透视变形可以这样计算：

```cpp
vec3 persp(vec3 v) {
    constexpr double f = 3.;
    return v / (1-v.z/f);
}
```

This computation may be formulated in a matrix way in homogeneous coordinates.
For a 3D point \((v_x, v_y, v_z)\), we embed it into 4D by adding 1 as the last coordinate: \((v_x, v_y, v_z, 1)\).
Next, we multiply it with the perspective matrix, getting again a 4D point.
Finally, we re-project (do not confond with the projection to the screen) the result to 3D by dividing by the last coordinate.
Here is the computation flow for the above function:

> 这个计算可以用齐次坐标写成矩阵形式。对于 3D 点 \((v_x, v_y, v_z)\)，我们补上 1 作为最后一个坐标，把它嵌入到 4D：\((v_x, v_y, v_z, 1)\)。接着用透视矩阵乘它，得到的仍是一个 4D 点。最后除以最后一个坐标，把结果重新投影回 3D（别与投影到屏幕混淆）。上面那个函数的计算流程如下：

\[
\begin{bmatrix}v_x \\ v_y \\ v_z \end{bmatrix} \quad \mapsto \quad
\begin{bmatrix}1&0&0& \\ 0&1&0&0\\ 0&0&1&0\\ 0 &0 &-\frac{1}{f}&1\end{bmatrix}
\begin{bmatrix}v_x \\ v_y \\ v_z \\ 1\end{bmatrix}=
\begin{bmatrix}v_x \\ v_y \\ v_z \\ 1-\frac{v_z}{f}\end{bmatrix}
\quad \mapsto \quad \frac{1}{ 1-\frac{v_z}{f}}\begin{bmatrix}v_x \\ v_y \\ v_z \end{bmatrix}
\]

So, I setup the \(4\times 4\) `Perspective` matrix located at the \(z\)-axis with focal distance \(f\) using the following function:

> 于是，我用下面这个函数来设置位于 \(z\) 轴、焦距为 \(f\) 的 \(4\times 4\) `Perspective` 矩阵：

```cpp
void perspective(const double f) {
    Perspective = {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0, -1/f,1}}};
}
```

### ModelView（模型视图矩阵）

In the last chapter I have simply rotated the scene to simulate a camera movement.
Let us check a more handy way to describe a camera, inspired by [gluLookAt](https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/gluLookAt.xml) function from the OpenGL Utility Library.
GLU proposes to position the camera onto the world space by specifying three view parameters, two 3D points (\(\text{eye}\) and \(\text{center}\)) and one 3D vector (\(\overrightarrow{\text{up}}\)),
all three measured in world space.

> 上一章我只是旋转了场景来模拟相机移动。下面来看一种更顺手的相机描述方式，它的灵感来自 OpenGL 工具库的 [gluLookAt](https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/gluLookAt.xml) 函数。GLU 提出通过指定三个视图参数来把相机放置到世界空间中：两个 3D 点（\(\text{eye}\) 和 \(\text{center}\)）和一个 3D 向量（\(\overrightarrow{\text{up}}\)），三者都在世界空间中度量的。

- The point \(\text{eye}\) defines the location of the camera.
- The point \(\text{center}\) denotes the direction where the camera is aiming at, usually at the center of the world or an object.
- The vector \(\overrightarrow{\text{up}}\) denotes roughly the upward orientation of the camera.

> - 点 \(\text{eye}\) 定义了相机的位置。
> - 点 \(\text{center}\) 表示相机瞄准的方向，通常指向世界的中心或某个物体。
> - 向量 \(\overrightarrow{\text{up}}\) 大致表示相机的朝上方向。

Here is an illustration:

> 如图所示：

![Image 7](https://haqr.eu/tinyrenderer/camera/glulookat.svg)

It means that we want to do the rendering in the frame \((\text{center}, \vec l, \vec m, \vec n)\),
where \(\vec l, \vec m\) and \(\vec n\) are computed from the parameters \(\text{eye}, \text{center}\) and \(\overrightarrow{\text{up}}\).

> 这意味着我们希望在坐标系 \((\text{center}, \vec l, \vec m, \vec n)\) 中进行渲染，其中 \(\vec l, \vec m\) 和 \(\vec n\) 由参数 \(\text{eye}, \text{center}\) 和 \(\overrightarrow{\text{up}}\) 计算得到。

#### Build the camera frame（构建相机坐标系）

Let us see how to compute three basis vectors \((\vec l, \vec m, \vec n)\), and then we will see how to compose the transformation matrix.
First of all, our perspective deformation supposes that the eye lies on the \(\vec n\) axis, therefore we can define \(\vec n\) as:

> 我们先看看如何计算三个基向量 \((\vec l, \vec m, \vec n)\)，然后再看怎样组合出变换矩阵。首先，我们的透视变形假设眼睛位于 \(\vec n\) 轴上，因此可以把 \(\vec n\) 定义为：

\[
\vec n := \frac{\text{eye} - \text{center}}{||\text{eye} - \text{center}||}.
\]

Then, we want vector \(\vec{\text{up}}\) vertical in the final projection. It means that \(\vec l\) must be orthogonal to the plane spanned by \(\vec{\text{up}}\) and \(\vec n\).
Therefore, we can compute \(\vec l\) by a cross product between \(\vec{\text{up}}\) and \(\vec n\):

> 然后，我们希望向量 \(\vec{\text{up}}\) 在最终投影中是竖直的。这意味着 \(\vec l\) 必须与 \(\vec{\text{up}}\) 和 \(\vec n\) 张成的平面正交。因此可以用 \(\vec{\text{up}}\) 与 \(\vec n\) 的叉积来算出 \(\vec l\)：

\[
\vec l := \frac{\vec{\text{up}} \times \vec n}{||\vec{\text{up}} \times \vec n||}.
\]

And then recover \(\vec m\) such that it is orthogonal to already calculated \(\vec l\) and \(\vec n\) (recall that in our problem settings \(\text{center} - \text{eye}\) and \(\vec{\text{up}}\) are not necessarily orthogonal):

> 接着求出 \(\vec m\)，让它与已经算出的 \(\vec l\) 和 \(\vec n\) 正交（回忆一下，在我们的问题设定中 \(\text{center} - \text{eye}\) 与 \(\vec{\text{up}}\) 不一定正交）：

\[
\vec m := \frac{\vec{n} \times \vec l}{||\vec{n} \times \vec l||}.
\]

#### Change of basis in 3D space（3D 空间中的基变换）

Now we have computed the camera frame, we need a way to transform coordinates from the world frame to the camera frame.
In Euclidean space, coordinates can be given by a point (the origin) and a basis. What does it mean that point \(P\) has coordinates \((x,y,z)\) in the frame \((O, \vec i,\vec j,\vec k)\)?
It means that the vector \(\overrightarrow{OP}\) can be expressed as follows:

> 现在相机坐标系已经算好了，我们还需要一种把坐标从世界坐标系变换到相机坐标系的方法。在欧几里得空间中，坐标系可以由一个点（原点）和一组基给出。点 \(P\) 在坐标系 \((O, \vec i,\vec j,\vec k)\) 中的坐标是 \((x,y,z)\)，这意味着什么？它意味着向量 \(\overrightarrow{OP}\) 可以这样表示：

\[
\overrightarrow{OP} = \vec{i}x + \vec{j}y + \vec{k}z = \begin{bmatrix}\vec{i} & \vec{j} & \vec{k}\end{bmatrix}\begin{bmatrix}x \\ y \\ z\end{bmatrix}
\]

Now imagine that we have another frame \((C, \vec l,\vec m,\vec n)\).
How do we transform coordinates given in one frame to another?
First of all let us note that since \((\vec i,\vec j,\vec k)\) and \((\vec l, \vec m,\vec n)\) are bases of 3D, there exists a (non degenerate) matrix \(M\) such that:

> 现在想象我们还有另一个坐标系 \((C, \vec l,\vec m,\vec n)\)。怎么把一个坐标系里给出的坐标变换到另一个坐标系？首先要注意：既然 \((\vec i,\vec j,\vec k)\) 和 \((\vec l, \vec m,\vec n)\) 都是 3D 的基，那么就存在一个（非退化的）矩阵 \(M\) 使得：

\[
\begin{bmatrix}\vec{l} & \vec{m} & \vec{n}\end{bmatrix} =
\begin{bmatrix}\vec{i} & \vec{j} & \vec{k}\end{bmatrix} \times M
\]

Let us draw an illustration:

> 画个图说明一下：

![Image 8](https://haqr.eu/tinyrenderer/camera/basis_change.svg)

Then let us re-express the vector OP:

> 然后把向量 OP 重新表达一下：

\[
\overrightarrow{OP} = \overrightarrow{OC} + \overrightarrow{CP} =
\begin{bmatrix}\vec{i} & \vec{j} & \vec{k}\end{bmatrix}
\begin{bmatrix}C_x \\ C_y \\ C_z\end{bmatrix} +
\begin{bmatrix}\vec{l} & \vec{m} & \vec{n}\end{bmatrix}
\begin{bmatrix}x' \\ y' \\ z'\end{bmatrix},
\]

where \((x', y', z')\) are the coordinates of the point \(P\) in the frame \((C, \vec l,\vec m,\vec n)\),
and \((C_x, C_y, C_Z)\) are the coordinates of the point \(C\) in the frame \((O, \vec i,\vec j,\vec k)\).

> 其中 \((x', y', z')\) 是点 \(P\) 在坐标系 \((C, \vec l,\vec m,\vec n)\) 中的坐标，而 \((C_x, C_y, C_Z)\) 是点 \(C\) 在坐标系 \((O, \vec i,\vec j,\vec k)\) 中的坐标。

Now let us substitute \((\vec l,\vec m,\vec n)\) in the right part with the change of basis matrix:

> 现在把右边的 \((\vec l,\vec m,\vec n)\) 替换成基变换矩阵：

\[
\overrightarrow{OP} =
\begin{bmatrix}\vec{i} & \vec{j} & \vec{k}\end{bmatrix}\left(
\begin{bmatrix}C_x \\ C_y \\ C_z\end{bmatrix} +
M \begin{bmatrix}x' \\ y' \\ z'\end{bmatrix} \right)
\]

And it gives us the formula to transform coordinates from one frame to another:

> 这样我们就得到了把一个坐标系的坐标变换到另一个坐标系的公式：

\[
\begin{bmatrix}x \\ y \\ z\end{bmatrix} =
\begin{bmatrix}C_x \\ C_y \\ C_z\end{bmatrix} +
M \begin{bmatrix}x' \\ y' \\ z'\end{bmatrix}
\qquad\Rightarrow\qquad
\begin{bmatrix}x' \\ y' \\ z'\end{bmatrix} =
M^{-1}\left( \begin{bmatrix}x \\ y \\ z\end{bmatrix} - \begin{bmatrix}C_x \\ C_y \\ C_z\end{bmatrix}\right)
\]

#### The matrix（矩阵）

So, we have two major equations to consider:

> 于是，我们有两个主要方程要考虑：

\[
\begin{bmatrix}\vec{l} & \vec{m} & \vec{n}\end{bmatrix} =
\begin{bmatrix}\vec{i} & \vec{j} & \vec{k}\end{bmatrix} \times M
\]

and

> 以及

\[
\begin{bmatrix}x' \\ y' \\ z'\end{bmatrix} =
M^{-1}\left( \begin{bmatrix}x \\ y \\ z\end{bmatrix} - \begin{bmatrix}C_x \\ C_y \\ C_z\end{bmatrix}\right).
\]

The first one will give us a way to compute the matrix \(M\), while the second one shows how to recompute coordinates in the camera frame.

> 第一个方程让我们能算出矩阵 \(M\)，第二个则告诉我们如何在相机坐标系中重新计算坐标。

In our case, the basis \((\vec i, \vec j, \vec k)\) is the global world basis, therefore \(\begin{bmatrix}\vec{i} & \vec{j} & \vec{k}\end{bmatrix}\) is the \(3\times 3\) identity matrix,
so the \(3\times 3\) basis change matrix \(M\) can be found as

> 在我们的情形中，基 \((\vec i, \vec j, \vec k)\) 就是全局世界基，因此 \(\begin{bmatrix}\vec{i} & \vec{j} & \vec{k}\end{bmatrix}\) 是 \(3\times 3\) 单位矩阵，于是 \(3\times 3\) 的基变换矩阵 \(M\) 可以这样求得：

\[
M = \begin{bmatrix}\vec{l}_x & \vec{m}_x & \vec{n}_x \\ \vec{l}_y & \vec{m}_y & \vec{n}_y \\ \vec{l}_z & \vec{m}_z & \vec{n}_z \end{bmatrix}.
\]

The matrix \(M\) is orthonormal, therefore, to compute \(M^{-1}\), it suffices to transpose it:

> 矩阵 \(M\) 是正交归一的，因此要计算 \(M^{-1}\)，转置一下就够了：

\[
M^{-1} = M^\top = \begin{bmatrix}\vec{l}_x & \vec{l}_y & \vec{l}_z \\ \vec{m}_x & \vec{m}_y & \vec{m}_z \\ \vec{n}_x & \vec{n}_y & \vec{n}_z \end{bmatrix}.
\]

The very last step is to handle the translation. Let us write it explicitly the transformation to the camera frame in the homogeneous coordinates:

> 最后一步是处理平移。我们把到相机坐标系的变换用齐次坐标明确地写出来：

\[
\begin{bmatrix}x' \\ y' \\ z' \\ 1\end{bmatrix} =
\underbrace{\begin{bmatrix}\vec{l}_x & \vec{l}_y & \vec{l}_z & 0\\ \vec{m}_x & \vec{m}_y & \vec{m}_z & 0 \\ \vec{n}_x & \vec{n}_y & \vec{n}_z & 0 \\ 0 & 0 & 0 & 1\end{bmatrix}
\begin{bmatrix}1 & 0 & 0 & -C_x\\ 0 & 1 & 0 & -C_y \\ 0 & 0 & 1 & -C_z \\ 0 & 0 & 0 & 1\end{bmatrix}}_{\text{ModelView}}
\begin{bmatrix}x \\ y \\ z \\ 1\end{bmatrix}.
\]

Now it suffices to get any point with coordinates `(x,y,z,1)` in the model frame, multiply it by the matrix `ModelView` and we get the coordinates in the camera frame!
Long story short, here is the code that sets up the `ModelView` matrix:

> 现在，只需取模型坐标系中任意一个坐标为 `(x,y,z,1)` 的点，乘上 `ModelView` 矩阵，就得到了它在相机坐标系中的坐标！长话短说，下面是设置 `ModelView` 矩阵的代码：

```cpp
void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    vec3 n = normalized(eye-center);
    vec3 l = normalized(cross(up,n));
    vec3 m = normalized(cross(n, l));
    ModelView = mat<4,4>{{{l.x,l.y,l.z,0}, {m.x,m.y,m.z,0}, {n.x,n.y,n.z,0}, {0,0,0,1}}} *
                mat<4,4>{{{1,0,0,-center.x}, {0,1,0,-center.y}, {0,0,1,-center.z}, {0,0,0,1}}};
}
```

Lines 3-5 compute the basis vectors, and lines 6-7 compose the change of basis matrix with the appropriate translation.

> 第 3–5 行计算基向量，第 6–7 行把基变换矩阵与相应的平移组合起来。

### The final code（最终代码）

Here is the revision handles all the chain of coordinate transformations explicitly ([check the commit](https://github.com/ssloy/tinyrenderer/tree/592ab4d89af9cb71517cdb6fc6dc882a463a4b65)):

> 下面是这次修订的版本，它显式地处理整条坐标变换链（[见这个提交](https://github.com/ssloy/tinyrenderer/tree/592ab4d89af9cb71517cdb6fc6dc882a463a4b65)）：

Camera handling

```cpp
#include <limits>
#include <algorithm>
#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

mat<4,4> ModelView, Viewport, Perspective;

void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    vec3 n = normalized(eye-center);
    vec3 l = normalized(cross(up,n));
    vec3 m = normalized(cross(n, l));
    ModelView = mat<4,4>{{{l.x,l.y,l.z,0}, {m.x,m.y,m.z,0}, {n.x,n.y,n.z,0}, {0,0,0,1}}} *
                mat<4,4>{{{1,0,0,-center.x}, {0,1,0,-center.y}, {0,0,1,-center.z}, {0,0,0,1}}};
}

void perspective(const double f) {
    Perspective = {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0, -1/f,1}}};
}

void viewport(const int x, const int y, const int w, const int h) {
    Viewport = {{{w/2., 0, 0, x+w/2.}, {0, h/2., 0, y+h/2.}, {0,0,1,0}, {0,0,0,1}}};
}

void rasterize(const vec4 clip[3], std::vector<double> &zbuffer, TGAImage &framebuffer, const TGAColor color) {
    vec4 ndc[3]    = { clip[0]/clip[0].w, clip[1]/clip[1].w, clip[2]/clip[2].w };                // normalized device coordinates
    vec2 screen[3] = { (Viewport*ndc[0]).xy(), (Viewport*ndc[1]).xy(), (Viewport*ndc[2]).xy() }; // screen coordinates

    mat<3,3> ABC = {{ {screen[0].x, screen[0].y, 1.}, {screen[1].x, screen[1].y, 1.}, {screen[2].x, screen[2].y, 1.} }};
    if (ABC.det()<1) return; // backface culling + discarding triangles that cover less than a pixel

    auto [bbminx,bbmaxx] = std::minmax({screen[0].x, screen[1].x, screen[2].x}); // bounding box for the triangle
    auto [bbminy,bbmaxy] = std::minmax({screen[0].y, screen[1].y, screen[2].y}); // defined by its top left and bottom right corners
#pragma omp parallel for
    for (int x=std::max<int>(bbminx, 0); x<=std::min<int>(bbmaxx, framebuffer.width()-1); x++) { // clip the bounding box by the screen
        for (int y=std::max<int>(bbminy, 0); y<=std::min<int>(bbmaxy, framebuffer.height()-1); y++) {
            vec3 bc = ABC.invert_transpose() * vec3{static_cast<double>(x), static_cast<double>(y), 1.}; // barycentric coordinates of {x,y} w.r.t the triangle
            if (bc.x<0 || bc.y<0 || bc.z<0) continue;                                                    // negative barycentric coordinate => the pixel is outside the triangle
            double z = bc * vec3{ ndc[0].z, ndc[1].z, ndc[2].z };
            if (z <= zbuffer[x+y*framebuffer.width()]) continue;
            zbuffer[x+y*framebuffer.width()] = z;
            framebuffer.set(x, y, color);
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    constexpr int width  = 800;    // output image size
    constexpr int height = 800;
    constexpr vec3    eye{-1,0,2}; // camera position
    constexpr vec3 center{0,0,0};  // camera direction
    constexpr vec3     up{0,1,0};  // camera up vector

    lookat(eye, center, up);                              // build the ModelView   matrix
    perspective(norm(eye-center));                        // build the Perspective matrix
    viewport(width/16, height/16, width*7/8, height*7/8); // build the Viewport    matrix

    TGAImage framebuffer(width, height, TGAImage::RGB);
    std::vector<double> zbuffer(width*height, -std::numeric_limits<double>::max());

    for (int m=1; m<argc; m++) { // iterate through all input objects
        Model model(argv[m]);
        for (int i=0; i<model.nfaces(); i++) { // iterate through all triangles
            vec4 clip[3];
            for (int d : {0,1,2}) {            // assemble the primitive
                vec3 v = model.vert(i, d);
                clip[d] = Perspective * ModelView * vec4{v.x, v.y, v.z, 1.};
            }
            TGAColor rnd;
            for (int c=0; c<3; c++) rnd[c] = std::rand()%255;
            rasterize(clip, zbuffer, framebuffer, rnd); // rasterize the primitive
        }
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
```

In line 7 I have declared three global \(4\times 4\) matrices, they are initialized in lines 59-61.
In line 72 I compute clip coordinates for every vertex of the scene, the perspective division is performed in line 26, and finally the screen coordinates are computed in line 27.
The rest of the code follows pretty much the same pattern as in the last chapter.

> 第 7 行声明了三个全局的 \(4\times 4\) 矩阵，它们在第 59–61 行被初始化。第 72 行计算场景中每个顶点的裁剪坐标，透视除法在第 26 行完成，最后屏幕坐标在第 27 行算出。代码的其余部分和上一章的模式基本相同。

Here is the image produced:

> 这是渲染出的图像：

![Image 9](https://haqr.eu/tinyrenderer/camera/framebuffer.png)

## Next time（下回预告）

This chapter was dense — take your time to digest it. The hardest part is behind us. Next, we’ll start discussing shading.

> 这一章信息量很大——慢慢消化。最难的坎已经迈过去了。接下来，我们要开始讨论着色了。

Stay tuned!

> 敬请期待！

> 原文：[Lesson 6: Better camera handling](https://haqr.eu/tinyrenderer/camera/)
