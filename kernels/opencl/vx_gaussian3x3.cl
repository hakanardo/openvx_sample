
#include <vx_cl.h>


#if defined(CL_USE_IMAGES)

__kernel void vx_gaussian3x3(read_only image2d_t src, write_only image2d_t dst)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    uint4 sum = 0;
    sum += 1*read_imageui(src, nearest_clamp, (int2)(x-1,y-1));
    sum += 2*read_imageui(src, nearest_clamp, (int2)(x+0,y-1));
    sum += 1*read_imageui(src, nearest_clamp, (int2)(x+1,y-1));
    sum += 2*read_imageui(src, nearest_clamp, (int2)(x-1,y+0));
    sum += 4*read_imageui(src, nearest_clamp, (int2)(x+0,y+0));
    sum += 2*read_imageui(src, nearest_clamp, (int2)(x+1,y+0));
    sum += 1*read_imageui(src, nearest_clamp, (int2)(x-1,y+1));
    sum += 2*read_imageui(src, nearest_clamp, (int2)(x+0,y+1));
    sum += 1*read_imageui(src, nearest_clamp, (int2)(x+1,y+1));
    sum /= 16;
    write_imageui(dst, (int2)(x,y), sum);
}

#else

__kernel void vx_gaussian3x3(int sw, int sh, int ssx, int ssy, __global void *src,
                             int dw, int dh, int dsx, int dsy, __global void *dst)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const size_t w = get_global_size(0);
    const size_t h = get_global_size(1);
    uint sum = 0;
    if (y == 0 || x == 0 || x == (w - 1) || y == (h - 1))
       return;
    sum += 1*(uint)vxImagePixel(uchar, src, x-1, y-1, ssx, ssy);
    sum += 2*(uint)vxImagePixel(uchar, src, x+0, y-1, ssx, ssy);
    sum += 1*(uint)vxImagePixel(uchar, src, x+1, y-1, ssx, ssy);
    sum += 2*(uint)vxImagePixel(uchar, src, x-1, y+0, ssx, ssy);
    sum += 4*(uint)vxImagePixel(uchar, src, x+0, y+0, ssx, ssy);
    sum += 2*(uint)vxImagePixel(uchar, src, x+1, y+0, ssx, ssy);
    sum += 1*(uint)vxImagePixel(uchar, src, x-1, y+1, ssx, ssy);
    sum += 2*(uint)vxImagePixel(uchar, src, x+0, y+1, ssx, ssy);
    sum += 1*(uint)vxImagePixel(uchar, src, x+1, y+1, ssx, ssy);
    sum /= 16;
    vxImagePixel(uchar, dst, x, y, dsx, dsy) = sum;
}

#endif
