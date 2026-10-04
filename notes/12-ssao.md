# Indirect lighting（间接光照）

## Introduction（引言）

When light interacts with a scene, not all of it travels directly from a light source to a surface.
Some of it bounces off other surfaces before arriving at the point we observe.
This bounced light is called indirect lighting.

> 当光照亮一个场景时，并不是所有的光都从光源直接到达物体表面：有些光要先在其他表面上反弹几次，才抵达我们注视的那个点。这种被反弹过的光，就叫间接光照。

For example, if you place a red ball on a white floor, the floor will pick up a faint reddish tint. That happens because light first hits the red ball, reflects off it, and then illuminates the floor:

> 举个例子，如果把一只红球放在白色地板上，地板会染上一层淡淡的红色。这是因为光先照到红球，从红球上反射出去，然后才照亮地板：

![Image 1](https://haqr.eu/ball.jpg)

Another example is a room lit by the sun through a window: even corners not directly exposed to the sunlight are visible. They receive light indirectly from the walls, floor, and ceiling.

> 再举一个例子：阳光透过窗户照进房间，即使是没有被阳光直接照到的角落也能看见——它们在接收来自墙壁、地板和天花板的间接光。

Indirect lighting is essential for realism because it accounts for how surfaces exchange light with each other. Without it, images look flat and artificial: shadows are completely black, and there’s no color bleeding between objects.

> 间接光照对真实感至关重要，因为它体现了表面之间如何互相交换光线。没有它，画面会显得扁平而虚假：阴影是一片死黑，物体之间也没有颜色渗透。

## Ambient occlusion（环境光遮蔽）

In the classical Phong reflection model, the ambient term is often represented as a simple **constant** added to the shading equation.
This constant acts as a crude approximation of indirect lighting: it prevents regions not directly lit by any source from appearing completely black.
While effective for real-time rendering, this approach lacks realism because it does not account for the actual geometry of the scene and how much of the surrounding light can reach a given point.

> 在经典的 Phong 反射模型中，环境光项往往只是简单地往着色方程里加一个**常数**。这个常数是对间接光照的粗略近似：它保证没有被任何光源直接照到的区域不至于一片漆黑。这种做法对实时渲染而言很管用，但缺乏真实感，因为它既不考虑场景的实际几何形状，也不考虑给定点究竟能接收到周围多少光。

![Image 2](https://haqr.eu/teaser.jpg)

A more physically motivated idea is to consider **global illumination**, where light is allowed to bounce and scatter between surfaces.
This approach is computationally expensive, though. One practical approximation of this idea is **ambient occlusion (AO)**.

> 一个更有物理依据的思路是考虑**全局光照**，让光在表面之间反复反弹、散射。不过这种方法的计算开销很大。对它的一个实用近似，就是**环境光遮蔽（AO）**。

Instead of assuming a uniform ambient term everywhere, we can approximate indirect lighting by extending the concept of shadow mapping.
In standard shadow mapping, we determine whether a point is visible from a light source, producing a binary "in shadow" or "lit" decision.

> 与其假定处处都存在一个均匀的环境光项，我们可以扩展阴影贴图的概念来近似间接光照。标准阴影贴图判断的是一个点能否被光源看到，产生的是非黑即白的结果：“在阴影里”还是“被照亮”。

For ambient occlusion, we could imagine rendering many such shadow maps from different directions across the hemisphere around the object, each one giving us a hard shadow.
By averaging the results of many such "virtual shadow tests," we obtain a continuous measure of how occluded a point is by its surroundings.
Here I show 1000 camera positions I have used to obtain the image:

> 对于环境光遮蔽，我们可以想象在物体周围半球的不同方向上渲染许多张这样的阴影贴图，每一张都给出一个硬阴影。把许多次这样的“虚拟阴影测试”的结果平均起来，就得到一种连续的度量，衡量一个点被周围环境遮挡的程度。下面展示了我生成图像时使用的 1000 个相机位置：

![Image 3](https://haqr.eu/samples.jpg)

This technique transforms the idea of a constant ambient term into a geometry-aware shading effect, bridging the gap between Phong’s local model and global illumination.
Points in narrow crevices or under overhangs appear darker, while exposed points on open surfaces remain brighter.
This creates the perception of depth and realism, even without fully simulating indirect light transport.

> 这项技术把“环境光只是一个常数”的想法，变成了一种感知几何形状的着色效果，在 Phong 的局部模型与全局光照之间架起了桥梁：狭窄缝隙里、悬挑结构下方的点显得更暗，而开阔表面上外露的点则保持明亮。即使没有完整模拟间接光传输，也能产生纵深感与真实感。

## Homework 1（作业 1）

Implement this brute-force approach of ambient occlusion computation.
If you have already implemented shadow mapping, this one should be straightforward.

> 请实现这种暴力计算环境光遮蔽的方法。如果你已经实现过阴影贴图，这一步应该很好上手。

As always, my implementation [is available here](https://github.com/ssloy/tinyrenderer/tree/974424b09ee46fe50a89b9dadaa743942f2b4adc).

> 和往常一样，我的实现[在这里](https://github.com/ssloy/tinyrenderer/tree/974424b09ee46fe50a89b9dadaa743942f2b4adc)。

## Screen-space ambient occlusion（屏幕空间环境光遮蔽）

The idea of computing ambient occlusion with many shadow maps is conceptually straightforward: render the scene from many different directions, check which points are visible and which are blocked, and then average the results.
This captures how much of the surrounding hemisphere is occluded by geometry.

> 用许多阴影贴图来计算环境光遮蔽，思路很直白：从许多不同方向渲染场景，检查哪些点可见、哪些点被挡住，然后把结果平均。这样就能测出周围半球被几何体遮住了多大一部分。

While this method can produce very realistic results, it requires a large number of shadow map renders per frame, which makes it far too expensive for real-time applications such as games.
To overcome this, graphics researchers developed Screen-Space Ambient Occlusion (SSAO).
Instead of sampling the entire 3D scene from many directions, SSAO estimates occlusion using only the information already available in the **screen space**, namely the **depth buffer** (distance from the camera for each pixel) and sometimes the **normal buffer** (surface orientation).

> 这种方法能产生非常真实的结果，但每一帧都要渲染大量阴影贴图，对游戏之类的实时应用来说实在太昂贵了。为了解决这个问题，图形学研究者提出了屏幕空间环境光遮蔽（SSAO）。SSAO 不再从许多方向对整个 3D 场景采样，而是仅利用**屏幕空间**中已有的信息估计遮挡，也就是**深度缓冲**（每个像素到相机的距离），有时还会用到**法线缓冲**（表面朝向）。

The basic idea is:

> 基本思路是：

- For each fragment, randomly sample a few points around it.
- Compare the sampled points’ depths with the depth buffer.
  If a sample point is behind existing geometry (occluded), it contributes to the ambient shadowing of that pixel.
- Average over the samples to estimate how occluded the fragment is.

> - 对每个片元（fragment），在它周围随机采样几个点。
> - 把采样点的深度与深度缓冲做比较。如果某个采样点位于已有几何体之后（被遮挡），它就会为该像素的环境光阴影做出贡献。
> - 对所有采样取平均，估计出该片元被遮挡的程度。

Here is an illustration:

> 来看一张示意图：

![Image 4](https://haqr.eu/ssao.jpg)

In this example, I consider the fragment shown in red, and generate 128 samples around it.
Those in blue are occluded by the surface; the ratio of green points to 128 is my estimate of indirect lighting :)

> 在这个例子里，我取图中标红的那个片元，在它周围生成 128 个采样点。蓝色的点被表面遮挡了；绿点的数量与 128 的比值，就是我对间接光照的估计 :)

This approach is much faster than averaging over many shadow maps, because it reuses data that is already computed for rendering.
In my example we only use the z-buffer to compute the indirect lighting, so the speed does not depend on the complexity of the geometry, it only depends on the screen resolution.

> 这种方法比在大量阴影贴图上取平均快得多，因为它复用了渲染时已经算好的数据。在我的例子里只用 z 缓冲来计算间接光照，所以速度不取决于几何体的复杂度，只取决于屏幕分辨率。

SSAO is only an approximation, as it cannot “see” geometry that is off-screen, and may introduce artifacts.
Nevertheless, it provides a convincing sense of contact shadows and depth perception at interactive frame rates.
That’s why SSAO and its refinements are widely used in modern real-time rendering pipelines.

> SSAO 只是一种近似：它“看不到”屏幕之外的几何体，还可能引入伪影。尽管如此，它能在交互式帧率下给出令人信服的接触阴影和纵深感。正因如此，SSAO 及其各种改进方案被广泛用于现代实时渲染管线。

## Homework 2（作业 2）

Implement a SSAO technique.
There are many possible variants.

> 请实现一种 SSAO 技术。可能的变体有很多。

For example, for each fragment, we can consider the hemisphere oriented around the surface normal, and generate sample points within or on this hemisphere.
Different kinds of artifacts can appear: for instance, in the image above, fragments near the screen boundary are somewhat lighter than those farther from the boundary.
The image is also noisy — how could we improve it?

> 例如，对每个片元，可以取以表面法线为轴的半球，在这个半球之内或表面上生成采样点。不同类型的伪影都可能出现：比如上图中，靠近屏幕边界的片元比远离边界的片元略亮一些。图像也会带有噪点——怎样才能改进它呢？

There is no single "right" answer. Invent an approach, implement it, improve it. Rinse, repeat.

> 没有唯一的“正确”答案。想一个办法，实现它，改进它。如此反复。

My implementation is [available here](https://github.com/ssloy/tinyrenderer/tree/ssao).

> 我的实现[在这里](https://github.com/ssloy/tinyrenderer/tree/ssao)。

> 原文：[Lesson 11: Indirect lighting](https://haqr.eu/tinyrenderer/ssao/)
