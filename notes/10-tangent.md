# Tangent space normal mapping（切线空间法线贴图）

## Global vs. tangent space（全局空间 vs. 切线空间）

Let us talk more about normal mapping.
It is a technique in computer graphics where a texture (the normal map) stores per-pixel normals, allowing fine surface detail to be simulated without increasing mesh complexity.

> 我们再深入聊聊法线贴图。这是计算机图形学中的一种技术：用一张纹理（即法线贴图）来存储逐像素的法线，从而在不增加网格复杂度的前提下模拟出精细的表面细节。

Previously, we used files with the `_nm.tga` suffix, whereas the repository also contains textures with the `_nm_tangent.tga` suffix.
The first one is a global-space normal map, while the latter is a tangent-space normal map.
Here they are side by side for the Diablo model:

> 之前我们用的文件带 `_nm.tga` 后缀，而仓库里还有带 `_nm_tangent.tga` 后缀的纹理。前者是全局空间法线贴图，后者是切线空间法线贴图。下面把它们在 Diablo 模型上并排放在一起：

![Image 1](https://haqr.eu/diablo-nm.jpg)

Last time we used the texture on the left, where each RGB value directly encodes a world-space normal.
There is a problem, though:
if an artist reuses UVs (rotates, mirrors, or overlaps the flattened mesh so that multiple surface parts have the same texels), then different 3D patches will generally require different global normals at the same texel.
One color cannot represent “two different directions at once,” so the result is wrong unless you forbid overlaps/rotations and enforce a unique, non-mirrored UV unwrap — very restrictive and memory-hungry.

> 上次我们用的是左边那张纹理，其中每个 RGB 值直接编码一个世界空间法线。不过这带来一个问题：如果美术师复用了 UV（把展平后的网格旋转、镜像或重叠，让多个表面部位共用同一批纹素），那么不同的 3D 面片在同一纹素处通常需要不同的全局法线。一种颜色无法同时表示“两个不同的方向”，所以除非禁止重叠/旋转，并强制采用唯一且非镜像的 UV 展开，否则结果就是错的——而这限制很大，还非常吃内存。

This problem arises in the Diablo model. For example, its arms, legs, and even both halves of the tail are superposed in the texture map.
Here is an illustration:

> Diablo 模型就碰到了这个问题。比如它的手臂、大腿，甚至尾巴的两半，在纹理贴图里都是叠在一起的。来看一张示意图：

![Image 2](https://haqr.eu/diablo-tangent.jpg)

Check the left image: the light comes from the right, so the right side of Diablo's hip is badly shaded — something we need to fix.
The solution is to use the tangent-space normal map.
Here, each texel encodes a normal __relative to the local basis__ that moves along the surface.

> 看左边那张图：光从右侧照来，Diablo 臀部右侧的着色很不对劲——这正是我们需要修复的地方。解决方案就是使用切线空间法线贴图。在这种贴图里，每个纹素编码的法线都是__相对于随表面移动的局部基底__的。

To illustrate what I mean by the local basis, I mapped a regular grid to the model:

> 为了说明我所说的局部基底是什么，我把一张规则网格映射到了模型上：

![Image 3](https://haqr.eu/head-tbn.jpg)

In addition, I have drawn smooth normals in green. At each point of the surface, we can define a __local, per-fragment basis__ made of the tangent, bitangent, and normal vectors (aligned with the red, blue, and green lines of the grid).
Because normals stored in tangent-space textures are relative to this moving frame, the same texel works correctly even for superposed or mirrored patches.

> 此外，我还用绿色画出了平滑法线。在表面的每个点上，我们都可以定义一个由切线（tangent）、副切线（bitangent）和法线向量构成的__局部的、逐片元的基底__（分别与网格的红线、蓝线和绿线对齐）。因为切线空间纹理中存储的法线是相对于这个移动标架的，所以即使是重叠或镜像的面片，同一个纹素也能正确工作。

In short, global-space normal maps tie each texel to a single fixed direction, which breaks when UVs are reused.
Tangent-space normal maps are expressed in a local frame that follows the surface, so the same texture region can drive multiple differently oriented surface parts correctly.

> 简而言之，全局空间法线贴图把每个纹素绑定到一个固定方向，一旦 UV 被复用就会失效。而切线空间法线贴图是在跟随表面的局部标架中表示的，所以同一块纹理区域可以正确地驱动多个朝向不同的表面部位。

Here's a quick question for you: why are tangent-space normal textures dominantly blue?

> 这里有个小问题考考你：为什么切线空间法线纹理大多以蓝色为主？

Spoiler Alert!

> 剧透警告！

Imagine a normal map for a perfect sphere.

> 想象一下一个完美球体的法线贴图。

- In global space, normals span every possible orientation, so the texture looks like a rainbow of directions.
- In tangent space, however, the normal is always the same vector \((0,0,1)\), which corresponds to a bluish color.
  The variation in shading comes from the moving local basis, not from the texel colors themselves.

> - 在全局空间中，法线涵盖所有可能的朝向，所以纹理看起来像一道由各种方向组成的彩虹。
> - 而在切线空间中，法线始终是同一个向量 \((0,0,1)\)，对应偏蓝的颜色。着色的变化来自不断移动的局部标架，而不是纹素颜色本身。

## Computing the tangent frame（计算切线标架）

As we saw, for each fragment we need to compute a basis whose tangent and bitangent vectors are aligned with the red and blue lines of the UV grid.
Consider a triangle with 3D vertices \(P_0, P_1, P_2\) in global space and corresponding 2D UV coordinates \(U_0, U_1, U_2\).

> 如我们所见，对每个片元，我们都需要计算一个基底，使它的切线向量和副切线向量分别与 UV 网格的红线和蓝线对齐。考虑一个三角形：它在全局空间中的 3D 顶点为 \(P_0, P_1, P_2\)，对应的 2D UV 坐标为 \(U_0, U_1, U_2\)。

Define the edge vectors:

> 定义边向量：

\[
\begin{array}{ll}
\vec{e_0} & := P_1 - P_0 \\ %\begin{pmatrix}P_{1x} - P_{0x} \\P_{1y} - P_{0y} \\P_{1z} - P_{0z} \\ \end{pmatrix}
\vec{e_1} & := P_2 - P_0 \\
\vec{u_0} & := U_1 - U_0 \\
\vec{u_1} & := U_2 - U_0
\end{array}
\]

Then the UV mapping can be described by a \(2 \times 3\) matrix \(M\):

> 那么，UV 映射可以用一个 \(2 \times 3\) 矩阵 \(M\) 来描述：

\[
M \times \underbrace{\begin{pmatrix} \vec{e_0} & \vec{e_1} \end{pmatrix}}_{3\times 2~\text{matrix}} = \underbrace{\begin{pmatrix} \vec{u_0} & \vec{u_1} \end{pmatrix}}_{2\times 2~\text{matrix}}
\]

Here is an illustration:

> 下图是一张示意图：

![Image 4](https://haqr.eu/tbn.svg)

Let us call the \(3\times 2\) matrix \(E\) and the \(2\times 2\) matrix \(U\):

> 我们把这个 \(3\times 2\) 矩阵记作 \(E\)，把 \(2\times 2\) 矩阵记作 \(U\)：

\[
\begin{array}{ll}
E &:= \begin{pmatrix} \vec{e_0} & \vec{e_1} \end{pmatrix}\\
U &:= \begin{pmatrix} \vec{u_0} & \vec{u_1} \end{pmatrix}
\end{array}
\]

So we have \(M \times E = U\).
We want to find the tangent vector \(\vec{t}\) and the bitangent vector \(\vec{b}\) such that they map to the unit UV axes:

> 于是有 \(M \times E = U\)。我们想找到切线向量 \(\vec{t}\) 和副切线向量 \(\vec{b}\)，使它们映射到单位 UV 坐标轴：

\[
\begin{array}{ll}
M \vec t & = \begin{pmatrix}1 \\ 0 \end{pmatrix}\\
M \vec b & = \begin{pmatrix}0 \\ 1 \end{pmatrix}
\end{array}
\]

Or, equivalently, \(M \times \begin{pmatrix}\vec t & \vec b\end{pmatrix} = \begin{pmatrix}1 & 0 \\ 0 & 1 \end{pmatrix}\).
Recall that \(M\times E = U\), therefore, \(M\times E \times U^{-1} = \begin{pmatrix}1 & 0 \\ 0 & 1 \end{pmatrix}\).

> 或者等价地，\(M \times \begin{pmatrix}\vec t & \vec b\end{pmatrix} = \begin{pmatrix}1 & 0 \\ 0 & 1 \end{pmatrix}\)。回忆一下 \(M\times E = U\)，因此 \(M\times E \times U^{-1} = \begin{pmatrix}1 & 0 \\ 0 & 1 \end{pmatrix}\)。

From this, we conclude:

> 由此我们得出：

\[
\begin{array}{ll}
\begin{pmatrix}\vec t & \vec b\end{pmatrix}  =  E \times U^{-1}.
\end{array}
\]

Finally, add the interpolated normal \(\vec{n}\), and you have constructed the full tangent-space basis \(\begin{pmatrix}\vec t & \vec b & \vec n\end{pmatrix}\).

> 最后，再加上插值得到的法线 \(\vec{n}\)，就构造出了完整的切线空间基底 \(\begin{pmatrix}\vec t & \vec b & \vec n\end{pmatrix}\)。

## Homework（作业）

Implement tangent-space normal mapping.
As usual, I provide [my code](https://github.com/ssloy/tinyrenderer/commit/32a4faac0a56498e78808a14840a9114440cb194) for reference.

> 实现切线空间法线贴图。和往常一样，我提供[我的代码](https://github.com/ssloy/tinyrenderer/commit/32a4faac0a56498e78808a14840a9114440cb194)供参考。

---

Next time we will talk about global illumination.

> 下次我们聊聊全局光照。

> 原文：[Lesson 9: Tangent space normal mapping](https://haqr.eu/tinyrenderer/tangent/)
