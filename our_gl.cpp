#include "our_gl.h"

mat<4,4> ModelView;
mat<4,4> Viewport;
mat<4,4> Projection;

void viewport(const int x, const int y, const int w, const int h) {
    Viewport = {{{w/2., 0, 0, x+w/2.}, {0, h/2., 0, y+h/2.}, {0,0,1,0}, {0,0,0,1}}};
}

void projection(const double f) { // check https://en.wikipedia.org/wiki/Camera_matrix
    Projection = {{{1,0,0,0}, {0,-1,0,0}, {0,0,1,0}, {0,0,-1/f,0}}};
}

void lookat(const vec3 eye, const vec3 center, const vec3 up) { // check https://github.com/ssloy/tinyrenderer/wiki/Lesson-5-Moving-the-camera
    vec3 z = normalized(center-eye);
    vec3 x = normalized(cross(up,z));
    vec3 y = normalized(cross(z, x));
    ModelView = mat<4,4>{{{x.x,x.y,x.z,0}, {y.x,y.y,y.z,0}, {z.x,z.y,z.z,0}, {0,0,0,1}}} *
                mat<4,4>{{{1,0,0,-eye.x},  {0,1,0,-eye.y},  {0,0,1,-eye.z},  {0,0,0,1}}};
}

vec3 barycentric(const vec2 tri[3], const vec2 P) {
    mat<3,3> ABC = {{ {tri[0].x, tri[0].y, 1.}, {tri[1].x, tri[1].y, 1.}, {tri[2].x, tri[2].y, 1.} }};
    if (ABC.det()<1) return {-1,1,1}; // for a degenerate triangle generate negative coordinates, it will be thrown away by the rasterizator
    return ABC.invert_transpose() * vec3{P.x, P.y, 1.};
}

void rasterize(const vec4 clip_verts[3], const IShader &shader, TGAImage &image, std::vector<double> &zbuffer) {
    vec4 pts [3] = { Viewport*clip_verts[0], Viewport*clip_verts[1], Viewport*clip_verts[2] }; // screen coordinates before persp. division
    vec2 pts2[3] = { (pts[0]/pts[0].w).xy(), (pts[1]/pts[1].w).xy(), (pts[2]/pts[2].w).xy() }; // screen coordinates after  perps. division

    int bbminx = std::max(0, static_cast<int>(std::min(std::min(pts2[0].x, pts2[1].x), pts2[2].x))); // bounding box for the triangle
    int bbminy = std::max(0, static_cast<int>(std::min(std::min(pts2[0].y, pts2[1].y), pts2[2].y))); // clipped by the screen
    int bbmaxx = std::min(image.width() -1, static_cast<int>(std::max(std::max(pts2[0].x, pts2[1].x), pts2[2].x)));
    int bbmaxy = std::min(image.height()-1, static_cast<int>(std::max(std::max(pts2[0].y, pts2[1].y), pts2[2].y)));
#pragma omp parallel for
    for (int x=bbminx; x<=bbmaxx; x++) { // rasterize the bounding box
        for (int y=bbminy; y<=bbmaxy; y++) {
            vec3 bc_screen = barycentric(pts2, {static_cast<double>(x), static_cast<double>(y)});
            vec3 bc_clip   = { bc_screen.x/pts[0].w, bc_screen.y/pts[1].w, bc_screen.z/pts[2].w };     // check https://github.com/ssloy/tinyrenderer/wiki/Technical-difficulties-linear-interpolation-with-perspective-deformations
            bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);
            double frag_depth = bc_clip * vec3{ clip_verts[0].z, clip_verts[1].z, clip_verts[2].z };
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0 || frag_depth > zbuffer[x+y*image.width()]) continue;
            TGAColor color;
            if (shader.fragment(bc_clip, color)) continue; // fragment shader can discard current fragment
            zbuffer[x+y*image.width()] = frag_depth;
            image.set(x, y, color);
        }
    }
}

