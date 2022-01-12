# Tiny Renderer or how OpenGL works: software rendering in 500 lines of code

# Check [the wiki](https://github.com/ssloy/tinyrenderer/wiki) for the detailed lessons.

## compilation
```sh
git clone https://github.com/ssloy/tinyrenderer.git &&
cd tinyrenderer &&
mkdir build &&
cd build &&
cmake .. &&
cmake --build . -j &&
./tinyrenderer ../obj/diablo3_pose/diablo3_pose.obj ../obj/floor.obj
```
The rendered image is saved to `framebuffer.tga`.

You can open the project in Gitpod, a free online dev evironment for GitHub:
[![Open in Gitpod](https://gitpod.io/button/open-in-gitpod.svg)](https://gitpod.io/#https://github.com/ssloy/tinyrenderer)

On open, the editor will compile & run the program as well as open the resulting image in the editor's preview.
Just change the code in the editor and rerun the script (use the terminal's history) to see updated images.

## The main idea

**My source code is irrelevant. Read the wiki and implement your own renderer. Only when you suffer through all the tiny details you will learn what is going on.**

In [this series of articles](https://github.com/ssloy/tinyrenderer/wiki), I want to show the way OpenGL works by writing its clone (a much simplified one). Surprisingly enough, I often meet people who cannot overcome the initial hurdle of learning OpenGL / DirectX. Thus, I have prepared a short series of lectures, after which my students show quite good renderers.

So, the task is formulated as follows: using no third-party libraries (especially graphic ones), get something like this picture:

![](https://raw.githubusercontent.com/ssloy/tinyrenderer/gh-pages/img/00-home/africanhead.png)

_Warning: this is a training material that will loosely repeat the structure of the OpenGL library. It will be a software renderer. **I do not want to show how to write applications for OpenGL. I want to show how OpenGL works.** I am deeply convinced that it is impossible to write efficient applications using 3D libraries without understanding this._

I will try to make the final code about 500 lines. My students need 10 to 20 programming hours to begin making such renderers. At the input, we get a test file with a polygonal wire + pictures with textures. At the output, we’ll get a rendered model. No graphical interface, the program simply generates an image.


Since the goal is to minimize external dependencies, I give my students just one class that allows working with [TGA](http://en.wikipedia.org/wiki/Truevision_TGA) files. It’s one of the simplest formats that supports images in RGB/RGBA/black and white formats. So, as a starting point, we’ll obtain a simple way to work with pictures. You should note that the only functionality available at the very beginning (in addition to loading and saving images) is the capability to set the color of one pixel.

There are no functions for drawing line segments and triangles. We’ll have to do all of this by hand. I provide my source code that I write in parallel with students. But I would not recommend using it, as this doesn’t make sense. The entire code is available on github, and [here](https://github.com/ssloy/tinyrenderer/tree/909fe20934ba5334144d2c748805690a1fa4c89f) you will find the source code I give to my students.

```C++
#include "tgaimage.h"
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
int main(int argc, char** argv) {
        TGAImage image(100, 100, TGAImage::RGB);
        image.set(52, 41, red);
        image.write_tga_file("output.tga");`
        return 0;
}
```

output.tga should look something like this:

![](https://raw.githubusercontent.com/ssloy/tinyrenderer/gh-pages/img/00-home/reddot.png)


# Teaser: few examples made with the renderer

![](https://raw.githubusercontent.com/ssloy/tinyrenderer/gh-pages/img/00-home/demon.png)

![](https://raw.githubusercontent.com/ssloy/tinyrenderer/gh-pages/img/00-home/diablo-glow.png)

![](https://raw.githubusercontent.com/ssloy/tinyrenderer/gh-pages/img/00-home/boggie.png) 

![](https://raw.githubusercontent.com/ssloy/tinyrenderer/gh-pages/img/00-home/diablo-ssao.png)
