# Bresenham's line drawing algorithm（布雷森汉姆画线算法）

The goal for today is to create wireframe renders.
To achieve this, we first need to learn how to draw line segments on the screen.
I provide my students with the following code:

> 今天的目标是做出线框渲染。
> 为此，我们首先得学会在屏幕上画线段。
> 我给我的学生们提供了下面这段代码：

The starting point

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

This code generates a 64×64 image named `framebuffer.tga`.
Here's a scaled-up version for better readability:

> 这段代码会生成一张名为 `framebuffer.tga` 的 64×64 图像。
> 为了看得更清楚，下面放一张放大后的版本：

![Image 3](https://haqr.eu/tinyrenderer/bresenham/bresenham0.png)

Our first objective is to draw three line segments forming a triangle.
At this stage, the only available functionality is setting the color of individual pixels and saving an image — there are no built-in functions for drawing lines.
We will implement it ourselves.

> 我们的第一个目标，是画出组成一个三角形的三条线段。
> 现阶段我们仅有对单个像素上色和保存图像这两项功能——并没有内置的画线函数。
> 所以得自己动手实现。

While Wikipedia provides an explanation of Bresenham's line algorithm, I find it difficult to follow.
Instead, I prefer a more gradual approach.
Given two 2D points \((a_x, a_y)\) and \((b_x, b_y)\), what is the simplest program to draw a line segment between them?

> 虽然 Wikipedia 上有布雷森汉姆画线算法的解释，但我觉得很难看懂。
> 相比之下，我更喜欢循序渐进的思路。
> 给定两个 2D 点 \((a_x, a_y)\) 和 \((b_x, b_y)\)，要在它们之间画一条线段，最简单的程序会是什么样？

## First attempt, the simplest approach（第一次尝试：最朴素的做法）

Imagine we have a parameter \(t \in [0, 1]\).
We define the 2D point \((x(t), y(t))\) as follows:

> 想象我们有一个参数 \(t \in [0, 1]\)。
> 我们这样定义 2D 点 \((x(t), y(t))\)：

\[
\left\{
\begin{array}{l}
x(t)= a_x + t \cdot (b_x-a_x) \\
y(t)= a_y + t \cdot (b_y-a_y) \\
\end{array}
\right.
\]

If \(t = 0\), then \((x(t), y(t)) = (a_x, a_y)\).
If \(t = 1\), then \((x(t), y(t)) = (b_x, b_y)\).
For intermediate values of \(t\), the point moves along the segment.

> 当 \(t = 0\) 时，\((x(t), y(t)) = (a_x, a_y)\)。
> 当 \(t = 1\) 时，\((x(t), y(t)) = (b_x, b_y)\)。
> 而对于 \(t\) 的中间取值，这个点就沿着线段移动。

Barycentric coordinates

> 重心坐标

An important note for the next lesson: the pair \((1-t), t\) forms the barycentric coordinates of the number \(x(t)\) with respect to the interval \([a_x, b_x]\)
and of the number \(y(t)\) with respect to the interval \([a_y, b_y]\).
This is a bit offtopic for today, but for the next lesson it will be a thing to remember.
Why am I talking about the pair \((1-t), t\)?

> 给下一课留个重要提醒：这一对 \((1-t), t\) 构成了数 \(x(t)\) 关于区间 \([a_x, b_x]\) 的重心坐标，
> 同时也是数 \(y(t)\) 关于区间 \([a_y, b_y]\) 的重心坐标。
> 这对今天来说有点跑题，但下一课里就要记住它了。
> 我为什么要专门提这一对 \((1-t), t\) 呢？

It is because we can rewrite the parametric equation for the segment in the following form:
\(\left\{
\begin{array}{l}
x(t)= (1-t) \cdot a_x + t \cdot b_x \\
y(t)= (1-t) \cdot a_y + t \cdot b_y
\end{array}
\right.\)

> 因为我们可以把线段的参数方程改写成下面这种形式：
> \(\left\{
\begin{array}{l}
x(t)= (1-t) \cdot a_x + t \cdot b_x \\
y(t)= (1-t) \cdot a_y + t \cdot b_y
\end{array}
\right.\)

Imagine we have 1 kilo of balance weights \((1-t~\text{kg} + t~\text{kg} = 1~\text{kg})\). Then if we put \(1-t\) kg to the point \((a_x, a_y)\) and \(t\) kg to the point \((b_x, b_y)\), then the barycenter of the system will be in the point \((x(t), y(t))\),
hence the barycentric coordinates.

> 想象我们有 1 千克的配重 \((1-t~\text{kg} + t~\text{kg} = 1~\text{kg})\)。把 \(1-t\) 千克放到点 \((a_x, a_y)\) 上，把 \(t\) 千克放到点 \((b_x, b_y)\) 上，那么这个系统的重心就会落在点 \((x(t), y(t))\) 处，
> 所谓重心坐标就是这么来的。

Here's the first program implementing this approach:

> 下面是按这个思路实现的第一个程序：

First attempt

```cpp
#include <cmath>
#include "tgaimage.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    for (float t=0.; t<1.; t+=.02) {
        int x = std::round( ax + (bx-ax)*t );
        int y = std::round( ay + (by-ay)*t );
        framebuffer.set(x, y, color);
    }
}

int main(int argc, char** argv) {
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    int ax =  7, ay =  3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    line(ax, ay, bx, by, framebuffer, blue);
    line(cx, cy, bx, by, framebuffer, green);
    line(cx, cy, ax, ay, framebuffer, yellow);
    line(ax, ay, cx, cy, framebuffer, red);

    framebuffer.set(ax, ay, white);
    framebuffer.set(bx, by, white);
    framebuffer.set(cx, cy, white);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
```

In lines 10-16 I have programmed exactly the parametric equation we talked about: I made the parameter \(t\) vary between \(0\) and \(1\), and I have colored the corresponding pixel \((x(t), y(t))\).
Then I asked to draw __four__ segments between the points \(a\), \(b\) and \(c\):

> 第 10-16 行实现的正是我们刚讨论的那个参数方程：让参数 \(t\) 从 \(0\) 变化到 \(1\)，并把对应的像素 \((x(t), y(t))\) 涂上颜色。
> 接着我让它在点 \(a\)、\(b\)、\(c\) 之间画了__四条__线段：

- blue from \(a\) to \(b\)
- green from \(c\) to \(b\)
- yellow from \(c\) to \(a\)
- red from \(a\) to \(c\).

> - 蓝色，从 \(a\) 到 \(b\)
> - 绿色，从 \(c\) 到 \(b\)
> - 黄色，从 \(c\) 到 \(a\)
> - 红色，从 \(a\) 到 \(c\)。

Note that the last two have exactly the same endpoints, just passed to the `line(..)` function in different order.
Here is the resulting image:

> 注意，最后两条线段的端点完全相同，只是传给 `line(..)` 函数的顺序不一样。
> 下面是渲染结果：

![Image 4](https://haqr.eu/tinyrenderer/bresenham/bresenham1.png)

What problems can you spot with my `line` function? First of all, I see that the yellow line is visible from behind the red one, and I'd expect to see exactly the same pixels colored
independently of the order of the arguments.
Second, I see gaps in the red line.
It is to be expected: `ax = 7` and `cx = 62`, so we need __at least__ 62-7 = 55 pixels colored for the line to have no gaps, but the increment for `t` is only `.02`, thus we have 51 samples for each `line` call.
51 < 55, therefore gaps.
Is there a way to choose the sampling step more wisely?

> 你能看出我这个 `line` 函数有哪些问题吗？首先，黄色线条从红色线条背后露了出来，而我认为无论参数顺序如何，被上色的像素都应当完全一样才对。
> 其次，红色线条上有空隙。
> 这倒在意料之中：`ax = 7`、`cx = 62`，所以要让线条没有空隙，__至少__ 需要上色 62-7 = 55 个像素，但 `t` 的步长只有 `.02`，于是每次调用 `line` 只采样了 51 个点。
> 51 < 55，空隙就这么来了。
> 有没有办法更聪明地选择采样步长呢？

## Second attempt, different sampling strategy（第二次尝试：换一种采样策略）

Let us try the other way:

> 我们换个方向试试：

Second attempt, round 1

```cpp
void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    for (int x=ax; x<=bx; x++) {
        float t = (x-ax) / static_cast<float>(bx-ax);
        int y = std::round( ay + (by-ay)*t );
        framebuffer.set(x, y, color);
    }
}
```

Note that we defined \(x\) as a function of \(t\), but we can do it the other way, and define \(t(x) = (x-a_x)/(b_x-a_x)\), and then compute \(y(t)\) as before.
This allows me to make the loop over `x` instead of `t`, and this way the number of samples somewhat adapts to the length of the segment.
Caution! The the integer division, like `(x-ax)/(bx-ax)` may be a source of bugs. Here I cast the division to the floating point.
Here is the resulting image:

> 注意，之前我们把 \(x\) 定义为 \(t\) 的函数，但也可以反过来，定义 \(t(x) = (x-a_x)/(b_x-a_x)\)，然后像之前那样计算 \(y(t)\)。
> 这样我就能用 `x` 而不是 `t` 来写循环，采样点的数量也就大致能随线段的长度自适应了。
> 当心！像 `(x-ax)/(bx-ax)` 这样的整数除法很容易埋下 bug。这里我把除法转成了浮点运算。
> 下面是渲染结果：

![Image 5](https://haqr.eu/tinyrenderer/bresenham/bresenham2a.png)

This resolves some issues but introduces others: since `x` is incremented in the loop, the green line disappears (`cx`>`bx`).
The blue line has large gaps, since we'd rather need to iterate over `y` for steep line segments: again the number of samples is insufficient.
No biggie, let us first make the green line re-appear:

> 这解决了一些问题，却引入了新的问题：由于循环里递增的是 `x`，绿色线条消失了（`cx`>`bx`）。
> 蓝色线条出现了大片空隙，因为对于陡峭的线段，我们更应该对 `y` 迭代：采样点数量又不够了。
> 不过问题不大，先让绿色线条重新出现：

Second attempt, round 2

```cpp
void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    if (ax>bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    for (int x=ax; x<=bx; x++) {
        float t = (x-ax) / static_cast<float>(bx-ax);
        int y = std::round( ay + (by-ay)*t );
        framebuffer.set(x, y, color);
    }
}
```

I did not modify my `for` loop, but I swap the endpoints to make the segment left-to-right.
Therefore, the green line reappears. Note that the yellow line is not visible: now both calls to the yellow and red lines are identical, and therefore the red one overwrites the yellow perfectly.

> 我没有动 `for` 循环，只是交换了两个端点，让线段变成从左到右的方向。
> 于是绿色线条又出现了。注意此时黄色线条看不见了：现在对黄色和红色线条的两次调用完全等价，因此红色线条完美地覆盖了黄色。

![Image 6](https://haqr.eu/tinyrenderer/bresenham/bresenham2b.png)

Let us fix the remaining issue, namely, the insufficient sampling for steep line segments.

> 下面来解决剩下的问题，也就是陡峭线段采样点不足的问题。

## Third attempt, the bulletproof one（第三次尝试：万无一失的版本）

Recall how we swapped endpoints to ensure the segment was drawn from left to right?
We can apply the same trick to address the remaining issue.
If the line is steep, meaning it is more vertical than horizontal, we transpose the image.
This transformation makes the line more horizontal in the transposed space.
Then, after drawing, we de-transpose it back — voilà!

> 还记得我们是怎么交换端点、让线段从左向右画的吗？
> 同样的把戏可以用在剩下的问题上。
> 如果这条线是陡峭的，也就是竖直方向比水平方向延伸得更多，我们就把图像转置。
> 转置之后，线条在转置空间里就变得更为水平。
> 画完之后再转置回来——瞧，搞定！

Check the highlighted lines in the following code:

> 请看下面代码中高亮的那几行：

The bulletproof method

```cpp
void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    bool steep = std::abs(ax-bx) < std::abs(ay-by);
    if (steep) { // if the line is steep, we transpose the image
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax>bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    for (int x=ax; x<=bx; x++) {
        float t = (x-ax) / static_cast<float>(bx-ax);
        int y = std::round( ay + (by-ay)*t );
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
    }
}
```

The result is flawless:

> 结果完美无瑕：

![Image 7](https://haqr.eu/tinyrenderer/bresenham/bresenham3.png)

This code works pretty well.
This is exactly the level of complexity I want to see in the final version of our renderer.
While it is inefficient (due to floating-point operations, multiple divisions, and the like), it remains concise and readable.

> 这段代码运转得相当不错。
> 这正是我希望在最终版渲染器中看到的复杂度水平。
> 虽说它效率不高（浮点运算、多次除法等等），但仍然简洁易读。

Notably, the code lacks assertions and boundary checks, which is a drawback.
However, I avoid overloading the code in these articles to keep it accessible.
At the same time, I consistently emphasize the importance of implementing necessary checks.

> 值得注意的是，这段代码缺少断言和边界检查，这是个缺点。
> 不过在这些文章里，我会避免把代码写得太冗赘，以保证可读性。
> 与此同时，我一直都在强调实现必要检查的重要性。

## Fourth attempt: optimizations（第四次尝试：优化）

__Warning: This section is primarily included for historical and cultural context.__

> __警告：本节主要是出于历史和人文背景的考虑而收录的。__

While the previous code works well, there is still room for optimization.
However, optimization must be approached with caution.
The key consideration is the target platform: optimizing for a graphics card versus a CPU are entirely different challenges.
Before and during any optimization, profiling is essential — there should be no optimization without measurement!

> 前面的代码虽然很好用，但仍有优化的余地。
> 不过优化必须谨慎行事。
> 关键要考虑的是目标平台：针对显卡优化和针对 CPU 优化是完全不同的挑战。
> 在优化之前以及优化的过程中，性能剖析都是必不可少的——没有测量就没有优化！

### Round 1（第 1 轮）

For the testing I did not modify the `line` function, but I called it 16 million times with random data so the copiler does not optimize it away.

> 做测试时我没有改动 `line` 函数，只是用随机数据调用了它 1600 万次，免得被编译器优化掉。

Testing

```cpp
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "tgaimage.h"

void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    bool steep = std::abs(ax-bx) < std::abs(ay-by);
    if (steep) { // if the line is steep, we transpose the image
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax>bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    for (int x=ax; x<=bx; x++) {
        float t = (x-ax) / static_cast<float>(bx-ax);
        int y = std::round( ay + (by-ay)*t );
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
    }
}

int main(int argc, char** argv) {
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    std::srand(std::time({}));
    for (int i=0; i<(1<<24); i++) {
        int ax = rand()%width, ay = rand()%height;
        int bx = rand()%width, by = rand()%height;
        line(ax, ay, bx, by, framebuffer, { rand()%255, rand()%255, rand()%255, rand()%255 });
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
```

This code creates a pretty random image like this one:

> 这段代码会生成一张相当随机的图像，就像下面这样：

![Image 8](https://haqr.eu/tinyrenderer/bresenham/bresenham4.png)

I ran the unoptimized `line` (you can find here) for the baseline for the testing:

> 我用未优化的 `line`（可以在[这里](https://github.com/ssloy/tinyrenderer/blob/master/main.cpp)找到）跑了基线测试：

```bash
ssloy@home:~/tinyrenderer/build$ g++ ../tgaimage.cpp ../main.cpp -O3 -Wno-narrowing && time ./a.out

real    0m3.986s
user    0m3.974s
sys     0m0.009s
```

So, on my machine (Intel Xeon w3-2423 @ 3.1 Ghz) it took 3.99 seconds to draw 16 millions lines with full compiler optimization settings `-O3` turned on.

> 于是，在我的机器上（Intel Xeon w3-2423 @ 3.1 Ghz），在开启 `-O3` 全量编译器优化的情况下，画 1600 万条线用了 3.99 秒。

Where can we optimize the code?
I ran the profiler and I saw that the majority of time is spent in the `framebuffer.set()` calls,
but the best we can do is to inline the function and it is out of the scope of this text.
We can, however, optimize the computation of `y`.
In fact, we saw that \(y(t)= a_y + t \cdot (b_y-a_y)\), and \(t(x) = (x-a_x)/(b_x-a_x)\).
Therefore,

> 我们能从哪里入手优化呢？
> 我跑了性能剖析器，发现大部分时间都花在 `framebuffer.set()` 调用上，
> 但我们能做的顶多是把函数内联，这超出了本文的范围。
> 不过，`y` 的计算是可以优化的。
> 事实上，我们已经知道 \(y(t)= a_y + t \cdot (b_y-a_y)\)，且 \(t(x) = (x-a_x)/(b_x-a_x)\)。
> 因此，

\[
y(x)= a_y + (x-a_x) \frac{b_y-a_y}{b_x-a_x}.
\]

Note that \(x-a_x\) sweeps all the values \(0,1,2\dots\) in the `for` loop.
We can therefore form the sequence \(y_0 = a_y\), \(y_1 = y_0 + \frac{b_y-a_y}{b_x-a_x}\), \(y_2 = y_1 + \frac{b_y-a_y}{b_x-a_x}\) and so on.

> 注意，在 `for` 循环中 \(x-a_x\) 会依次取遍 \(0,1,2\dots\) 这些值。
> 因此我们可以构造序列 \(y_0 = a_y\)、\(y_1 = y_0 + \frac{b_y-a_y}{b_x-a_x}\)、\(y_2 = y_1 + \frac{b_y-a_y}{b_x-a_x}\)，依此类推。

In other words, at the expense of transforming `y` from integer type to floating point,
we can rewrite the previous code this way, the modifications are highlighted:

> 换句话说，代价是把 `y` 从整数类型变成浮点数，
> 我们可以把之前的代码改写成下面这样，改动之处已高亮：

First optimization

```cpp
void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    bool steep = std::abs(ax-bx) < std::abs(ay-by);
    if (steep) { // if the line is steep, we transpose the image
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax>bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    float y = ay;
    for (int x=ax; x<=bx; x++) {
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        y += (by-ay) / static_cast<float>(bx-ax);
    }
}
```

```bash
ssloy@home:~/tinyrenderer/build$ g++ ../tgaimage.cpp ../main.cpp -O3 -Wno-narrowing && time ./a.out

real    0m2.806s
user    0m2.801s
sys     0m0.004s
```

Here we have drastic changes in performance: the running time dropped from 3.99 to 2.8 seconds.
Frankly, I was very surprized to see this improvement, as I'd expect for the optimizer to make this optimization on its own.
However, while modern optimizing compilers may be the most complex and impressive creation of humanity in the field of software engineering,
they are not magic, and they are certainly myopic.
Adding more context helps them, and this is exactly the reason why we must measure the performance.
Note that I kept `(by-ay) / static_cast<float>(bx-ax)` inside the critical loop, but the compiler is smart enough to see that this is a constant to be precomputed (I have measured it!).

> 这里的性能变化相当显著：运行时间从 3.99 秒降到了 2.8 秒。
> 说实话，看到这样的提升我非常惊讶，我本以为优化器自己就能完成这项优化。
> 然而，现代优化编译器或许是软件工程领域人类最复杂、最令人惊叹的造物，
> 但它们并不神奇，而且目光肯定短浅。
> 给它们提供更多上下文会有帮助，这也正是我们必须测量性能的原因。
> 注意，我把 `(by-ay) / static_cast<float>(bx-ax)` 留在了关键循环里，但编译器足够聪明，能看出这是一个可以预先计算的常量（我实测过！）。

### Round 2（第 2 轮）

It is a bit disappointing that we had to switch to `float y` from `int y`, especially when `framebuffer.set(...)` needs to round it anyways.
Recall that at the core loop the line is more horizontal than vertical, therefore, `y` is never incremented by more than 1.
Let us try to switch to `int y` back: we will introduce a `float error` variable that measures the error commited by `y`.
That is, for each value of `x`, `error` gives the distance from `y` to the perfect line segment.
Every time `error` exceeds half a pixel, we will increment `y` and decrement `error`.
Long story short, check this code:

> 不得不把 `int y` 换成 `float y`，总归有点让人失望，尤其是 `framebuffer.set(...)` 反正也需要对它取整。
> 回想一下，在核心循环里线条更接近水平而非竖直，因此 `y` 每次最多增加 1。
> 我们试着换回 `int y`：引入一个 `float error` 变量，用来衡量 `y` 所犯的“误差”。
> 也就是说，对每个 `x` 的取值，`error` 给出 `y` 到理想线段的距离。
> 每当 `error` 超过半个像素，我们就把 `y` 递增、把 `error` 递减。
> 长话短说，看代码：

Second optimization

```cpp
void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    bool steep = std::abs(ax-bx) < std::abs(ay-by);
    if (steep) { // if the line is steep, we transpose the image
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax>bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    int y = ay;
    float error = 0;
    for (int x=ax; x<=bx; x++) {
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        error += std::abs(by-ay)/static_cast<float>(bx-ax);
        if (error>.5) {
            y += by > ay ? 1 : -1;
            error -= 1.;
        }
    }
}
```

Once again, I highlighted the lines affected by the changes.
This time the timings are not surprizing, we are doing more work, so the time increased from 2.8 to 3.7 seconds :

> 高亮的依旧是受改动影响的行。
> 这次的耗时并不让人意外，我们做的活儿更多了，时间从 2.8 秒增加到了 3.7 秒：

```bash
ssloy@home:~/tinyrenderer/build$ g++ ../tgaimage.cpp ../main.cpp -O3 -Wno-narrowing && time ./a.out

real    0m3.717s
user    0m3.712s
sys     0m0.004s
```

### Round 3, the final one（第 3 轮：最终版）

Can we eliminate floating-point computations entirely? Let's examine the previous listing.

> 能不能彻底消除浮点运算？我们来端详一下前面的代码。

We start with `error` initialized to zero, and we actually use this variable three times—in lines 18, 19, and 21.
Do we need fractional values? Surprisingly, not at all.
By introducing an integer variable `ierror`, defined as `error * 2 * (bx - ax)`, we can eliminate floating-point operations altogether.
Check the following code:

> 一开始 `error` 被初始化为零，这个变量实际上被用到了三次——在第 18、19 和第 21 行。
> 我们需要小数值吗？出乎意料，完全不需要。
> 引入一个整数变量 `ierror`，定义为 `error * 2 * (bx - ax)`，就能把浮点运算一扫而光。
> 看下面的代码：

Final optimization

```cpp
void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    bool steep = std::abs(ax-bx) < std::abs(ay-by);
    if (steep) { // if the line is steep, we transpose the image
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax>bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    int y = ay;
    int ierror = 0;
    for (int x=ax; x<=bx; x++) {
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        ierror += 2 * std::abs(by-ay);
        if (ierror > bx - ax) {
            y += by > ay ? 1 : -1;
            ierror -= 2 * (bx-ax);
        }
    }
}
```

Once again, I have highlighted the affected lines.
It turns out we have arrived at Bresenham's line-drawing algorithm.
Now, let's measure the performance:

> 同样，受影响的几行已经高亮。
> 就这样，我们到达了布雷森汉姆画线算法。
> 现在测测性能：

```bash
ssloy@home:~/tinyrenderer/build$ g++ ../tgaimage.cpp ../main.cpp -O3 -Wno-narrowing && time ./a.out

real    0m3.573s
user    0m3.572s
sys     0m0.000s
```

The time dropped from 3.7 seconds in the previous round (and from 3.99 seconds in the non-optimized version) to 3.57 seconds in this one.
However, it __increased__ from 2.8 seconds in round 1!
I attempted optimizations such as moving `2*(bx - ax)` out of the loop, but the compiler had already performed this optimization.

> 耗时从上一轮的 3.7 秒（以及未优化版本的 3.99 秒）降到了这一轮的 3.57 秒。
> 然而，相比第 1 轮的 2.8 秒反而__增加了__！
> 我尝试过把 `2*(bx - ax)` 挪出循环之类的优化，但编译器早就做过这项优化了。

In the past, floating-point operations were significantly more expensive than integer ones (or even entirely inaccessible).
This is why Jack Elton Bresenham developed his all-integer rasterization algorithm in the 1960s.
As seen in my experiments, integers can still be faster than floating-point computations (this round is more efficient than the previous one), but the performance gain is marginal.
Today, integer operations are not always more efficient than floating-point calculations — it depends on the context.

> 在过去，浮点运算比整数运算昂贵得多（甚至完全无法使用）。
> 这正是 Jack Elton Bresenham 在 1960 年代提出他那套全整数光栅化算法的原因。
> 从我的实验可以看到，整数如今依然可能比浮点计算更快（这一轮就比上一轮更高效），但性能收益已经微乎其微。
> 如今，整数运算并不总是比浮点计算更高效——这取决于具体场景。

Nevertheless, mastering these techniques remains valuable.
As mentioned earlier, this section serves mainly as a historical tribute to Professor Bresenham.
His algorithm is elegant, and the discovery of all-integer rasterization was truly ingenious.

> 尽管如此，掌握这些技巧仍然很有价值。
> 如前所述，这一节主要是向 Bresenham 教授致敬。
> 他的算法很优雅，全整数光栅化的发现堪称真正的天才之举。

### Round 4, postscriptum（第 4 轮：附言）

What is the real reason why the code from round 3 is slower than the code from round 1? We are performing integer computations, which are generally faster than floating-point ones.
However, we introduced an `if` condition inside the critical loop, and that negatively impacts performance.

> 第 3 轮的代码为什么比第 1 轮慢？我们做的可是整数运算，通常比浮点运算更快。
> 但我们在关键循环里引入了一个 `if` 条件，这对性能有负面影响。

On a typical x86 processor, instruction execution is divided into several stages.
Different hardware components handle different stages, allowing multiple instructions to be processed simultaneously.
This technique is called __pipelining__.
However, when a branch occurs, the next instruction depends on the result of the previous one, preventing efficient pipelining.
The processor either waits or predicts the outcome, potentially mispredicting and stalling the pipeline.

> 在典型的 x86 处理器上，指令执行被划分为若干个阶段。
> 不同的硬件部件负责不同阶段，从而可以同时处理多条指令。
> 这项技术叫做__流水线（pipelining）__。
> 然而，一旦出现分支，下一条指令就依赖于前一条指令的结果，流水线就没法高效运转了。
> 处理器要么等待，要么预测结果，而预测失败就会导致流水线停顿。

The `if (steep)` condition does not significantly affect performance, as modern branch predictors handle it well — especially since the same branch is consistently taken inside the critical loop.
However, `if (ierror > bx - ax)` is the performance killer.

> `if (steep)` 这个条件对性能影响不大，现代分支预测器能处理得很好——尤其因为在关键循环内部，这一分支的走向始终一致。
> 而 `if (ierror > bx - ax)` 才是性能杀手。

There are techniques to rewrite computations in a branchless form, but I applied a trick so ugly, that I didn't even commit it to the repository.
Check lines 19-20:

> 有一些技巧可以把计算改写成无分支的形式，但我用的招数实在太丑，连仓库都没好意思提交。
> 请看第 19-20 行：

Ugly branchless rasterization

```cpp
void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    bool steep = std::abs(ax-bx) < std::abs(ay-by);
    if (steep) { // if the line is steep, we transpose the image
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax>bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    int y = ay;
    int ierror = 0;
    for (int x=ax; x<=bx; x++) {
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        ierror += 2 * std::abs(by-ay);
        y += (by > ay ? 1 : -1) * (ierror > bx - ax);
        ierror -= 2 * (bx-ax)   * (ierror > bx - ax);
    }
}
```

As I said, the trick is ugly, I __always__ increment `y` and `ierror`, but most of time by zero :)
Let us measure the performance:

> 如我所说，这个招数很丑：我__总是__递增 `y` 和 `ierror`，只不过大多数时候加的是零 :)
> 来看看性能如何：

```bash
ssloy@home:~/tinyrenderer/build$ g++ ../tgaimage.cpp ../main.cpp -O3 -Wno-narrowing && time ./a.out

real    0m2.732s
user    0m2.731s
sys     0m0.000s
```

Yay, this code is the fastest!

> 耶，这段代码最快！

## Homework: wireframe rendering（作业：线框渲染）

Now we are ready to create wireframe renderings. What is missing?
The art!
Here is an example of an object to render.
I used the wavefront obj file format to store the models,
it is a simple text-based format.
At this stage, we are interested by the lines starting by `v` and by `f`.

> 现在我们准备好做线框渲染了。还差什么？
> 差素材！
> 这里有一个可以拿来渲染的物体示例。
> 我用 wavefront obj 文件格式来存储模型，
> 它是一种简单的基于文本的格式。
> 在这个阶段，我们关心的是以 `v` 和以 `f` 开头的行。

Lines starting with `v` define vertices in 3D space. Each line contains three numbers representing the \(x\), \(y\) and \(z\) coordinates of a vertex.
Example:

> 以 `v` 开头的行定义 3D 空间中的顶点。每行包含三个数字，分别表示一个顶点的 \(x\)、\(y\) 和 \(z\) 坐标。
> 例如：

```bash
v 0.608654 -0.568839 -0.416318
```

This means there is a vertex at coordinates (0.608654,−0.568839,−0.416318).

> 这表示在坐标 (0.608654,−0.568839,−0.416318) 处有一个顶点。

Lines starting with `f` define faces, which are polygons connecting the vertices from the previous array.
Example:

> 以 `f` 开头的行定义面，也就是把前面数组中的顶点连接起来构成的多边形。
> 例如：

```bash
f 1193/1240/1193 1180/1227/1180 1179/1226/1179
```

Right now we are interested in the first number after each space.
It is the index of the vertex in the `v` array that we have read before.
Thus, this line says that vertices 1193, 1180 and 1179 form a triangle.
Note that in obj files indices start from 1, so we need to decrement these indices when using C++ arrays.

> 目前我们关心的是每个空格之后的第一个数字。
> 它是之前读取的 `v` 数组中的顶点索引。
> 因此，这一行的意思是顶点 1193、1180 和 1179 构成一个三角形。
> 注意，obj 文件中索引从 1 开始，所以在 C++ 数组里使用时要把这些索引减一。

Pause here for a moment and try to generate the following image.
Whether you succeed or struggle, attempting the task before reading further is essential.
Simply reading the code that follows will not be as beneficial unless you have made an effort to implement your own version first.

> 先停一停，试着生成下面这张图像。
> 无论你是成功还是卡住，在继续往下读之前先动手尝试都是至关重要的。
> 除非你先努力实现过自己的版本，否则单纯读后面的代码不会有多大收获。

Once again, my source code is not what matters — I am a mediocre programmer.
The real learning happens when you compare your implementation to mine.

> 再说一遍，重要的不是我的源码——我不过是个平庸的程序员。
> 真正学到东西的时刻，是你把自己的实现和我的作对比的时候。

![Image 9](https://haqr.eu/tinyrenderer/bresenham/wireframe.png)

## My solution（我的解答）

Here is my solution to the homework assignment.
The data structure I am loading the file is straightforward:

> 以下是我对这道作业的解答。
> 我用来加载文件的数据结构非常直白：

model.h

```cpp
#include <vector>
#include "geometry.h"

class Model {
    std::vector<vec3> verts = {};    // array of vertices
    std::vector<int> facet_vrt = {}; // per-triangle index in the above array
public:
    Model(const std::string filename);
    int nverts() const; // number of vertices
    int nfaces() const; // number of triangles
    vec3 vert(const int i) const;                          // 0 <= i < nverts()
    vec3 vert(const int iface, const int nthvert) const;   // 0 <= iface <= nfaces(), 0 <= nthvert < 3
};
```

The `v` lines are loaded as is to the `verts` array as is, and the indices of the vertices forming each triangle are loaded to the `facet_vrt` array.
It means that the size of `facet_vrt` is a multiple of 3, as it stores three integers per triangle.
Those are private members, there are more high-level functions to interface the model:
`int nverts()` and `int nfaces()` allow to query the number of vertices and the number of triangles in the model,
`vec3 vert(int)` allows to query the point cloud directly, and `vec3 vert(int, int)` allows to ask for one of three points of a specific triangle.
Finally, in the constructor of `Model(std::string filename)` there is a very basic parser for the part of .obj files we are interested in right now.

> `v` 行被原样载入 `verts` 数组，构成每个三角形的顶点索引则被载入 `facet_vrt` 数组。
> 这意味着 `facet_vrt` 的大小是 3 的倍数，因为每个三角形要存三个整数。
> 这些是私有成员，另外还有一些更高层的函数供外界与模型打交道：
> `int nverts()` 和 `int nfaces()` 用来查询模型中的顶点数和三角形数，
> `vec3 vert(int)` 可以直接查询点云，而 `vec3 vert(int, int)` 则用来询问某个特定三角形的三个点之一。
> 最后，`Model(std::string filename)` 的构造函数里包含一个非常基础的解析器，处理我们目前关心的那部分 .obj 文件内容。

`vec3` template gives a storage container for 3D vectors, the data can be accessed through both `operator[]` and `.x`, `.y` and `.z` members.
At the moment it can look a bit overdesigned, but I need templates to implement `vec2` and `vec4`, as well as matrices and common arithmetic operations over it.

> `vec3` 模板提供了 3D 向量的存储容器，数据既可以通过 `operator[]` 访问，也可以通过 `.x`、`.y` 和 `.z` 成员访问。
> 眼下它可能显得有点设计过度，但我需要模板来实现 `vec2` 和 `vec4`，以及矩阵和定义在其上的常见算术运算。

And here is the main rendering code:

> 下面是主渲染代码：

main.cpp

```cpp
#include <cmath>
#include <tuple>
#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

constexpr int width  = 800;
constexpr int height = 800;

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    bool steep = std::abs(ax-bx) < std::abs(ay-by);
    if (steep) { // if the line is steep, we transpose the image
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax>bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    int y = ay;
    int ierror = 0;
    for (int x=ax; x<=bx; x++) {
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        ierror += 2 * std::abs(by-ay);
        if (ierror > bx - ax) {
            y += by > ay ? 1 : -1;
            ierror -= 2 * (bx-ax);
        }
    }
}

std::tuple<int,int> project(vec3 v) { // First of all, (x,y) is an orthogonal projection of the vector (x,y,z).
    return { (v.x + 1.) *  width/2,   // Second, since the input models are scaled to have fit in the [-1,1]^3 world coordinates,
             (v.y + 1.) * height/2 }; // we want to shift the vector (x,y) and then scale it to span the entire screen.
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    Model model(argv[1]);
    TGAImage framebuffer(width, height, TGAImage::RGB);

    for (int i=0; i<model.nfaces(); i++) { // iterate through all triangles
        auto [ax, ay] = project(model.vert(i, 0));
        auto [bx, by] = project(model.vert(i, 1));
        auto [cx, cy] = project(model.vert(i, 2));
        line(ax, ay, bx, by, framebuffer, red);
        line(bx, by, cx, cy, framebuffer, red);
        line(cx, cy, ax, ay, framebuffer, red);
    }

    for (int i=0; i<model.nverts(); i++) { // iterate through all vertices
        vec3 v = model.vert(i);            // get i-th vertex
        auto [x, y] = project(v);          // project it to the screen
        framebuffer.set(x, y, white);
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
```

The highlighted lines are of particular interest.
The loop in lines 55-62 performs actual rendering, it iterates through all triangles,
projects three vertices to the screen, and calls `line()` function we have implemented.

> 高亮的几行特别值得关注。
> 第 55-62 行的循环执行真正的渲染：遍历所有三角形，
> 把三个顶点投影到屏幕上，并调用我们实现的 `line()` 函数。

The projection (lines 41-44) merits a bit of attention. First of all, here I take a 3D point cloud that I am projecting to a 2D screen.
It turns out that if we simply forget the \(z\) coordinate, it is an orthogonal projection of the model to the \(0xy\) plane.
Then we need a small tweaking. The models in my repository are scaled so they fit the \([-1,1]^3\) cube.
Therefore, if we just drop the \(z\) coordinate, they will fit only 4 pixels, and 3 of them are out of the screen.
So, the projection is made with three following steps:

> 投影部分（第 41-44 行）值得多花点心思。首先，这里我拿到一团 3D 点云，要把它投影到 2D 屏幕上。
> 事实证明，如果干脆忘掉 \(z\) 坐标，这就是把模型正交投影到 \(0xy\) 平面上。
> 接着还需要做一点微调。我仓库里的模型都缩放过，以便正好放进 \([-1,1]^3\) 的立方体里。
> 因此，如果直接丢掉 \(z\) 坐标，它们只能占据 4 个像素，其中还有 3 个在屏幕之外。
> 所以，投影分以下三步完成：

- we drop the \(z\) coordinate
- we add 1 to both \(x\) and \(y\), therefore the 2D point cloud lies in the \([0,2]^2\) square
- we scale the square so it spans all the screen by multiplying the coordinates by `size/2`, now the 2D point cloud fits the \([0,\text{size}]^2\) square.

> - 丢掉 \(z\) 坐标
> - 把 \(x\) 和 \(y\) 都加上 1，于是 2D 点云就落在 \([0,2]^2\) 的正方形里
> - 把坐标乘以 `size/2` 缩放这个正方形使其铺满整个屏幕，此时 2D 点云就装进 \([0,\text{size}]^2\) 的正方形了。

This is called the viewport transform that we will address a couple of lessons later.

> 这就是所谓的视口变换（viewport transform），我们会在几课之后讲到它。

Here are two more renderings generated with my solution:

> 下面是用我的解答多渲染出的两张图：

![Image 10](https://haqr.eu/tinyrenderer/bresenham/wireframe2.png)

![Image 11](https://haqr.eu/tinyrenderer/bresenham/wireframe3.png)

Next, we will move on to filling triangles to create solid renders. Stay tuned!

> 接下来，我们将转向三角形填充，做出实心渲染。敬请期待！

> 原文：[Lesson 1: Bresenham's line drawing algorithm](https://haqr.eu/tinyrenderer/bresenham/)
