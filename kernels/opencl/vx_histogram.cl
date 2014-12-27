
#include <vx_cl.h>

#if defined(CL_USE_IMAGES)

__kernel void vx_histogram_8(read_only image2d_t src,
                             uint num, uint range, uint offset, uint winsize, __global int *hist)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    uint4 pix = read_imageui(src, nearest_clamp, (int2)(x,y));
    vxIncFrequency(hist, pix.s0, offset, range, winsize); 
}
#else

__kernel void vx_histogram_8(int w, int h, int sx, int sy, __global uchar *src,
                             uint num, uint range, uint offset, uint winsize, __global int *hist)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    uchar pix = vxImagePixel(uchar, src, x, y, sx, sy);
    vxIncFrequency(hist, pix, offset, range, winsize); 
}
#endif