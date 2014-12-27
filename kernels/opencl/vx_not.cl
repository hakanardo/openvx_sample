
#include <vx_cl.h>

#if defined(CL_USE_IMAGES)

__kernel void vx_not(read_only image2d_t a, write_only image2d_t b) {
    int2 coord = (get_global_id(0), get_global_id(1));
    write_imageui(b, coord, ~read_imageui(a, nearest_clamp, coord));
}

#else

__kernel void vx_not(int aw, int ah, int asx, int asy, __global void *a, 
                     int bw, int bh, int bsx, int bsy, __global void *b)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    vxImagePixel(uchar, b, x, y, bsx, bsy) = ~vxImagePixel(uchar, a, x, y, asx, asy);
}

#endif
