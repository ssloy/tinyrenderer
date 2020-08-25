#include <limits>
#include "our_gl.h"

mat<4,4> ModelView;
mat<4,4> Viewport;
mat<4,4> Projection;

void viewport(const int x, const int y, const int w, const int h) {
    Viewport = {{{w/2., 0, 0, x+w/2.}, {0, h/2., 0, y+h/2.}, {0,0,1,0}, {0,0,0,1}}};
}

void projection(const double coeff) {
    Projection = {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,coeff,1}}};
}

void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    vec3 z = (eye-center).normalize();
    vec3 x = cross(up,z).normalize();
    vec3 y = cross(z,x).normalize();
    mat<4,4> Minv = mat<4,4>::identity();
    mat<4,4> Tr   = mat<4,4>::identity();
    for (int i=0; i<3; i++) {
        Minv[0][i] = x[i];
        Minv[1][i] = y[i];
        Minv[2][i] = z[i];
        Tr[i][3] = -center[i];
    }
    ModelView = Minv*Tr;
}

vec3 barycentric(const vec2 A, const vec2 B, const vec2 C, const vec2 P) {
    mat<3,3> ABC = {{embed<3>(A), embed<3>(B), embed<3>(C)}};
    if (ABC.det()<1e-3) return vec3(-1,1,1); // for a degenerate triangle generate negative coordinates, it will be thrown away by the rasterizator
    return ABC.invert_transpose() * embed<3>(P);
}

void triangle(const mat<4,3> &clipc, IShader &shader, TGAImage &image, std::vector<double> &zbuffer) {
    mat<3,4> pts  = (Viewport*clipc).transpose(); // transposed to ease access to each of the points
    mat<3,2> pts2;
    for (int i=0; i<3; i++) pts2[i] = proj<2>(pts[i]/pts[i][3]);

    vec2 bboxmin( std::numeric_limits<double>::max(),  std::numeric_limits<double>::max());
    vec2 bboxmax(-std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
    vec2 clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++)
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::max(0.,       std::min(bboxmin[j], pts2[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts2[i][j]));
        }
#pragma omp parallel for
    for (int x=(int)bboxmin.x; x<=(int)bboxmax.x; x++) {
        for (int y=(int)bboxmin.y; y<=(int)bboxmax.y; y++) {
            vec3 bc_screen  = barycentric(pts2[0], pts2[1], pts2[2], {(double)x, (double)y});
            vec3 bc_clip    = vec3(bc_screen.x/pts[0][3], bc_screen.y/pts[1][3], bc_screen.z/pts[2][3]);
            bc_clip = bc_clip/(bc_clip.x+bc_clip.y+bc_clip.z);
            double frag_depth = clipc[2]*bc_clip;
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0 || zbuffer[x+y*image.get_width()]>frag_depth) continue;
            TGAColor color;
            bool discard = shader.fragment(bc_clip, color);
            if (!discard) {
                zbuffer[x+y*image.get_width()] = frag_depth;
                image.set(x, y, color);
            }
        }
    }
}

