# Triangle rasterization（三角形光栅化）

Let me remind you: this series of articles is designed to encourage you to write your own code from scratch.
Now is the time to do just that.
My code is provided solely for comparison with your (working) program.
I consider myself an average programmer—it’s very likely that you are a better one.
Do not simply copy and paste my code.

> 先提醒一句：这一系列文章的目的，是鼓励你从零开始写自己的代码。
> 现在正是动手的时候。
> 我提供的代码，仅仅用来和你那个（能正常工作的）程序做对照。
> 我自认只是个水平一般的程序员——你很可能比我更强。
> 不要直接复制粘贴我的代码。

Any comments and questions are welcome.

> 欢迎提出任何评论和问题。

## The task at hand（手头的任务）

Today's challenge is to draw two-dimensional triangles.
For motivated students, this usually takes a couple of hours, even if they are not confident programmers.

> 今天的挑战是画二维三角形。
> 对肯动脑筋的学生来说，这通常要花上几个小时——哪怕他们对自己的编程水平没什么信心。

Last time, we explored Bresenham’s line-drawing algorithm.
Now we need to draw a __filled__ triangle.
Surprisingly, this is not a trivial task.
I’m not sure why, but I do know that it’s true — most of my students struggle with this seemingly simple problem.

> 上次课我们研究了 Bresenham 画线算法。
> 现在我们要画一个 __实心__ 三角形。
> 出乎意料的是，这可不是个轻松的任务。
> 我也说不清为什么，但我知道事实确实如此——我的大多数学生都在这个看似简单的问题上卡过壳。

