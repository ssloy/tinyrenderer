# Bonus: toon shading（附赠：卡通着色）

## Introduction（引言）

In most of computer graphics, the ultimate goal is realism: simulating light and materials in a way that makes synthetic images indistinguishable from photographs.
This pursuit has given us advanced rendering techniques, global illumination models, and physically based shading pipelines.

> 在计算机图形学的大部分领域，终极目标都是写实：模拟光线与材质，让合成图像逼真到与照片难以分辨。这种追求为我们带来了各种高级渲染技术、全局光照模型，以及基于物理的着色管线。

But realism is not always the end of the story.
Sometimes we want our images to be more expressive, more stylized, or simply more fun.
This is where non-photorealistic rendering (NPR) comes in.
Instead of imitating the real world, NPR techniques borrow inspiration from art, comics, or illustration, emphasizing certain visual qualities while ignoring others.

> 但写实并不总是故事的终点。有时候，我们希望图像更有表现力、更有风格，或者单纯更好玩。这正是非真实感渲染（non-photorealistic rendering，NPR）的用武之地。NPR 技术不去模仿真实世界，而是从绘画、漫画或插画中汲取灵感，强调某些视觉特质，同时忽略另一些。

Toon shading (sometimes called cel shading) is one of the most iconic NPR methods.
It gives 3D models a flat, cartoon-like look, replacing smooth gradients with sharp color regions and emphasizing contours with bold outlines.
The result is an aesthetic closer to hand-drawn animation than to photographic realism.

> 卡通着色（toon shading，有时也叫 cel shading）是最具代表性的 NPR 手法之一。它让 3D 模型呈现出扁平的卡通外观：用清晰的色块取代平滑的渐变，用粗重的轮廓线强化外形。最终效果更接近手绘动画，而非照片般的写实。

