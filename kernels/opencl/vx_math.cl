
#include <vx_cl.h>

#if defined(__IMAGE_SUPPORT__) // && defined(CL_USE_LUMINANCE)

// trunc(a+b)=c
__kernel void vx_add_u8_truncate(read_only image2d_t a, read_only image2d_t b, write_only image2d_t c) {
    int2 coord = (int2)(get_global_id(0), get_global_id(1));
    uint4 mask = (uint4)(255, 255, 255, 255);
    uint4 a4 = read_imageui(a, nearest_clamp, coord);
    uint4 b4 = read_imageui(b, nearest_clamp, coord);
    uint4 sum = (a4 + b4) & mask;
    write_imageui(c, coord, sum); 
}

// sat(a+b)=c
__kernel void vx_add_u8_saturate(read_only image2d_t a, read_only image2d_t b, write_only image2d_t c) {
    int2 coord = (int2)(get_global_id(0), get_global_id(1));
    uint4 a4 = read_imageui(a, nearest_clamp, coord);
    uint4 b4 = read_imageui(b, nearest_clamp, coord);
    uint4 sum = clamp(a4 + b4, 0u, 255u);
    write_imageui(c, coord, sum); 
}

// a+b=c
__kernel void vx_add_s16(read_only image2d_t a, read_only image2d_t b, write_only image2d_t c) {
    int2 coord = (int2)(get_global_id(0), get_global_id(1));
    uint4 a4 = read_imageui(a, nearest_clamp, coord);
    uint4 b4 = read_imageui(b, nearest_clamp, coord);
    int4 ai4 = convert_int4(a4);
    int4 bi4 = convert_int4(b4);
    int4 sum = ai4 + bi4;
    write_imagei(c, coord, sum); 
}

// a-b=c
__kernel void vx_sub_u8(read_only image2d_t a, read_only image2d_t b, write_only image2d_t c) {
    int2 coord = (int2)(get_global_id(0), get_global_id(1));
    uint4 a4 = read_imageui(a, nearest_clamp, coord);
    uint4 b4 = read_imageui(b, nearest_clamp, coord);
    int4 ai4 = convert_int4(a4);
    int4 bi4 = convert_int4(b4);
    int4 sum = ai4 - bi4;
    write_imagei(c, coord, sum); 

}

#else

#endif
