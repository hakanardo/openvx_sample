
#include <vx_cl.h>

#if defined(__IMAGE_SUPPORT__) // && defined(CL_USE_LUMINANCE)

__kernel void vx_single_threshold(read_only image2d_t in, uchar value, write_only image2d_t out) {
    int2 coord = (int2)(get_global_id(0), get_global_id(1));
    uint4 a = read_imageui(in, nearest_clamp, coord);
    uint4 b = clamp(a, value-1u, 255u);
    uint4 c = 0u;
}

#else

#endif
