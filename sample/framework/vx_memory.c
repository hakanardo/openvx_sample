/*
 * Copyright (c) 2012-2014 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */

#include <vx_internal.h>

vx_bool vxFreeMemory(vx_context context, vx_memory_t *memory)
{
    if (memory->allocated == vx_true_e)
    {
        vx_int32 p = 0u;
        vxPrintMemory(memory);
        for (p = 0; p < memory->nptrs; p++)
        {
            if (memory->ptrs[p])
            {
                VX_PRINT(VX_ZONE_INFO, "Freeing %p\n", memory->ptrs[p]);
#if defined(EXPERIMENTAL_USE_OPENCL)
                clReleaseMemObject(memory->hdls[p]);
#endif
                free(memory->ptrs[p]);
                vxDestroySem(&memory->locks[p]);
                memory->ptrs[p] = NULL;
            }
        }
        memory->allocated = vx_false_e;
    }
    return memory->allocated;
}


vx_bool vxAllocateMemory(vx_context context, vx_memory_t *memory)
{
    if (memory->allocated == vx_false_e)
    {
        vx_int32 d = 0, p = 0;
        VX_PRINT(VX_ZONE_INFO, "Allocating %u pointers of %u dimensions each.\n", memory->nptrs, memory->ndims);
        memory->allocated = vx_true_e;
        for (p = 0; p < memory->nptrs; p++)
        {
            vx_size size = sizeof(vx_uint8);
            /* channel is a declared size, don't assume */
            if (memory->strides[p][VX_DIM_C] != 0)
                size = (size_t)abs(memory->strides[p][VX_DIM_C]);
            for (d = 0; d < memory->ndims; d++)
            {
                memory->strides[p][d] = (vx_int32)size;
                size *= (vx_size)abs(memory->dims[p][d]);
            }
            /* don't presume that memory should be zeroed */
            memory->ptrs[p] = malloc(size);
            if (memory->ptrs[p] == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to allocated "VX_FMT_SIZE" bytes\n", size);
                /* unroll */
                memory->allocated = vx_false_e;
                for (p = p - 1; p >= 0; p--)
                {
                    VX_PRINT(VX_ZONE_INFO, "Freeing %p\n", memory->ptrs[p]);
                    free(memory->ptrs[p]);
                    memory->ptrs[p] = NULL;
                }
                break;
            }
            else
            {
                vxCreateSem(&memory->locks[p], 1);
                VX_PRINT(VX_ZONE_INFO, "Allocated %p for "VX_FMT_SIZE" bytes\n", memory->ptrs[p], size);
#if defined(EXPERIMENTAL_USE_OPENCL)
                {
                    cl_int err = 0;
                    if (memory->cl_type == CL_MEM_OBJECT_BUFFER) {
                        memory->hdls[p] = clCreateBuffer(context->global[0],
                                            CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,
                                            size,
                                            memory->ptrs[p],
                                            &err);
                        VX_PRINT(VX_ZONE_INFO, "Mapping %p to cl_buffer %p\n",
                                            memory->ptrs[p],
                                            memory->hdls[p]);
                    }
                    else if (memory->cl_type == CL_MEM_OBJECT_IMAGE2D) {
                        memory->hdls[p] = clCreateImage2D(context->global[0],
                                            CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,
                                            &memory->cl_format,
                                            memory->dims[p][VX_DIM_X],
                                            memory->dims[p][VX_DIM_Y],
                                            memory->strides[p][VX_DIM_Y],
                                            memory->ptrs[p], &err);
                        VX_PRINT(VX_ZONE_INFO, "Mapping %p to cl_image2d %p\n",
                                            memory->ptrs[p],
                                            memory->hdls[p]);
                    }
                    else {
                        VX_PRINT(VX_ZONE_WARNING, "Memory struct %p is neither buffer or image!\n", memory);
                    }
                }
#endif
            }
        }
        vxPrintMemory(memory);
    }
    return memory->allocated;
}

void vxPrintMemory(vx_memory_t *mem)
{
    vx_int32 d = 0, p = 0;
    for (p = 0; p < mem->nptrs; p++)
    {
        vx_bool gotlock = vxSemTryWait(&mem->locks[p]);
        if (gotlock == vx_true_e)
            vxSemPost(&mem->locks[p]);
        VX_PRINT(VX_ZONE_INFO, "ptr[%u]=%p %s\n", p, mem->ptrs[p],
                (gotlock==vx_true_e?"UNLOCKED":"LOCKED"));
        for (d = 0; d < mem->ndims; d++)
        {
            VX_PRINT(VX_ZONE_INFO, "\tdim[%u][%u]=%d strides[%u][%u]=%d\n", p, d, mem->dims[p][d], p, d, mem->strides[p][d]);
        }
    }
}

vx_size vxComputeMemorySize(vx_memory_t *memory, vx_uint32 p)
{
    return (memory->ndims == 0 ? 0 : (memory->dims[p][memory->ndims-1] * memory->strides[p][memory->ndims-1]));
}