To get started, [here is the initial commit](https://github.com/ssloy/tinyrenderer/commit/9e2dcd6fddca0eec5cbf6b75b12202dea833e622).
It builds upon the line rasterization routine that we implemented in the previous lesson.

> 开始之前，[这里是初始提交](https://github.com/ssloy/tinyrenderer/commit/9e2dcd6fddca0eec5cbf6b75b12202dea833e622)。
> 它建立在我们上一课实现的画线光栅化例程之上。

![Image 1](https://haqr.eu/task.png)

The starting point（起点）

```
void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color) {
    line(ax, ay, bx, by, framebuffer, color);
    line(bx, by, cx, cy, framebuffer, color);
    line(cx, cy, ax, ay, framebuffer, color);
}

int main(int argc, char** argv) {
    TGAImage framebuffer(width, height, TGAImage::RGB);
    triangle(  7, 45, 35, 100, 45,  60, framebuffer, red);
    triangle(120, 35, 90,   5, 45, 110, framebuffer, white);
    triangle(115, 83, 80,  90, 85, 120, framebuffer, green);
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
```

The code is straightforward: I provide three test triangles for initial debugging.
If we call `line()` inside the `triangle()` function, we get the triangle’s edges.
But how do we fill the triangle?

> 代码很直白：我准备了三个测试用的三角形，方便一开始调试。
> 如果在 `triangle()` 函数里调用 `line()`，我们得到的只是三角形的三条边。
> 但怎么把三角形填充起来呢？

### Properties of a good triangle-filling algorithm（一个好用的三角形填充算法应具备的性质）

A good method for drawing a filled triangle should meet the following criteria:

> 一个好用的实心三角形绘制方法，应当满足以下标准：

- It should be __simple and fast__.
- It should be __symmetrical__ — the output should not depend on the order of vertices passed to the function.
- If two triangles share two vertices, __there should be no gaps between them__ due to rasterization rounding errors.

> - 应当__简单且快速__。
> - 应当__对称__——输出不应依赖于传给函数的顶点顺序。
> - 如果两个三角形共享两个顶点，__它们之间不应出现缝隙__——不能因为光栅化的舍入误差而露缝。

We could add more requirements, but let’s focus on these for now.

> 我们还可以提出更多要求，但眼下先关注这几条。

## Scanline rendering, the old-school way（扫描线渲染，老派做法）

A common approach for filling triangles involves [scanline rendering](https://en.wikipedia.org/wiki/Scanline_rendering):

> 填充三角形的一种常见做法是[扫描线渲染](https://en.wikipedia.org/wiki/Scanline_rendering)：

1. Sort the vertices of the triangle by their y-coordinates.
2. Rasterize both the left and right edges of the triangle simultaneously.
3. Draw horizontal line segments between the left and right boundary points.

> 1. 按 y 坐标给三角形的顶点排序。
> 2. 同时光栅化三角形的左、右两条边。
> 3. 在左右两个边界点之间画水平线段。

At this stage, my students often start to lose confidence: _Which segment is the left one? Which is the right?_ After all, a triangle has three sides…

> 到了这一步，我的学生常常开始发懵：_哪条是左边？哪条是右边？_ 毕竟三角形有三条边……

Usually, after introducing this problem, I leave my students for about an hour.
Once again, reading my code is far less valuable than writing your own and comparing it with mine.
Pause here for a moment and try to draw filled triangles.

> 通常，抛出这个问题之后，我会让学生们自己琢磨大约一个小时。
> 再说一次：读我的代码，远不如自己动手写出来、再和我的代码对照更有价值。
> 在这里暂停一下，试着自己画出实心三角形。

[One hour later...]

> [一小时后……]

### Sorting the vertices（给顶点排序）

Let us assume that we have three points of the triangle: `a`, `b`, `c`, they are sorted in ascending order by the `y`-coordinate.
As in the line rasterization, to meet this assumption we will reorder the vertices that were passed to the function.
To sort three elements, [bubble sort](https://en.wikipedia.org/wiki/Bubble_sort) is the ultimate method:

> 假设三角形的三个点是 `a`、`b`、`c`，并且已按 `y` 坐标升序排好。
> 和画线光栅化时一样，为了满足这个前提，我们会对传入函数的顶点重新排序。
> 给三个元素排序，[冒泡排序](https://en.wikipedia.org/wiki/Bubble_sort)就是终极方案：

Bubble sort（冒泡排序）

```
void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color) {
    // sort the vertices, a,b,c in ascending y order (bubblesort yay!)
    if (ay>by) { std::swap(ax, bx); std::swap(ay, by); }
    if (ay>cy) { std::swap(ax, cx); std::swap(ay, cy); }
    if (by>cy) { std::swap(bx, cx); std::swap(by, cy); }
    line(ax, ay, bx, by, framebuffer, green);
    line(bx, by, cx, cy, framebuffer, green);
    line(cx, cy, ax, ay, framebuffer, red);
}
```

Then, the boundary `A` is between `a` and `c`, boundary `B` is between `a` and `b`, and then between `b` and `c`.
Here the boundary `A` is red, and the boundary `B` is green:

> 这样一来，边界 `A` 位于 `a` 和 `c` 之间，边界 `B` 则先是位于 `a` 和 `b` 之间，随后位于 `b` 和 `c` 之间。
> 图中边界 `A` 画成红色，边界 `B` 画成绿色：

![Image 2](https://haqr.eu/scanline1.png)

### Rasterizing the boundary（光栅化边界）

Unfortunately, boundary `B` is made of two parts. Let us draw the bottom half of the triangle by cutting it horizontally at the point `b`:

> 麻烦的是，边界 `B` 由两部分组成。我们把三角形在点 `b` 处水平切开，先画它的下半部分：

Border rasterization（边界光栅化）

```
void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color) {
    // sort the vertices, a,b,c in ascending y order (bubblesort yay!)
    if (ay>by) { std::swap(ax, bx); std::swap(ay, by); }
    if (ay>cy) { std::swap(ax, cx); std::swap(ay, cy); }
    if (by>cy) { std::swap(bx, cx); std::swap(by, cy); }
    int total_height = cy-ay;

    if (ay != by) { // if the bottom half is not degenerate
        int segment_height = by - ay;
        for (int y=ay; y<=by; y++) { // sweep the horizontal line from ay to by
            int x1 = ax + ((cx - ax)*(y - ay)) / total_height;
            int x2 = ax + ((bx - ax)*(y - ay)) / segment_height;
            framebuffer.set(x1, y, red);
            framebuffer.set(x2, y, green);
        }
    }
}
```

Since we are looping over `y` coordinates (line 10), it is to be expected for the segments to have gaps if the segment is rather horizontal.
Last time when we drew straight lines we struggled to get continuous segments and here I did not bother with rotating the image (remember the xy swapping?).
Why? We fill the triangles aftewards, that's why.

> 因为我们是按 `y` 坐标循环的（第 10 行），所以如果某段边界比较水平，线段之间出现空隙是意料之中的事。
> 上次画直线时，我们费了好大劲才让线段保持连续；而这里我压根没去操心旋转图像（还记得 xy 互换吗？）。
> 为什么？因为我们随后会把三角形填满，就是这么回事。

![Image 3](https://haqr.eu/scanline2.png)

### Scanline rasterization（扫描线光栅化）

So, at each iteration of the lopp we have two points with the same `y` coordinate, namely, `(x1, y)` and `(x2, y)`.
To fill the triangle it suffices to iterate from `x1` to `x2` (lines 13-14):

> 于是，循环每迭代一次，我们就得到两个 `y` 坐标相同的点，即 `(x1, y)` 和 `(x2, y)`。
> 要填充三角形，只需从 `x1` 遍历到 `x2`（第 13-14 行）：

Scan line for the lower half（下半部分的扫描线）

```
void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color) {
    // sort the vertices, a,b,c in ascending y order (bubblesort yay!)
    if (ay>by) { std::swap(ax, bx); std::swap(ay, by); }
    if (ay>cy) { std::swap(ax, cx); std::swap(ay, cy); }
    if (by>cy) { std::swap(bx, cx); std::swap(by, cy); }
    int total_height = cy-ay;

    if (ay != by) { // if the bottom half is not degenerate
        int segment_height = by - ay;
        for (int y=ay; y<=by; y++) { // sweep the horizontal line from ay to by
            int x1 = ax + ((cx - ax)*(y - ay)) / total_height;
            int x2 = ax + ((bx - ax)*(y - ay)) / segment_height;
            for (int x=std::min(x1,x2); x<std::max(x1,x2); x++)  // draw a horizontal line
                framebuffer.set(x, y, color);
        }
    }
}
```

![Image 4](https://haqr.eu/scanline3.png)

Now, let us draw the second (upper) half of the triangle. We can do this by adding a second loop:

> 接下来画三角形的另一半（上半部分）。加第二个循环就行：

Complete scan line（完整的扫描线）

```
void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color) {
    // sort the vertices, a,b,c in ascending y order (bubblesort yay!)
    if (ay>by) { std::swap(ax, bx); std::swap(ay, by); }
    if (ay>cy) { std::swap(ax, cx); std::swap(ay, cy); }
    if (by>cy) { std::swap(bx, cx); std::swap(by, cy); }
    int total_height = cy-ay;

    if (ay != by) { // if the bottom half is not degenerate
        int segment_height = by - ay;
        for (int y=ay; y<=by; y++) { // sweep the horizontal line from ay to by
            int x1 = ax + ((cx - ax)*(y - ay)) / total_height;
            int x2 = ax + ((bx - ax)*(y - ay)) / segment_height;
            for (int x=std::min(x1,x2); x<std::max(x1,x2); x++)  // draw a horizontal line
                framebuffer.set(x, y, color);
        }
    }
    if (by != cy) { // if the upper half is not degenerate
        int segment_height = cy - by;
        for (int y=by; y<=cy; y++) { // sweep the horizontal line from by to cy
            int x1 = ax + ((cx - ax)*(y - ay)) / total_height;
            int x2 = bx + ((cx - bx)*(y - by)) / segment_height;
            for (int x=std::min(x1,x2); x<std::max(x1,x2); x++)  // draw a horizontal line
                framebuffer.set(x, y, color);
        }
    }
}
```

![Image 5](https://haqr.eu/scanline4.png)

[Here’s the commit](https://github.com/ssloy/tinyrenderer/tree/ad5bb950c6dcd0dd48bedea1cf814f795bc3c0b2) for drawing 2D triangles.

> 这是绘制 2D 三角形的[提交](https://github.com/ssloy/tinyrenderer/tree/ad5bb950c6dcd0dd48bedea1cf814f795bc3c0b2)。

## Modern rasterization approach（现代光栅化方法）

While not particularly complicated, the source code for line sweeping can be somewhat messy.
Moreover, it is an old-school approach designed for single-threaded CPU programming.
Let’s take a look at the following pseudo-code:

> 用扫描线横扫填充的写法虽然不算特别复杂，但源代码多少有些凌乱。
> 而且，这是为单线程 CPU 编程设计的老派做法。
> 来看看下面这段伪代码：

```
triangle(vec2 points[3]) {
    vec2 bbox[2] = find_bounding_box(points);
    for (each pixel in the bounding box) {
        if (inside(points, pixel)) {
            put_pixel(pixel);
        }
    }
}
```

Do you like it? I do.
Finding a bounding box is straightforward, and checking whether a point belongs to a 2D triangle (or any convex polygon) is certainly manageable.

> 你喜欢它吗？我喜欢。
> 求包围盒很直白，而判断一个点是否属于二维三角形（或任意凸多边形）也完全不在话下。

There’s one aspect I appreciate about this pseudocode: a novice programmer finds it intuitive, while more experienced developers might chuckle and ask,
“Who wrote this nonsense? So much overhead!” But an expert in computer graphics will simply shrug and say, “Well, that’s how it works in real life.”
Massively parallel computations running in thousands of threads, even on regular consumer hardware, fundamentally change the way we approach problems.

> 这段伪代码有一点我特别欣赏：新手程序员觉得它很直观，而经验更丰富的开发者可能会扑哧一笑，问道：
> “谁写的这什么玩意儿？开销这么大！”但图形学专家只会耸耸肩说：“嗯，现实里就是这么干的。”
> 成千上万条线程同时运行的大规模并行计算——哪怕只是在普通消费级硬件上——已经从根本上改变了我们解决问题的方式。

First, we need to compute an axis-aligned bounding box for our three points. The bounding box is defined by two points: the bottom-left and upper-right corners.
To determine these corners, we iterate through the vertices of the triangle and select the minimum and maximum coordinates.
Here’s how we can draw a filled bounding box for each of the three triangles in our image:

> 首先，我们要为这三个点计算一个轴对齐包围盒。包围盒由两个点定义：左下角和右上角。
> 确定这两个角点的方法是：遍历三角形的顶点，取各坐标的最小值和最大值。
> 下面这段代码为图中的三个三角形分别画出实心包围盒：

Bounding box rasterization（包围盒光栅化）

```
void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color) {
    int bbminx = std::min(std::min(ax, bx), cx); // bounding box for the triangle
    int bbminy = std::min(std::min(ay, by), cy); // defined by its top left and bottom right corners
    int bbmaxx = std::max(std::max(ax, bx), cx);
    int bbmaxy = std::max(std::max(ay, by), cy);
#pragma omp parallel for
    for (int x=bbminx; x<=bbmaxx; x++) {
        for (int y=bbminy; y<=bbmaxy; y++) {
            framebuffer.set(x, y, color);
        }
    }
}
```

Note the line 6. Once we have determined the bounding box, this line tells to run the loop __in parallel__.
As opposed to the scanline method, here each pixel is treated separately,
and thus this is an [embarrassingly parallel](https://en.wikipedia.org/wiki/Embarrassingly_parallel#:~:text=In%20parallel%20computing%2C%20an%20embarrassingly,a%20number%20of%20parallel%20tasks) method.

> 注意第 6 行。确定好包围盒之后，这一行让循环__并行__执行。
> 与扫描线方法不同，这里每个像素都是单独处理的，
> 因此这是一种[易并行](https://en.wikipedia.org/wiki/Embarrassingly_parallel#:~:text=In%20parallel%20computing%2C%20an%20embarrassingly,a%20number%20of%20parallel%20tasks)（embarrassingly parallel）的方法。

![Image 6](https://haqr.eu/bbox1.png)

All we need to do now is to eliminate the pixels that do not belong to the triangle.
There are plenty of ways to check if a point \(P\) belongs to a triangle defined by vertices \(A\),\(B\),\(C\).
Here I chose to compute the [barycentric coordinates](https://en.wikipedia.org/wiki/Barycentric_coordinate_system) of \(P\) with respect to \(ABC\).
We can express \(P\) as a combination of the triangle's vertices: \(P=\alpha A + \beta B + \gamma C\).
While being a bit frightening at the first glance, it is really simple: imagine that we put three weights \(\alpha,\beta,\gamma\) at the vertices \(A\), \(B\) and \(C\), respectively.
Then the barycenter of the system is exactly in the point \(P\).

> 现在要做的，就是把不属于三角形的像素剔除掉。
> 判断一个点 \(P\) 是否属于由顶点 \(A\)、\(B\)、\(C\) 定义的三角形，方法有很多。
> 这里我选择计算 \(P\) 相对于 \(ABC\) 的[重心坐标](https://en.wikipedia.org/wiki/Barycentric_coordinate_system)。
> 我们可以把 \(P\) 表示为三角形三个顶点的组合：\(P=\alpha A + \beta B + \gamma C\)。
> 乍一看有点吓人，其实非常简单：想象我们在顶点 \(A\)、\(B\)、\(C\) 上分别放了三个权重 \(\alpha,\beta,\gamma\)。
> 那么整个系统的重心恰好就落在点 \(P\) 处。

Barycentric coordinates \(\alpha,\beta,\gamma\) are proportional to sub-triangle areas \(\text{Area}(PBC)\), \(\text{Area}(PCA)\) and \(\text{Area}(PAB)\).
If all three coordinates are positive, \(P\) lies within the triangle.
If any barycentric coordinate is negative, \(P\) lies outside the triangle.

> 重心坐标 \(\alpha,\beta,\gamma\) 与三个子三角形的面积 \(\text{Area}(PBC)\)、\(\text{Area}(PCA)\)、\(\text{Area}(PAB)\) 成正比。
> 如果三个坐标都为正，\(P\) 就在三角形内部。
> 只要有任何一个重心坐标为负，\(P\) 就在三角形外部。

So, let us program our new rasterization routine: we iterate through all pixels of a bounding box for a given triangle.
For each pixel we compute its barycentric coordinates.
If it has at least one negative component, then the pixel is outside of the triangle.
Probably it is more clear to see the program directly:

> 那么，来编写新的光栅化例程吧：对给定的三角形，遍历它包围盒里的所有像素。
> 对每个像素，计算它的重心坐标。
> 只要有一个分量为负，这个像素就在三角形之外。
> 直接看程序可能更清楚：

Triangle rasterization, modern apporach（三角形光栅化，现代方法）

```
double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5*((by-ay)*(bx+ax) + (cy-by)*(cx+bx) + (ay-cy)*(ax+cx));
}

void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color) {
    int bbminx = std::min(std::min(ax, bx), cx); // bounding box for the triangle
    int bbminy = std::min(std::min(ay, by), cy); // defined by its top left and bottom right corners
    int bbmaxx = std::max(std::max(ax, bx), cx);
    int bbmaxy = std::max(std::max(ay, by), cy);
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);

#pragma omp parallel for
    for (int x=bbminx; x<=bbmaxx; x++) {
        for (int y=bbminy; y<=bbmaxy; y++) {
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta  = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha<0 || beta<0 || gamma<0) continue; // negative barycentric coordinate => the pixel is outside the triangle
            framebuffer.set(x, y, color);
        }
    }
}
```

It is the line 18 that discards the pixels outside of the triangle.

> 正是第 18 行把三角形之外的像素丢弃掉。

![Image 7](https://haqr.eu/bbox2.png)

As always, I follow closely the one-commit-per-tutorial-image rule, so you can find the [code snapshot here](https://github.com/ssloy/tinyrenderer/tree/fc3eae6122c3f1ae33064db58ca81beaff952638).

> 和往常一样，我严格遵守“一张教程图对应一次提交”的规则，所以你可以在[这里找到代码快照](https://github.com/ssloy/tinyrenderer/tree/fc3eae6122c3f1ae33064db58ca81beaff952638)。

## Putting all together + back-face culling（整合所有内容 + 背面剔除）

Let us recall the wireframe [rendering we made](https://haqr.eu/tinyrenderer/bresenham/#my-solution) last time.
We looped over the triangles, projected the triangles to the screen and made 3 calls to the `line()` function.
We can replace the the 3 calls by a single call to our brand new `triangle()`:

> 回想一下我们上次做的线框[渲染](https://haqr.eu/tinyrenderer/bresenham/#my-solution)。
> 我们遍历所有三角形，把它们投影到屏幕上，然后调用了 3 次 `line()` 函数。
> 现在可以用一次全新的 `triangle()` 调用取代这 3 次调用：

Straightforward rasterization of all triangles（直接光栅化所有三角形）

```
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
        TGAColor rnd;
        for (int c=0; c<3; c++) rnd[c] = std::rand()%255;
        triangle(ax, ay, bx, by, cx, cy, framebuffer, rnd);
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
```

You can find the commit here [here](https://github.com/ssloy/tinyrenderer/tree/e25bb49be00a43ab777e511baa4b72e8ab16d6b1).
The changes are highlighted, lines 14-16. I generate a random color and draw a filled triangle.
Here is the result:

> 你可以在[这里](https://github.com/ssloy/tinyrenderer/tree/e25bb49be00a43ab777e511baa4b72e8ab16d6b1)找到该提交。
> 改动已高亮标出，位于第 14-16 行。我随机生成一个颜色，画一个实心三角形。
> 结果如下：

![Image 8](https://haqr.eu/no-back-face-culling.png)

Well, it is certainly something, but I'd like to see a face rather than a complete mess. What happened?
It happened that we did zero removal of hiden faces, and being drawn in disorder, the "back" triangles may be shown ontop of the "front".
We will fully address the issue next time, but let us do a simple hack right now. Let us add one single line 7
(you can [check the diff](https://github.com/ssloy/tinyrenderer/commit/ab559b1325df38266b420c6236a33794bd7e4c65) with the previous version):

> 嗯，确实画出了点东西，但我想要的是看清一张脸，而不是一团乱麻。这是怎么回事？
> 原因是我们完全没有做隐藏面消除；三角形又是乱序绘制的，于是“背面”的三角形就可能显示在“正面”之上。
> 这个问题我们下次再彻底解决，不过现在先做个简单的应急处理：只加第 7 行这一行
> （你可以[查看差异](https://github.com/ssloy/tinyrenderer/commit/ab559b1325df38266b420c6236a33794bd7e4c65)来和上一版对比）：

Back-face culling（背面剔除）

```
void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color) {
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
            framebuffer.set(x, y, color);
        }
    }
}
```

And while not being perfect, the result improves drastically:

> 虽然还不完美，但效果已经大幅改善：

![Image 9](https://haqr.eu/back-face-culling.png)

What happened? Let us reduce the number of dimensions and make a 1D rendering of a 2D scene.
Imagine that we have a 2D polygon \(ABCDE\) that we want to draw on a 1D line defined by the \(x\)-axis of our coordinate system:

> 这是怎么回事？我们把维度降一降，来做一个二维场景的一维渲染。
> 想象有一个二维多边形 \(ABCDE\)，要把它画到由我们坐标系的 \(x\) 轴所定义的一条一维直线上：

![Image 10: backface culling illustration](https://haqr.eu/culling.png)

We need to draw 5 segments, namely, \(AB\), \(BC\), \(CD\), \(DE\), and \(EA\).
However, we can be quite certain that we can safely discard segments \(AB\), \(CD\) and \(EA\) since they are facing back and not front.

> 我们需要画 5 条线段，即 \(AB\)、\(BC\)、\(CD\)、\(DE\) 和 \(EA\)。
> 不过我们可以相当确定，\(AB\)、\(CD\) 和 \(EA\) 这三条线段可以放心丢弃，因为它们背朝我们，而不是面朝我们。

The order in which segments are stored in memory can be arbitrary, but an important aspect is that there is consistency in how the endpoints are stored for each segment.
To decide whether a segment is facing front or back, we can simply compute the sign of its length once projected onto the screen.
For now, our projection is orthogonal (we are ignoring the \(y\) coordinate), so the problem reduces to comparing the \(x\) coordinates of the segment’s endpoints.

> 线段在内存中的存储顺序可以任意，但有一点很重要：每条线段的端点以何种顺序存储，必须保持一致。
> 要判断一条线段是朝前还是朝后，只需计算它投影到屏幕后长度的符号即可。
> 目前我们的投影是正交投影（我们忽略了 \(y\) 坐标），所以问题简化为比较线段两端点的 \(x\) 坐标。

If a segment extends from point \(A\) to point \(B\), its projected length on the screen is given by:

> 如果一条线段从点 \(A\) 延伸到点 \(B\)，它在屏幕上的投影长度为：

\[ L_{AB} = B_x - A_x \]

- If \(L_{AB} > 0\), the segment is front-facing and should be drawn.
- If \(L_{AB} < 0\), the segment is back-facing and can be culled.

> - 若 \(L_{AB} > 0\)，线段面朝前方，应当绘制。
> - 若 \(L_{AB} < 0\)，线段背朝后方，可以剔除。

For example, in the case of segment \(AB\):

> 例如，对于线段 \(AB\) 来说：

\[ B_x < A_x \Rightarrow L_{AB} < 0 \]

Since the projected length is negative, we can discard segment \(AB\).
By applying this simple test to each segment, we can efficiently determine which edges should be drawn and which can be ignored,
optimizing the rendering process and avoiding unnecessary computations for back-facing segments.

> 由于投影长度为负，线段 \(AB\) 可以丢弃。
> 对每条线段做这个简单判断，我们就能高效地确定哪些边应当绘制、哪些可以忽略，
> 从而优化渲染过程，避免对背向线段做无用功。

Back to our 3D rendering of the head, the line of code we have added checks the signed area of the triangle to be drawn.
If it is negative, the triangle can be discarded.
Note that this is a conservative test that does not prevent all rendering errors.
In our 2D example, nothing prevents segment \(DE\) from being drawn over \(BC\), since they are both front-facing.
This is exactly why the rendering of the head is not perfect: we have eliminated all back-facing triangles,
but some front-facing triangles inside the mouth were drawn over the lips. We will correct this artefact later.

> 回到头部的 3D 渲染：我们加的那行代码检查的是待绘制三角形的有符号面积。
> 如果为负，这个三角形就可以丢弃。
> 注意，这是一个保守的测试，并不能杜绝所有渲染错误。
> 在刚才的二维例子里，没有什么能阻止线段 \(DE\) 被画在 \(BC\) 上面，因为它们都面朝前方。
> 这正是头部渲染还不完美的原因：我们剔除了所有背向的三角形，
> 但嘴巴内部一些面朝前的三角形被画到了嘴唇上面。这个瑕疵我们以后再修正。

## Next time（下一次）

Next time, I propose spending a little more time on [barycentric coordinates](https://haqr.eu/tinyrenderer/barycentric/), as I will make heavy use of them later on.
Then, we can tackle the [removal of hidden faces](https://haqr.eu/tinyrenderer/z-buffer/).

> 下一次，我打算多花点时间讲讲[重心坐标](https://haqr.eu/tinyrenderer/barycentric/)，因为后面我会大量用到它。
> 然后，我们就可以着手解决[隐藏面消除](https://haqr.eu/tinyrenderer/z-buffer/)的问题了。

> 原文：[Lesson 2: Triangle rasterization](https://haqr.eu/tinyrenderer/rasterization/)
