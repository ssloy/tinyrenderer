# Shadow mapping（阴影贴图）

## Introduction（引言）

Let us continue talking about lights and shadows.
In real-time rendering, the Phong reflection model provides a simple yet effective way to simulate how light interacts with surfaces.
Its strength lies in being entirely local: the color at a point depends only on the material properties, the light direction, and the viewing direction.
This locality makes the model extremely cheap to compute, which is why it has been the workhorse of interactive graphics for decades.

> 我们继续聊聊光照和阴影。在实时渲染中，Phong 反射模型提供了一种简单而有效的方式，用来模拟光线与表面的相互作用。它的长处在于完全局部化：某一点的颜色只取决于材质属性、光照方向和观察方向。这种局部性让模型的计算开销低得惊人，正因如此，它几十年来一直是交互式图形学的顶梁柱。

However, this same locality is also a limitation.
The Phong model cannot account for global effects such as one object blocking light from reaching another.
As a result, casted shadows — essential for realism and spatial perception — are completely absent.
To address this, additional techniques are needed.
One of the most widely used approaches in real-time graphics is shadow mapping, which extends local shading with a global visibility test to determine whether a point lies in light or shadow.

> 然而，这种局部性同时也是一种局限。Phong 模型无法考虑全局效果，比如一个物体挡住了本该照到另一个物体上的光。结果就是，投射阴影——它对真实感和空间感知至关重要——完全缺席。要应对这一点，就需要额外的技术。实时图形学中最广泛使用的方法之一就是阴影贴图：它用一个全局可见性测试来扩展局部着色，判断某个点究竟身处光照还是阴影之中。

In this chapter, we want to go from the image on the left to the one on the right:

> 在这一章里，我们要从左图出发，做出右图的效果：

