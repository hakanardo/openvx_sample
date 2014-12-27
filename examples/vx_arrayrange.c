/*
 * Copyright (c) 2013-2014 The Khronos Group Inc.
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

#include <VX/vx.h>

vx_array example_array_of_custom_type_and_initialization(vx_context context)
{
    //! [array define]
    typedef struct _mystruct {
        vx_uint32 some_uint;
        vx_float64 some_double;
    } mystruct;
#define MY_NUM_ITEMS (10)
    vx_enum mytype = vxRegisterUserStruct(context, sizeof(mystruct));
    vx_array array = vxCreateArray(context, mytype, MY_NUM_ITEMS);
    //! [array define]
    //! [array query]
    vx_size num_items = 0ul;
    vxQueryArray(array, VX_ARRAY_ATTRIBUTE_NUMITEMS, &num_items, sizeof(num_items));
    //! [array query]
    {
        //! [array range]
        vx_size i, stride = 0ul;
        void *base = NULL;
        /* access entire array at once */
        vxAccessArrayRange(array, 0, num_items, &stride, &base, VX_READ_AND_WRITE);
        for (i = 0; i < num_items; i++)
        {
            vxArrayItem(mystruct, base, i, stride).some_uint += i;
            vxArrayItem(mystruct, base, i, stride).some_double = 3.14f;
        }
        vxCommitArrayRange(array, 0, num_items, base);
        //! [array range]

        //! [array subaccess]
        /* access each array item individually */
        for (i = 0; i < num_items; i++)
        {
            mystruct *myptr = NULL;
            vxAccessArrayRange(array, i, i+1, &stride, (void **)&myptr, VX_READ_AND_WRITE);
            myptr->some_uint += 1;
            myptr->some_double = 3.14f;
            vxCommitArrayRange(array, i, i+1, (void *)myptr);
        }
        //! [array subaccess]
    }
    return array;
}
