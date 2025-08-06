/*
 * Software 3D Renderer for Baremetal OS
 * Features:
 * - Fixed-point 3D rotation
 * - Z-buffer
 * - Lighting with directional light
 * - ARGB8888 output
 *
 * Author: ChatGPT
 */

 #include <stdint.h>
 #include <stddef.h>
 
 #define WIDTH 1024
 #define HEIGHT 768
 #define FP_SHIFT 16
 #define FP_ONE (1 << FP_SHIFT)
 #define NUM_VERTS 8
 #define NUM_TRIS 12
 
 // Fixed-point types and macros
 typedef int32_t fixed;
 #define INT_TO_FIXED(x) ((x) << FP_SHIFT)
 #define FIXED_TO_INT(x) ((x) >> FP_SHIFT)
 #define FMUL(a, b) (((int64_t)(a) * (b)) >> FP_SHIFT)
 #define FDIV(a, b) (((int64_t)(a) << FP_SHIFT) / (b))
 
 // Vec2/Vec3 Types
 typedef struct { fixed x, y; } Vec2;
 typedef struct { fixed x, y, z; } Vec3;
 typedef struct { int a, b, c; } Triangle;
 
 // Global framebuffer (must be mapped properly)
 uint32_t* framebuffer = (uint32_t*) 0xE0000000;
 fixed zbuffer[WIDTH * HEIGHT];
 
 // Math utilities
 fixed isin_table[360];
 fixed icos_table[360];
 
 void init_trig_tables() {
     for (int i = 0; i < 360; i++) {
         double rad = i * 3.14159265 / 180.0;
         isin_table[i] = (fixed)(sin(rad) * FP_ONE);
         icos_table[i] = (fixed)(cos(rad) * FP_ONE);
     }
 }
 
 fixed isin(int deg) { return isin_table[deg % 360]; }
 fixed icos(int deg) { return icos_table[deg % 360]; }
 
 Vec3 vec3_sub(Vec3 a, Vec3 b) {
     return (Vec3){ a.x - b.x, a.y - b.y, a.z - b.z };
 }
 
 Vec3 vec3_cross(Vec3 a, Vec3 b) {
     return (Vec3){
         FMUL(a.y, b.z) - FMUL(a.z, b.y),
         FMUL(a.z, b.x) - FMUL(a.x, b.z),
         FMUL(a.x, b.y) - FMUL(a.y, b.x)
     };
 }
 
 fixed vec3_dot(Vec3 a, Vec3 b) {
     return FMUL(a.x, b.x) + FMUL(a.y, b.y) + FMUL(a.z, b.z);
 }
 
 Vec3 vec3_normalize(Vec3 v) {
     fixed len = vec3_dot(v, v);
     if (len == 0) return v;
     fixed inv = FDIV(FP_ONE, len);
     return (Vec3){ FMUL(v.x, inv), FMUL(v.y, inv), FMUL(v.z, inv) };
 }
 
 Vec2 project(Vec3 v) {
     fixed z_inv = FDIV(FP_ONE, v.z);
     Vec2 p = {
         FMUL(v.x, FMUL(INT_TO_FIXED(WIDTH) / 2, z_inv)) + INT_TO_FIXED(WIDTH / 2),
         INT_TO_FIXED(HEIGHT / 2) - FMUL(v.y, FMUL(INT_TO_FIXED(HEIGHT) / 2, z_inv))
     };
     return p;
 }
 
 void put_pixel(int x, int y, uint32_t color, fixed depth) {
     if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
     int idx = y * WIDTH + x;
     if (depth < zbuffer[idx]) {
         zbuffer[idx] = depth;
         framebuffer[idx] = color;
     }
 }
 
 void draw_filled_triangle(Vec3 a, Vec3 b, Vec3 c, uint32_t color) {
     Vec2 pa = project(a);
     Vec2 pb = project(b);
     Vec2 pc = project(c);
 
     int minX = FIXED_TO_INT(pa.x < pb.x ? (pa.x < pc.x ? pa.x : pc.x) : (pb.x < pc.x ? pb.x : pc.x));
     int maxX = FIXED_TO_INT(pa.x > pb.x ? (pa.x > pc.x ? pa.x : pc.x) : (pb.x > pc.x ? pb.x : pc.x));
     int minY = FIXED_TO_INT(pa.y < pb.y ? (pa.y < pc.y ? pa.y : pc.y) : (pb.y < pc.y ? pb.y : pc.y));
     int maxY = FIXED_TO_INT(pa.y > pb.y ? (pa.y > pc.y ? pa.y : pc.y) : (pb.y > pc.y ? pb.y : pc.y));
 
     for (int y = minY; y <= maxY; y++) {
         for (int x = minX; x <= maxX; x++) {
             fixed px = INT_TO_FIXED(x);
             fixed py = INT_TO_FIXED(y);
 
             fixed w0 = (px - pb.x)*(pa.y - pb.y) - (py - pb.y)*(pa.x - pb.x);
             fixed w1 = (px - pc.x)*(pb.y - pc.y) - (py - pc.y)*(pb.x - pc.x);
             fixed w2 = (px - pa.x)*(pc.y - pa.y) - (py - pa.y)*(pc.x - pa.x);
 
             if ((w0 | w1 | w2) >= 0) {
                 fixed depth = (a.z + b.z + c.z) / 3;
                 put_pixel(x, y, color, depth);
             }
         }
     }
 }
 
 Vec3 rotate(Vec3 v, int ax, int ay) {
     fixed sin_x = isin(ax), cos_x = icos(ax);
     fixed sin_y = isin(ay), cos_y = icos(ay);
 
     fixed x1 = FMUL(v.x, cos_y) - FMUL(v.z, sin_y);
     fixed z1 = FMUL(v.x, sin_y) + FMUL(v.z, cos_y);
     fixed y1 = FMUL(v.y, cos_x) - FMUL(z1, sin_x);
     fixed z2 = FMUL(v.y, sin_x) + FMUL(z1, cos_x);
 
     return (Vec3){ x1, y1, z2 + INT_TO_FIXED(5) }; // move cube into view
 }
 
 // Cube data
 Vec3 cube_vertices[NUM_VERTS] = {
     { -FP_ONE, -FP_ONE, -FP_ONE },
     {  FP_ONE, -FP_ONE, -FP_ONE },
     {  FP_ONE,  FP_ONE, -FP_ONE },
     { -FP_ONE,  FP_ONE, -FP_ONE },
     { -FP_ONE, -FP_ONE,  FP_ONE },
     {  FP_ONE, -FP_ONE,  FP_ONE },
     {  FP_ONE,  FP_ONE,  FP_ONE },
     { -FP_ONE,  FP_ONE,  FP_ONE }
 };
 
 Triangle cube_tris[NUM_TRIS] = {
     {0, 1, 2}, {0, 2, 3},
     {1, 5, 6}, {1, 6, 2},
     {5, 4, 7}, {5, 7, 6},
     {4, 0, 3}, {4, 3, 7},
     {3, 2, 6}, {3, 6, 7},
     {4, 5, 1}, {4, 1, 0}
 };
 
 void render_cube(int angle_x, int angle_y) {
     Vec3 light_dir = vec3_normalize((Vec3){ INT_TO_FIXED(0), INT_TO_FIXED(0), -FP_ONE });
 
     for (int i = 0; i < NUM_TRIS; i++) {
         Vec3 a = rotate(cube_vertices[cube_tris[i].a], angle_x, angle_y);
         Vec3 b = rotate(cube_vertices[cube_tris[i].b], angle_x, angle_y);
         Vec3 c = rotate(cube_vertices[cube_tris[i].c], angle_x, angle_y);
 
         Vec3 ab = vec3_sub(b, a);
         Vec3 ac = vec3_sub(c, a);
         Vec3 normal = vec3_normalize(vec3_cross(ab, ac));
         
         fixed brightness = vec3_dot(normal, light_dir);
         if (brightness <= 0) continue; // backface cull
 
         uint8_t intensity = brightness >> (FP_SHIFT - 8);
         uint32_t color = (0xFF << 24) | (intensity << 16) | (intensity << 8) | intensity;
         draw_filled_triangle(a, b, c, color);
     }
 }
 
 void clear_zbuffer() {
     for (int i = 0; i < WIDTH * HEIGHT; i++) {
         zbuffer[i] = INT_TO_FIXED(9999);
     }
 }
 
 // Main loop example
 void render_frame(int frame_num) {
     clear_zbuffer();
     render_cube(frame_num % 360, (frame_num * 2) % 360);
 }
 