![Image 1](https://haqr.eu/teaser.jpg)

## A practical recipe（实用方案）

Shadow mapping is built on the idea of using the __depth buffer twice__, in two separate rendering passes:

> 阴影贴图建立在一个朴素的想法之上：把深度缓冲用两次，并且分两个独立的渲染 Pass 进行：

1. __First pass (from the light’s perspective):__
   The scene is rendered from the light’s point of view. The depth buffer stores the distance of the closest surfaces to the light.
   This buffer (the shadow map) encodes which parts of the scene are visible to the light source.
2. __Second pass (from the camera’s perspective):__
   The scene is rendered normally from the viewer’s position, but for each fragment we perform an extra check: we test whether the fragment is visible to the light or hidden in shadow.

> 1. **第一个 Pass（从光源的视角出发）：** 场景从光源的位置渲染，深度缓冲里存的是离光源最近的表面的距离。这个缓冲区（即阴影贴图）记录了场景中哪些部分对光源可见。
> 2. **第二个 Pass（从相机的视角出发）：** 场景照常从观察者的位置渲染，但对每个片元我们多做一次检查：看这个片元是对光源可见，还是躲在阴影里。

Here is a schematic view of the two spaces involved:

> 下图是所涉及的两个空间的示意：

![Image 2](https://haqr.eu/spaces.jpg)

To link the two passes, we keep track of both `Viewport * Perspective * ModelView` products. This allows us to transform any fragment seen from the camera back into the light’s screen space.

> 为了把两个 Pass 串联起来，我们需要同时记录两个 `Viewport * Perspective * ModelView` 乘积（相机和光源各一份）。这样就能把从相机看到的任意片元，反向变换回光源的屏幕空间。

Let us say we have a fragment \(f = (i, j, z, 1)\) in the camera’s screen-space coordinates.
We can transform it back to object coordinates using \(M^{-1} f\), and then into the light’s screen-space coordinates using \(N M^{-1} f\), where \(M\) is the camera transform and \(N\) is the light transform.
Let us say that \(N M^{-1} f = (w i', w j', w z', w)\).
This gives us the coordinates of the fragment as the light “sees” it.
We then compare the fragment’s depth \(z'\) in that space with the value stored in the shadow map at the coordinates \((i', j')\):

> 假设我们有一个位于相机屏幕空间坐标中的片元 \(f = (i, j, z, 1)\)。可以用 \(M^{-1} f\) 把它变换回物体坐标，再用 \(N M^{-1} f\) 变换到光源的屏幕空间坐标，其中 \(M\) 是相机的变换，\(N\) 是光源的变换。设 \(N M^{-1} f = (w i', w j', w z', w)\)。这样就得到了该片元在光源“眼中”的坐标。接下来，我们把片元在该空间中的深度 \(z'\) 与阴影贴图在坐标 \((i', j')\) 处存储的值做比较：

- If the fragment’s depth __matches__ the stored depth, the fragment is visible to the light and should be lit.
- If the fragment’s depth is __greater__, it means another surface was closer to the light, and the fragment lies in shadow.

> - 如果片元的深度与存储的深度**相等**，说明该片元对光源可见，应当被照亮。
> - 如果片元的深度**更大**，说明有另一个表面离光源更近，该片元就处在阴影之中。

By doing this for every fragment, we obtain a binary mask of hard shadows. In this way, shadow mapping introduces a __global visibility test__ into the otherwise purely local Phong shading model.

> 对每个片元都这样处理一遍，我们就得到了一张硬阴影的二值掩码。就这样，阴影贴图把一个**全局可见性测试**引入了原本纯粹局部的 Phong 着色模型之中。

![Image 3](https://haqr.eu/sum.jpg)

## Homework（作业）

Implement the shadow mapping technique.
A few things to note:

> 实现阴影贴图技术。有几点需要注意：

- One of the shaders can be minimal or even blank: in the first pass we only need the depth buffer, so colors can be discarded.
- Shadows can either be applied as a post-processing pass over the framebuffer, or computed directly inside the main shader — both approaches are valid.
- __Z-fighting caveat:__
  When comparing the fragment’s depth with the value stored in the shadow map, numerical precision issues can occur. Two depths that are mathematically equal may differ slightly due to floating-point rounding and rasterization rules. As a result, some pixels that should be lit may be incorrectly classified as shadowed, producing flickering or dark streaks along shadow edges.

  This is known as z-fighting. The usual solution is to __apply a small depth bias__ when storing or comparing depths. In practice, this means slightly offsetting the depth values in the shadow map so that the surface being lit is not considered to block itself. Graphics APIs often provide built-in options such as _polygon offset_ (OpenGL) or _depth bias_ (DirectX) for this purpose.

> - 其中一个着色器可以做到最简，甚至完全留空：第一个 Pass 里我们只需要深度缓冲，颜色可以直接丢弃。
> - 阴影既可以作为帧缓冲上的后处理 Pass 来应用，也可以直接在主着色器内部计算——两种做法都可行。
> - **z-fighting 注意事项：**
>   在比较片元深度与阴影贴图中存储的值时，可能出现数值精度问题。两个在数学上相等的深度，可能因为浮点舍入和光栅化规则而产生细微差异。结果就是，一些本应被照亮的像素被错误地判为处于阴影中，在阴影边缘产生闪烁或者暗色条纹。
>
>   这就是所谓的 z-fighting（深度冲突）。通常的解决办法是，在存储或比较深度时**施加一个小的深度偏移（depth bias）**。实际做法就是把阴影贴图中的深度值稍微偏移一点，让正在被照亮的表面不会被判定为遮挡了它自己。图形 API 通常会为此提供内置选项，比如 OpenGL 的 _polygon offset_（多边形偏移）或 DirectX 的 _depth bias_（深度偏移）。

As usual, my code is available for reference.
Next time we will talk about ambient occlusion.

> 和往常一样，我的代码可供参考。下次我们聊聊环境光遮蔽。

> 原文：[Lesson 10: Shadow mapping](https://haqr.eu/tinyrenderer/shadow/)
