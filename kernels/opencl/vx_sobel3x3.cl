#include <vx_cl.h>

/* Sobel Gradient Operators (for reference) 
    short gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    short gy[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1},
    };
*/

#if defined(__IMAGE_SUPPORT__) && defined(CL_USE_LUMINANCE)

__kernel void vx_sobel3x3(sampler_t sampler, 
                          read_only image2d_t src,
                          write_only image2d_t dst_gx,
                          write_only image2d_t dst_gy) 
{   
#if defined(CLOOPY)
    int x = get_global_id(0);
    int y = get_global_id(1);
    int4 sx = 0, sy = 0;
    for (int j = -1, k = 0; j <= 1; j++, k++) {
        for (int i = -1, l = 0; i <= 1; i++, l++) {
            uint4 pixel = read_imageui(src, sampler, (int2)(x+i,y+j));
            sx.s0 += gx[k][l] * pixel.s0;
            sy.s0 += gy[k][l] * pixel.s0;
        }
    }
    write_imagei(dst_gx, (int2)(x,y), sx);
    write_imagei(dst_gy, (int2)(x,y), sy);
#else
    int x = get_global_id(0);
    int y = get_global_id(1);
    short sx = 0;
    short sy = 0;
    
    sx -= read_imageui(src, sampler, (int2)(x-1,y-1)).s0;
    sx -= read_imageui(src, sampler, (int2)(x-1,y-0)).s0 * 2;
    sx -= read_imageui(src, sampler, (int2)(x-1,y+1)).s0;
    sx += read_imageui(src, sampler, (int2)(x+1,y-1)).s0;
    sx += read_imageui(src, sampler, (int2)(x+1,y-0)).s0 * 2;
    sx += read_imageui(src, sampler, (int2)(x+1,y+1)).s0;
    write_imagei(dst_gx, (int2)(x,y), (int4)(sx, sx, sx, 1));    
    sy -= read_imageui(src, sampler, (int2)(x-1,y-1)).s0;
    sy -= read_imageui(src, sampler, (int2)(x-0,y-1)).s0 * 2;
    sy -= read_imageui(src, sampler, (int2)(x+1,y-1)).s0;
    sy += read_imageui(src, sampler, (int2)(x-1,y+1)).s0;
    sy += read_imageui(src, sampler, (int2)(x-0,y+1)).s0 * 2;
    sy += read_imageui(src, sampler, (int2)(x+1,y+1)).s0;
    write_imagei(dst_gy, (int2)(x,y), (int4)(sy, sy, sy, 1));
#endif
}

#else

__kernel void vx_sobel3x3(
    __global vx_cl_imagepatch_addressing_t *sa,
    __global void *src,
    __global vx_cl_imagepatch_addressing_t *gxa,
    __global void *gx,
    __global vx_cl_imagepatch_addressing_t *gya,
    __global void *gy)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    int sx = 0, sy = 0;
    if (gx) {
        sx -= (uint)vxImagePixel(uchar, src, x-1, y-1, sa);
        sx -= (uint)vxImagePixel(uchar, src, x-1, y-0, sa) * 2;
        sx -= (uint)vxImagePixel(uchar, src, x-1, y+1, sa);
        sx += (uint)vxImagePixel(uchar, src, x+1, y-1, sa);
        sx += (uint)vxImagePixel(uchar, src, x+1, y-0, sa) * 2;
        sx += (uint)vxImagePixel(uchar, src, x+1, y+1, sa);
        vxImagePixel(short, gx, x, y, gxa) = sx;
    }
    if (gy) {
        sy -= (uint)vxImagePixel(uchar, src, x-1, y-1, sa);
        sy -= (uint)vxImagePixel(uchar, src, x-0, y-1, sa) * 2;
        sy -= (uint)vxImagePixel(uchar, src, x+1, y-1, sa);
        sy += (uint)vxImagePixel(uchar, src, x-1, y-1, sa);
        sy += (uint)vxImagePixel(uchar, src, x-0, y-1, sa) * 2;
        sy += (uint)vxImagePixel(uchar, src, x+1, y-1, sa);
        vxImagePixel(short, gy, x, y, gya) = sy;
    }
}
#endif