![Image 1](https://haqr.eu/toon.png)

## The idea（思路）

The essence of toon shading is to take the familiar tools of smooth shading and deliberately reduce their precision. Instead of aiming for subtle gradients and continuous variations of light, we quantize the result into a handful of discrete tones.

> 卡通着色的本质，是拿起平滑着色中那些熟悉的工具，刻意降低它们的精度。我们不再追求微妙的光影渐变和连续的光照变化，而是把结果量化成少数几档离散的色调。

A simple approach works like this:

> 一个简单的做法是这样的：

- __Start with standard lighting.__
  Compute the diffuse lighting just as you would in a traditional Phong or Lambert shading model. At this stage, each point on the surface has a smoothly varying brightness.

  ![Image 2](https://haqr.eu/smooth.png)
- __Quantize the intensities.__
  Instead of displaying the raw lighting value, clamp it into a few distinct levels — say three or four. This replaces the soft gradient of light with sharp “bands” of brightness, giving the object its characteristic flat, cartoon-like look.

  ![Image 3](https://haqr.eu/quantization.png)

  In my example the quantization is as simple as that:

  ```
      if (intensity>.66) intensity = 1;
      else if (intensity>.33) intensity = .66;
      else intensity = .33;
  ```
- __Add outlines.__
  To reinforce the drawing-like style, we can render black edges around the object.
  A simple technique is to detect silhouettes in the z-buffer: wherever the depth changes abruptly, we draw a dark contour line.
  This gives the impression of inked outlines, similar to what you see in traditional animation or comics.

  ![Image 4](https://haqr.eu/sobel.png)

> - **先用标准光照。**
>   像传统的 Phong 或 Lambert 着色模型那样计算漫反射光照。此时，表面上每个点的亮度都是平滑变化的。
> - **对光强进行量化。**
>   不要直接显示原始的光照值，而是把它压到几个离散档位上——比如说三档或四档。这样，柔和的光照渐变就被替换为锐利的亮度“色带”，物体也就有了标志性的扁平卡通外观。
>
>   在我的例子里，量化就是这么简单：
> - **加上轮廓线。**
>   为了强化手绘般的效果，我们可以在物体周围渲染出黑色的边缘。
>   一个简单的技巧是在深度缓冲（z 缓冲）中检测轮廓：凡是深度发生剧烈变化的地方，就画一条深色的轮廓线。
>   这就会带来墨线勾边的观感，类似你在传统动画或漫画里看到的效果。

The combination of banded shading and dark silhouettes already produces a convincing toon effect. More advanced variants exist (e.g., controlling band placement, adding hatching textures, or stylizing colors), but this threshold-plus-outline approach captures the core idea in a way that is simple, effective, and fun to implement.

> 色带着色与深色轮廓的组合，已经能产生令人信服的卡通效果。更高级的变体当然存在（比如控制色带的位置、加入排线纹理，或者对颜色做风格化处理），但这种“阈值 + 轮廓”的做法，以简单、有效且实现起来充满乐趣的方式抓住了核心思想。

## Sobel edge detection（Sobel 边缘检测）

This is the final chapter of the course. By now you are fully prepared to compute lighting, but you may still need a bit of help when it comes to detecting edges.
Let us start with a simple 1D example.

> 这是本课程的最后一章。到现在为止，计算光照你已经完全没问题了，但在边缘检测这件事上，你可能还需要一点帮助。让我们从一个简单的一维例子开始。

### 1D edge detection（一维边缘检测）

Think of a row of pixel intensities taken from a z-buffer image.
In 1D, edge detection is about finding abrupt changes in intensity values. The simplest idea is:

> 想象从一张 z 缓冲图像里取出的一行像素光强。在一维情形下，边缘检测就是要找出光强值的突变之处。最简单的想法是：

```
    for every pixel in the row:
        look at the left neighbor and the right neighbor
        subtract them
        if the result exceeds some threshold → mark as an edge
```

Here is a small example:

> 这里有一个小例子：

1D Sobel edge detection

```python
import numpy as np
import matplotlib.pyplot as plt

x = np.linspace(0, 16, 100)
signal = np.sin(x/2) + (x > 10) # sinusoid + step

sobel = np.array([-1, 0, 1]) # 1D Sobel kernel
edges = np.zeros_like(signal)
for i in range(1, len(signal)-1):
    window = signal[i-1:i+2]    # take [left, center, right]
    edges[i] = np.sum(window * sobel)

plt.figure(figsize=(10,3))
plt.step(x, signal, label="Pixel intensities")
plt.step(x, edges, label="Edge response")
plt.title("1D Sobel edge detection")
plt.legend()
plt.show()
```

![Image 5](https://haqr.eu/sobel1d.png)

In blue, you see a row of 16 pixels sampled from a “z-buffer.” For illustration, it is just a sine wave with a sudden step in the middle.

> 蓝色曲线是从“z 缓冲”中采样的一行 16 个像素。为了方便演示，它其实就是一条正弦波，只不过中间有个突然的阶跃。

For each pixel we do the following:

> 对每个像素，我们做下面这件事：

- Multiply the left neighbor by –1
- Ignore the pixel itself
- Multiply the right neighbor by +1
- Add them up → a large value means a jump, a small value means flat

> - 把左邻居乘以 –1
> - 忽略像素本身
> - 把右邻居乘以 +1
> - 把它们加起来 → 值大意味着有跳变，值小意味着平坦

This operation is called a convolution, applied here with the Sobel kernel \([-1, 0, +1]\).
The orange curve shows the edge response: it clearly highlights an edge around pixel 10.

> 这个操作叫卷积（convolution），在这里用 Sobel 核 \([-1, 0, +1]\) 来完成。橙色曲线表示边缘响应：它清晰地标出了第 10 号像素附近的边缘。

### 2D example（二维示例）

In 2D images the idea is the same, except we now care about changes in both the \(x\) and \(y\) directions: edges can be horizontal, vertical, or diagonal.

> 在二维图像中，思路是一样的，只不过现在要同时考虑 \(x\) 和 \(y\) 两个方向的变化：边缘可以是水平的、垂直的，也可以是斜的。

Irwin Sobel proposed two small \(3 \times 3\) convolution kernels.

> Irwin Sobel 提出了两个小巧的 \(3 \times 3\) 卷积核。

One emphasizes __horizontal changes__ in intensity (detecting vertical edges):

> 一个强调光强的**水平变化**（用于检测垂直边缘）：

\[
K_x =
\begin{bmatrix}
-1& 0 &+1\\
-2& 0 &+2\\
-1& 0 &+1
\end{bmatrix}
\]

The other emphasizes __vertical changes__ (detecting horizontal edges):

> 另一个强调**垂直变化**（用于检测水平边缘）：

\[
K_y =
\begin{bmatrix}
-1& -2 &-1\\
0& 0 &0\\
+1& +2 &+1
\end{bmatrix}
\]

We compute two intermediate edge images by convolving the original image \(I\) with these kernels:

> 我们用这两个核与原图像 \(I\) 做卷积，得到两幅中间边缘图像：

\[
\begin{array}{ll}
G_x &= K_x * I\\
G_y &= K_y * I,
\end{array}
\]

where \(*\) denotes 2D convolution.
Then, we combine both results to obtain the final edge strength:

> 其中 \(*\) 表示二维卷积。然后，把两个结果合并，就得到最终的边缘强度：

\[
\text{Edge strength} = \sqrt{G_x^2 + G_y^2}.
\]

If this description feels too mathematical, here is a concrete example:

> 如果这段描述让你觉得太数学了，这里有一个具体的例子：

2D Sobel edge detection

```python
import numpy as np
import matplotlib.pyplot as plt

img = np.zeros((128, 128))
img[32:96, 32:96] = 1.0

# Sobel kernels
sobel_x = np.array([[-1,0,1],
                    [-2,0,2],
                    [-1,0,1]])
sobel_y = np.array([[-1,-2,-1],
                    [ 0, 0, 0],
                    [ 1, 2, 1]])

# Convolution by hand (no SciPy)
gx = np.zeros_like(img)
gy = np.zeros_like(img)

for i in range(1, img.shape[0]-1):
    for j in range(1, img.shape[1]-1):
        region = img[i-1:i+2, j-1:j+2]
        gx[i,j] = np.sum(region * sobel_x)
        gy[i,j] = np.sum(region * sobel_y)

edges = np.sqrt(gx**2 + gy**2)

plt.figure(figsize=(12,4))
plt.subplot(1,4,1); plt.imshow(img, cmap="gray"); plt.title("Original image")
plt.subplot(1,4,2); plt.imshow(gx, cmap="gray"); plt.title("Vertical edges")
plt.subplot(1,4,3); plt.imshow(gy, cmap="gray"); plt.title("Horizontal edges")
plt.subplot(1,4,4); plt.imshow(edges, cmap="gray"); plt.title("Combined edges")
plt.show()
```

![Image 6](https://haqr.eu/sobel2d.png)

The input is a simple \(128 \times 128\) image of a solid white square on a black background. The Sobel filter perfectly outlines the square, demonstrating how edges are extracted.

> 输入是一幅简单的 \(128 \times 128\) 图像：黑色背景上有一个纯白实心方块。Sobel 滤波器完美地勾出了方块的轮廓，展示了边缘是如何被提取出来的。

## Homework（作业）

Experiment with non-photorealistic rendering (NPR).
By stepping away from strict realism, we gain a deeper understanding of the visual pipeline and discover how even small changes in shading rules can produce strikingly different effects.
My implementation of a toon shader is available [here](https://github.com/ssloy/tinyrenderer/tree/toon).

> 动手实验非真实感渲染（NPR）。跳出严格的写实主义，我们能更深入地理解整个图形管线，并发现：着色规则中哪怕很小的改动，都能带来截然不同的效果。我实现的卡通着色器在[这里](https://github.com/ssloy/tinyrenderer/tree/toon)。

Exploring NPR is not only enjoyable, it also broadens our perspective as graphics practitioners, and it leaves plenty of room for creativity and invention.

> 探索 NPR 不仅有趣，还能拓宽我们作为图形从业者的视野，并且为创意与发明留下了充足的空间。

> 原文：[Lesson 12: Bonus: toon shading](https://haqr.eu/tinyrenderer/toon/)
