# tinyrenderer 课程笔记（中英对照）

这是 [ssloy/tinyrenderer](https://github.com/ssloy/tinyrenderer) 课程教程的中英对照学习笔记，原文取自 https://haqr.eu/tinyrenderer/（作者新版课程站点）。

## 阅读方式

每篇笔记按「逐段对照」组织：

- 英文原文段落完整保留
- 紧随其后的引用块（`>`）为该段的中文翻译
- 代码块、图片、数学公式原样保留，不译
- 标题格式为 `English title（中文译名）`

## 目录

| # | 课程 | 笔记 |
|---|------|------|
| 1 | Introduction / 引言 | [01-introduction.md](01-introduction.md) |
| 2 | Bresenham's line drawing algorithm / 布雷森汉姆画线算法 | [02-bresenham.md](02-bresenham.md) |
| 3 | Triangle rasterization / 三角形光栅化 | [03-rasterization.md](03-rasterization.md) |
| 4 | Primer on barycentric coordinates / 重心坐标入门 | [04-barycentric.md](04-barycentric.md) |
| 5 | Hidden faces removal / 隐藏面消除（z 缓冲） | [05-z-buffer.md](05-z-buffer.md) |
| 6 | A crude (but simple) approach to camera handling / 粗暴但简单的相机处理方式 | [06-camera-naive.md](06-camera-naive.md) |
| 7 | Better camera handling / 更好的相机处理 | [07-camera.md](07-camera.md) |
| 8 | Shading / 着色 | [08-shading.md](08-shading.md) |
| 9 | More data! / 更多数据！（纹理） | [09-textures.md](09-textures.md) |
| 10 | Tangent space normal mapping / 切线空间法线贴图 | [10-tangent.md](10-tangent.md) |
| 11 | Shadow mapping / 阴影贴图 | [11-shadow.md](11-shadow.md) |
| 12 | Indirect lighting / 间接光照（SSAO） | [12-ssao.md](12-ssao.md) |
| 13 | Bonus: toon shading / 附赠：卡通着色 | [13-toon.md](13-toon.md) |
| 14 | Afterword / 后记 | [14-afterword.md](14-afterword.md) |

## 术语约定

- rasterization 光栅化 / z-buffer 深度缓冲 / framebuffer 帧缓冲 / barycentric coordinates 重心坐标
- shading 着色 / shader 着色器 / diffuse 漫反射 / specular 镜面反射
- normal mapping 法线贴图 / tangent space 切线空间 / shadow mapping 阴影贴图 / ambient occlusion 环境光遮蔽
- 专有名词保留英文：OpenGL、Vulkan、Metal、DirectX、TGA、PNG、GPU、CPU
