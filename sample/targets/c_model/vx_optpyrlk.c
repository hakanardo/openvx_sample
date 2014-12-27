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

/*!
 * \file
 * \brief The Absolute Difference Kernel.
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_helper.h>

#include <vx_internal.h>
#include <c_model.h>

typedef struct _vx_keypoint_t_optpyrlk_internal {
    vx_float32 x;                 /*!< \brief The x coordinate. */
    vx_float32 y;                 /*!< \brief The y coordinate. */
    vx_float32 strength;        /*!< \brief The strength of the keypoint. */
    vx_float32 scale;           /*!< \brief Unused field reserved for future use. */
    vx_float32 orientation;     /*!< \brief Unused field reserved for future use. */
    vx_int32 tracking_status;   /*!< \brief A zero indicates a lost point. */
    vx_float32 error;           /*!< \brief An tracking method specific error. */
} vx_keypoint_t_optpyrlk_internal;

#define  INT_ROUND(x,n)     (((x) + (1 << ((n)-1))) >> (n))

static vx_status LKTracker(
        const vx_image prevImg, const vx_image prevDerivIx, const vx_image prevDerivIy, const vx_image nextImg,
        const vx_array prevPts, vx_array nextPts,
        vx_scalar winSize_s, vx_scalar criteria_s,
        vx_uint32 level,vx_scalar epsilon,
        vx_scalar num_iterations)
{
    vx_status status = VX_FAILURE;

    vx_size winSize;
    vxAccessScalarValue(winSize_s,&winSize);
    vx_size halfWin = winSize*0.5f;
    vx_size list_length,list_indx;
    const vx_image I = prevImg;
    const vx_image J = nextImg;
    const vx_image derivIx = prevDerivIx;
    const vx_image derivIy = prevDerivIy;

    int j;
    vx_enum termination_Criteria;

    vx_image IWinBuf;
    vx_image derivIWinBuf_x;
    vx_image derivIWinBuf_y;

    vx_df_image derivIx_format = 0;
    vx_df_image J_format = 0;
    vx_df_image derivIy_format = 0;
    vx_df_image I_format = 0;
    vx_rectangle_t rect;

    void *derivIx_base = 0,*J_base = 0,*derivIy_base = 0,*I_base = 0;
    vx_imagepatch_addressing_t  derivIx_addr,J_addr,derivIy_addr,I_addr;


    void *IWinBuf_base = 0,*derivIWinBuf_x_base = 0,*derivIWinBuf_y_base = 0;
    vx_df_image IWinBuf_format = 0;
    vx_df_image derivIWinBuf_x_format = 0;
    vx_df_image derivIWinBuf_y_format = 0;
    vx_imagepatch_addressing_t  IWinBuf_addr,derivIWinBuf_x_addr,derivIWinBuf_y_addr;

    vx_size prevPts_stride = 0;
    vx_size nextPts_stride = 0;
    void *prevPtsFirstItem = NULL;
    void *nextPtsFirstItem = NULL;
    vxQueryArray(prevPts,VX_ARRAY_ATTRIBUTE_NUMITEMS,&list_length,sizeof(list_length));
    vxAccessArrayRange(prevPts, 0, list_length, &prevPts_stride, &prevPtsFirstItem, VX_READ_ONLY);
    vxAccessArrayRange(nextPts, 0, list_length, &nextPts_stride, &nextPtsFirstItem, VX_READ_AND_WRITE);

    vx_keypoint_t_optpyrlk_internal *nextPt_item = (vx_keypoint_t_optpyrlk_internal*)nextPtsFirstItem;
    vx_keypoint_t_optpyrlk_internal *prevPt_item = (vx_keypoint_t_optpyrlk_internal*)prevPtsFirstItem;


    int num_iterations_i;
    vx_float32 epsilon_f;

    vxAccessScalarValue(num_iterations,&num_iterations_i);
    vxAccessScalarValue(epsilon,&epsilon_f);
    vxAccessScalarValue(criteria_s,&termination_Criteria);

    vx_context context_lk_internal = vxCreateContext();

    IWinBuf = vxCreateImage(context_lk_internal,winSize,winSize,VX_DF_IMAGE_S16);
    derivIWinBuf_x = vxCreateImage(context_lk_internal,winSize,winSize,VX_DF_IMAGE_S16);
    derivIWinBuf_y = vxCreateImage(context_lk_internal,winSize,winSize,VX_DF_IMAGE_S16);

    vxInitImage((vx_image_t*)IWinBuf,winSize,winSize,VX_DF_IMAGE_S16);
    vxAllocateImage((vx_image_t*)IWinBuf);
    vxInitImage((vx_image_t*)derivIWinBuf_x,winSize,winSize,VX_DF_IMAGE_S16);
    vxAllocateImage((vx_image_t*)derivIWinBuf_x);
    vxInitImage((vx_image_t*)derivIWinBuf_y,winSize,winSize,VX_DF_IMAGE_S16);
    vxAllocateImage((vx_image_t*)derivIWinBuf_y);



    vxQueryImage(derivIx, VX_IMAGE_ATTRIBUTE_FORMAT, &derivIx_format, sizeof(derivIx_format));
    vxQueryImage(J, VX_IMAGE_ATTRIBUTE_FORMAT, &J_format, sizeof(J_format));
    vxQueryImage(derivIy, VX_IMAGE_ATTRIBUTE_FORMAT, &derivIy_format, sizeof(derivIy_format));
    vxQueryImage(I, VX_IMAGE_ATTRIBUTE_FORMAT, &I_format, sizeof(I_format));

    vxGetValidRegionImage(derivIx,&rect);
    status = VX_SUCCESS;

    status |= vxAccessImagePatch(derivIx, &rect, 0, &derivIx_addr, (void **)&derivIx_base,VX_READ_ONLY);
    status |= vxAccessImagePatch(J, &rect, 0, &J_addr, (void **)&J_base,VX_READ_ONLY);
    status |= vxAccessImagePatch(derivIy, &rect, 0, &derivIy_addr, (void **)&derivIy_base,VX_READ_ONLY);
    status |= vxAccessImagePatch(I, &rect, 0, &I_addr, (void **)&I_base,VX_READ_ONLY);

    vxQueryImage(IWinBuf, VX_IMAGE_ATTRIBUTE_FORMAT, &IWinBuf_format, sizeof(IWinBuf_format));
    vxQueryImage(derivIWinBuf_x, VX_IMAGE_ATTRIBUTE_FORMAT, &derivIWinBuf_x_format, sizeof(derivIWinBuf_x_format));
    vxQueryImage(derivIWinBuf_y, VX_IMAGE_ATTRIBUTE_FORMAT, &derivIWinBuf_y_format, sizeof(derivIWinBuf_y_format));

    vxGetValidRegionImage(IWinBuf,&rect);

    status |= vxAccessImagePatch(IWinBuf, &rect, 0, &IWinBuf_addr, (void **)&IWinBuf_base,VX_READ_AND_WRITE);
    status |= vxAccessImagePatch(derivIWinBuf_x, &rect, 0, &derivIWinBuf_x_addr, (void **)&derivIWinBuf_x_base,VX_READ_AND_WRITE);
    status |= vxAccessImagePatch(derivIWinBuf_y, &rect, 0, &derivIWinBuf_y_addr, (void **)&derivIWinBuf_y_base,VX_READ_AND_WRITE);



    for(list_indx=0;list_indx<list_length;list_indx++)
    {


        vx_keypoint_t_optpyrlk_internal nextPt,prevPt;


        vx_keypoint_t_optpyrlk_internal iprevPt, inextPt;

        prevPt.x = prevPt_item[list_indx].x;
        prevPt.y = prevPt_item[list_indx].y;

        // nextPt.x = prevPt.x;
        // nextPt.y = prevPt.y;
        nextPt.x = nextPt_item[list_indx].x;
        nextPt.y = nextPt_item[list_indx].y;

        prevPt.x -= halfWin;
        prevPt.y -= halfWin;
        nextPt.x -= halfWin;
        nextPt.y -= halfWin;

        iprevPt.x = floor(prevPt.x);
        iprevPt.y = floor(prevPt.y);

        if( iprevPt.x < 0 || iprevPt.x >= derivIx_addr.dim_x - winSize-1 ||
            iprevPt.y < 0 || iprevPt.y >= derivIx_addr.dim_y - winSize-1 )
        {
            if( level == 0 )
            {
                nextPt.tracking_status = 0;
                nextPt.error = 0;
            }
            continue;
        }

        float a = prevPt.x - iprevPt.x;
        float b = prevPt.y - iprevPt.y;
        const int W_BITS = 14, W_BITS1 = 14;
        const float FLT_SCALE = 1.f/(1 << 20);
        int iw00 = (int)(((1.f - a)*(1.f - b)*(1 << W_BITS))+0.5f);
        int iw01 = (int)((a*(1.f - b)*(1 << W_BITS))+0.5f);
        int iw10 = (int)(((1.f - a)*b*(1 << W_BITS))+0.5f);
        int iw11 = (1 << W_BITS) - iw00 - iw01 - iw10;

        int dstep_x = (int)(derivIx_addr.stride_y)/2;
        int dstep_y = (int)(derivIy_addr.stride_y)/2;
        int stepJ = (int)(J_addr.stride_y);
        int stepI = (int)(I_addr.stride_y);
        double A11 = 0, A12 = 0, A22 = 0;


        // extract the patch from the first image, compute covariation matrix of derivatives
        int x, y;
        for( y = 0; y < winSize; y++ )
        {
            unsigned char *src = (unsigned char*)vxFormatImagePatchAddress2d(I_base, iprevPt.x, y + iprevPt.y, &I_addr);
            short *dsrc_x = (short*)vxFormatImagePatchAddress2d(derivIx_base, iprevPt.x, y + iprevPt.y, &derivIx_addr);
            short *dsrc_y = (short*)vxFormatImagePatchAddress2d(derivIy_base, iprevPt.x, y + iprevPt.y, &derivIy_addr);


            short* Iptr = (short*)vxFormatImagePatchAddress2d(IWinBuf_base,0, y,&IWinBuf_addr);
            short* dIptr_x = (short*)vxFormatImagePatchAddress2d(derivIWinBuf_x_base,0, y,&derivIWinBuf_x_addr);
            short* dIptr_y = (short*)vxFormatImagePatchAddress2d(derivIWinBuf_y_base,0, y,&derivIWinBuf_y_addr);

            x = 0;


            for( ; x < winSize; x++, dsrc_x ++, dsrc_y ++)
            {
                int ival = INT_ROUND(src[x]*iw00 + src[x+1]*iw01 +
                                      src[x+stepI]*iw10 + src[x+stepI+1]*iw11, W_BITS1-5);
                int ixval = INT_ROUND(dsrc_x[0]*iw00 + dsrc_x[1]*iw01 +
                                       dsrc_x[dstep_x]*iw10 + dsrc_x[dstep_x+1]*iw11, W_BITS1);
                int iyval = INT_ROUND(dsrc_y[0]*iw00 + dsrc_y[1]*iw01 + dsrc_y[dstep_y]*iw10 +
                                       dsrc_y[dstep_y+1]*iw11, W_BITS1);

                Iptr[x] = (short)ival;
                dIptr_x[x] = (short)ixval;
                dIptr_y[x] = (short)iyval;

                A11 += (float)(ixval*ixval);
                A12 += (float)(ixval*iyval);
                A22 += (float)(iyval*iyval);
            }
        }


        A11 *= FLT_SCALE;
        A12 *= FLT_SCALE;
        A22 *= FLT_SCALE;

        double D = A11*A22 - A12*A12;
        float minEig = (A22 + A11 - sqrt((A11-A22)*(A11-A22) +
                        4.f*A12*A12))/(2*winSize*winSize);


        if( minEig < 1.0e-04F || D < 1.0e-07F  )
        {
            if( level == 0  )
                nextPt.tracking_status = 0;
            continue;
        }

        D = 1.f/D;

        // nextPt.x -= halfWin;
        // nextPt.y -= halfWin;
        float prevDelta_x=0.0f,prevDelta_y=0.0f;

        j = 0;
        while(j < num_iterations_i || termination_Criteria == VX_TERM_CRITERIA_EPSILON)
        {
            inextPt.x = floor(nextPt.x);
            inextPt.y = floor(nextPt.y);

            if( inextPt.x < 0 || inextPt.x >= J_addr.dim_x-winSize-1 ||
               inextPt.y < 0 || inextPt.y >= J_addr.dim_y- winSize-1 )
            {
                if( level == 0  )
                    nextPt.tracking_status = 0;
                break;
            }

            a = nextPt.x - inextPt.x;
            b = nextPt.y - inextPt.y;
            iw00 = (int)(((1.f - a)*(1.f - b)*(1 << W_BITS))+0.5);
            iw01 = (int)((a*(1.f - b)*(1 << W_BITS))+0.5);
            iw10 = (int)(((1.f - a)*b*(1 << W_BITS))+0.5);
            iw11 = (1 << W_BITS) - iw00 - iw01 - iw10;
            double b1 = 0, b2 = 0;

            for( y = 0; y < winSize; y++ )
            {
                unsigned char* Jptr = (unsigned char*)vxFormatImagePatchAddress2d(J_base, inextPt.x, y + inextPt.y, &J_addr);
                short* Iptr = (short*)vxFormatImagePatchAddress2d(IWinBuf_base,0, y,&IWinBuf_addr);
                short* dIptr_x = (short*)vxFormatImagePatchAddress2d(derivIWinBuf_x_base,0, y,&derivIWinBuf_x_addr);
                short* dIptr_y = (short*)vxFormatImagePatchAddress2d(derivIWinBuf_y_base,0, y,&derivIWinBuf_y_addr);

                x = 0;


                for( ; x < winSize; x++)
                {
                    int diff = INT_ROUND(Jptr[x]*iw00 + Jptr[x+1]*iw01 +
                                          Jptr[x+stepJ]*iw10 + Jptr[x+stepJ+1]*iw11,
                                          W_BITS1-5) - Iptr[x];
                    b1 += (float)(diff*dIptr_x[x]);
                    b2 += (float)(diff*dIptr_y[x]);
                }
            }


            b1 *= FLT_SCALE;
            b2 *= FLT_SCALE;

            float delta_x = (float)((A12*b2 - A22*b1) * D);
            float delta_y = (float)((A12*b1 - A11*b2) * D);
            //printf("lvl=%d pt=%d iter=%d   -> (%f, %f) += (%f, %f)\n", (int)level, (int)list_indx, (int)j, nextPt.x, nextPt.y, delta_x, delta_y);
            //delta = -delta;

            nextPt.x += delta_x;
            nextPt.y += delta_y;
            nextPt_item[list_indx].x = nextPt.x + halfWin;
            nextPt_item[list_indx].y = nextPt.y + halfWin;

            if( (delta_x*delta_x + delta_y*delta_y) <= epsilon_f && (termination_Criteria == VX_TERM_CRITERIA_EPSILON || termination_Criteria == VX_TERM_CRITERIA_BOTH))
                break;

            if( j > 0 && abs(delta_x + prevDelta_x) < 0.01 &&
               abs(delta_y + prevDelta_y) < 0.01 )
            {
                nextPt_item[list_indx].x -= delta_x*0.5f;
                nextPt_item[list_indx].y -= delta_y*0.5f;
                break;
            }
            prevDelta_x = delta_x;
            prevDelta_y = delta_y;
            j++;
        }
    }
    status |= vxCommitArrayRange(prevPts, 0, list_length,prevPtsFirstItem);
    status |= vxCommitArrayRange(nextPts, 0, list_length,nextPtsFirstItem);

    status |= vxCommitImagePatch(IWinBuf, &rect, 0, &IWinBuf_addr, (void *)IWinBuf_base);
    status |= vxCommitImagePatch(derivIWinBuf_x, &rect, 0, &derivIWinBuf_x_addr, (void *)derivIWinBuf_x_base);
    status |= vxCommitImagePatch(derivIWinBuf_y, &rect, 0, &derivIWinBuf_y_addr, (void *)derivIWinBuf_y_base);

    status |= vxCommitImagePatch(derivIx, &rect, 0, &derivIx_addr, (void *)derivIx_base);
    status |= vxCommitImagePatch(J, &rect, 0, &J_addr, (void *)J_base);
    status |= vxCommitImagePatch(derivIy, &rect, 0, &derivIy_addr, (void *)derivIy_base);
    status |= vxCommitImagePatch(I, &rect, 0, &I_addr, (void *)I_base);

    vxFreeImage((vx_image_t*)IWinBuf);
    vxFreeImage((vx_image_t*)derivIWinBuf_x);
    vxFreeImage((vx_image_t*)derivIWinBuf_y);

    vxReleaseImage(&IWinBuf);
    vxReleaseImage(&derivIWinBuf_x);
    vxReleaseImage(&derivIWinBuf_y);

    vxReleaseContext(&context_lk_internal);

    return VX_SUCCESS;
}


static vx_status VX_CALLBACK vxOpticalFlowPyrLKKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_FAILURE;
    if (num == 10)
    {
        vx_size maxLevel, list_length,list_indx;
        vx_int32 level;
        vx_pyramid old_pyramid = (vx_pyramid)parameters[0];
        vx_pyramid new_pyramid = (vx_pyramid)parameters[1];
        vx_array prevPts =  (vx_array)parameters[2];
        vx_array estimatedPts =  (vx_array)parameters[3];
        vx_array nextPts =  (vx_array)parameters[4];
        vx_scalar termination =  (vx_scalar)parameters[5];
        vx_scalar epsilon =  (vx_scalar)parameters[6];
        vx_scalar num_iterations =  (vx_scalar)parameters[7];
        vx_scalar use_initial_estimate =  (vx_scalar)parameters[8];
        vx_scalar window_dimension =  (vx_scalar)parameters[9];
        vx_size prevPts_stride = 0;
        vx_size estimatedPts_stride = 0;
        vx_size nextPts_stride = 0;
        void *prevPtsFirstItem;
        void *initialPtsFirstItem;
        void *nextPtsFirstItem;

        vx_bool use_initial_estimate_b;
        vx_float32 pyramid_scale;
        vx_keypoint_t_optpyrlk_internal *prevPt;
        vx_keypoint_t *initialPt = NULL;
        vx_keypoint_t_optpyrlk_internal *nextPt = NULL;

        vxAccessScalarValue(use_initial_estimate,&use_initial_estimate_b);
        vxQueryPyramid(old_pyramid, VX_PYRAMID_ATTRIBUTE_LEVELS, &maxLevel, sizeof(maxLevel));
        vxQueryPyramid(old_pyramid, VX_PYRAMID_ATTRIBUTE_SCALE , &pyramid_scale, sizeof(pyramid_scale));

        // the point in the list are in integer coordinates of x,y
        // The algorithm demand  a float so we convert the point first.
        vxQueryArray(prevPts, VX_ARRAY_ATTRIBUTE_NUMITEMS, &list_length,sizeof(list_length));

        for( level = maxLevel; level > 0; level-- )
        {
            vx_image old_image = vxGetPyramidLevel(old_pyramid, level-1);
            vx_image new_image = vxGetPyramidLevel(new_pyramid, level-1);
            vx_rectangle_t rec;

            prevPtsFirstItem = NULL;
            vxAccessArrayRange(prevPts, 0, list_length, &prevPts_stride, &prevPtsFirstItem, VX_READ_AND_WRITE);
            prevPt = (vx_keypoint_t_optpyrlk_internal*)prevPtsFirstItem;

            if (level == maxLevel)
            {
                if (use_initial_estimate_b)
                {
                    vx_size list_length2 = 0;
                    vxQueryArray(estimatedPts, VX_ARRAY_ATTRIBUTE_NUMITEMS, &list_length2,sizeof(list_length2));
                    if (list_length2 != list_length)
                        return VX_ERROR_INVALID_PARAMETERS;

                    initialPtsFirstItem = NULL;
                    vxAccessArrayRange(estimatedPts, 0, list_length, &estimatedPts_stride, &initialPtsFirstItem, VX_READ_ONLY);
                    initialPt = (vx_keypoint_t *)initialPtsFirstItem;
                }
                else
                {
                    initialPt = (vx_keypoint_t *)prevPt;
                }

                vxTruncateArray(nextPts, 0);
            }
            else
            {
                nextPtsFirstItem = NULL;
                vxAccessArrayRange(nextPts, 0, list_length, &nextPts_stride, &nextPtsFirstItem, VX_READ_AND_WRITE);
                nextPt = (vx_keypoint_t_optpyrlk_internal*)nextPtsFirstItem;
            }

            for(list_indx=0;list_indx<list_length;list_indx++)
            {
                if(level == maxLevel)
                {
                    (prevPt)->x = (((vx_keypoint_t*)prevPt))->x *(float)((pow(pyramid_scale,level-1)));
                    (prevPt)->y = (((vx_keypoint_t*)prevPt))->y *(float)((pow(pyramid_scale,level-1)));
                    if (use_initial_estimate_b)
                    {
                        vx_keypoint_t_optpyrlk_internal keypoint;
                        keypoint.x = (initialPt)->x*(float)((pow(pyramid_scale,level-1)));
                        keypoint.y = (initialPt)->y*(float)((pow(pyramid_scale,level-1)));
                        keypoint.strength = (initialPt)->strength;
                        keypoint.tracking_status = (initialPt)->tracking_status;
                        keypoint.error = (initialPt)->error;
                        status |= vxAddArrayItems(nextPts, 1, &keypoint, 0);
                    }
                    else
                    {
                        vx_keypoint_t_optpyrlk_internal keypoint;
                        keypoint.x = (prevPt)->x;
                        keypoint.y = (prevPt)->y;
                        keypoint.strength = 0;
                        keypoint.tracking_status = 1;
                        keypoint.error = 10000;
                        status |= vxAddArrayItems(nextPts, 1, &keypoint, 0);
                    }
                }
                else
                {
                    (prevPt)->x = (prevPt)->x *(1.0f/pyramid_scale);
                    (prevPt)->y = (prevPt)->y *(1.0f/pyramid_scale);
                    (nextPt)->x = (nextPt)->x *(1.0f/pyramid_scale);
                    (nextPt)->y = (nextPt)->y *(1.0f/pyramid_scale);
                    nextPt++;
                }
                prevPt++;
                initialPt++;

            }
            vxCommitArrayRange(prevPts, 0, list_length,prevPtsFirstItem);

            if (level !=maxLevel)
            {
                vxCommitArrayRange(nextPts, 0, list_length, nextPtsFirstItem);
            }
            else if (use_initial_estimate_b)
            {
                vxCommitArrayRange(estimatedPts, 0, list_length,initialPtsFirstItem);
            }

            vxGetValidRegionImage(old_image,&rec);
            // printf("%ux%u - %ux%u\n", rec.start_x, rec.start_y, rec.end_x, rec.end_y);

            {
                vx_context context_scharr = vxCreateContext();
                if(context_scharr)
                {
                    vx_uint32 width,height,n;

                    width = rec.end_x - rec.start_x;
                    height = rec.end_y - rec.start_y;

                    vx_image shar_images[] = {
                            old_image,     // index 0: Input 1
                            vxCreateImage(context_scharr, width, height, VX_DF_IMAGE_S16),     // index 1: Input 2
                            vxCreateImage(context_scharr, width, height, VX_DF_IMAGE_S16),       // index 2: Get Y channel
                    };
                    vx_graph graph_scharr = vxCreateGraph(context_scharr);
                    vx_status extras_status = vxLoadKernels(context_scharr, "openvx-extras");
                    if (graph_scharr && extras_status == VX_SUCCESS)
                    {
                        vx_node scharr_nodes[] = {
                            vxScharr3x3Node(graph_scharr, shar_images[0], shar_images[1], shar_images[2]),
                        };
                        status = vxVerifyGraph(graph_scharr);
                        if (status == VX_SUCCESS)
                        {

                            status = vxProcessGraph(graph_scharr);

                            status |= LKTracker(old_image, shar_images[1], shar_images[2],
                                        new_image, prevPts, nextPts,
                                        window_dimension, termination, level-1,
                                        epsilon,num_iterations);
                        }
                        for (n = 0; n < dimof(scharr_nodes); n++)
                        {
                            vxReleaseNode(&scharr_nodes[n]);
                        }
                        vxReleaseGraph(&graph_scharr);
                    }
                    for (n = 1; n < dimof(shar_images); n++)
                    {
                        vxReleaseImage(&shar_images[n]);
                    }
                    vxReleaseContext(&context_scharr);

                }
            }

            vxReleaseImage(&new_image);
            vxReleaseImage(&old_image);
        }

        nextPtsFirstItem = NULL;
        vxAccessArrayRange(nextPts, 0, list_length, &nextPts_stride, &nextPtsFirstItem, VX_READ_AND_WRITE);
        nextPt = (vx_keypoint_t_optpyrlk_internal*)nextPtsFirstItem;

        prevPt = (vx_keypoint_t_optpyrlk_internal*)prevPtsFirstItem;
        initialPt = (vx_keypoint_t *)initialPtsFirstItem;
        nextPt = (vx_keypoint_t_optpyrlk_internal*)nextPtsFirstItem;
        for(list_indx=0;list_indx<list_length;list_indx++)
        {
            (((vx_keypoint_t*)nextPt))->x = (vx_int32)((nextPt)->x);
            (((vx_keypoint_t*)nextPt))->y = (vx_int32)((nextPt)->y);
            (((vx_keypoint_t*)prevPt))->x = (vx_int32)((prevPt)->x);
            (((vx_keypoint_t*)prevPt))->y = (vx_int32)((prevPt)->y);
            nextPt++;
            prevPt++;
        }

        vxCommitArrayRange(nextPts, 0, list_length,nextPtsFirstItem);

       return status;
    }
    return VX_ERROR_INVALID_PARAMETERS;
}

static vx_status VX_CALLBACK vxOpticalFlowPyrLKInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 0 || index == 1)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_pyramid input = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
            if (input)
            {
                vx_size level = 0;
                vxQueryPyramid(input, VX_PYRAMID_ATTRIBUTE_LEVELS, &level, sizeof(level));
                if (level !=0)
                {
                    status = VX_SUCCESS;
                }
                vxReleasePyramid(&input);
            }
            vxReleaseParameter(&param);
        }
    }
    else if (index == 2 || index == 3)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_array arr = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &arr, sizeof(arr));
            if (arr)
            {
                vx_enum item_type = 0;
                vxQueryArray(arr, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &item_type, sizeof(item_type));
                if (item_type == VX_TYPE_KEYPOINT)
                {
                    status = VX_SUCCESS;
                }
                vxReleaseArray(&arr);
            }
            vxReleaseParameter(&param);
        }
    }
    else if (index == 5)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
         if (param)
         {
             vx_scalar sens = 0;
             status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &sens, sizeof(sens));
             if ((status == VX_SUCCESS) && (sens))
             {
                 vx_enum type = 0;
                 vxQueryScalar(sens, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
                 if (type == VX_TYPE_ENUM)
                 {
                     status = VX_SUCCESS;
                 }
                 else
                 {
                     status = VX_ERROR_INVALID_TYPE;
                 }
                 vxReleaseScalar(&sens);
             }
             vxReleaseParameter(&param);
         }
    }
    else if (index ==6)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
         if (param)
         {
             vx_scalar sens = 0;
             status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &sens, sizeof(sens));
             if ((status == VX_SUCCESS) && (sens))
             {
                 vx_enum type = 0;
                 vxQueryScalar(sens, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
                 if (type == VX_TYPE_FLOAT32)
                 {
                     status = VX_SUCCESS;
                 }
                 else
                 {
                     status = VX_ERROR_INVALID_TYPE;
                 }
                 vxReleaseScalar(&sens);
             }
             vxReleaseParameter(&param);
         }
    }
    else if (index ==7)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
         if (param)
         {
             vx_scalar sens = 0;
             status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &sens, sizeof(sens));
             if ((status == VX_SUCCESS) && (sens))
             {
                 vx_enum type = 0;
                 vxQueryScalar(sens, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
                 if (type == VX_TYPE_UINT32)
                 {
                     status = VX_SUCCESS;
                 }
                 else
                 {
                     status = VX_ERROR_INVALID_TYPE;
                 }
                 vxReleaseScalar(&sens);
             }
             vxReleaseParameter(&param);
         }
    }
    else if (index ==8)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_scalar sens = 0;
            status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &sens, sizeof(sens));
            if ((status == VX_SUCCESS) && (sens))
            {
                vx_enum type = 0;
                vxQueryScalar(sens, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
                if (type == VX_TYPE_BOOL)
                {
                    status = VX_SUCCESS;
                }
                else
                {
                    status = VX_ERROR_INVALID_TYPE;
                }
                vxReleaseScalar(&sens);
            }
            vxReleaseParameter(&param);
        }
    }
    else if (index == 9)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
         if (param)
         {
             vx_scalar sens = 0;
             status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &sens, sizeof(sens));
             if ((status == VX_SUCCESS) && (sens))
             {
                 vx_enum type = 0;
                 vxQueryScalar(sens, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
                 if (type == VX_TYPE_SIZE)
                 {
                     status = VX_SUCCESS;
                 }
                 else
                 {
                     status = VX_ERROR_INVALID_TYPE;
                 }
                 vxReleaseScalar(&sens);
             }
             vxReleaseParameter(&param);
         }
    }
    return status;
}

static vx_status VX_CALLBACK vxOpticalFlowPyrLKOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 4)
    {
        vx_array arr = 0;
        vx_size capacity = 0;
        vx_parameter param = vxGetParameterByIndex(node, 2);
        vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &arr, sizeof(arr));
        vxQueryArray(arr, VX_ARRAY_ATTRIBUTE_CAPACITY, &capacity, sizeof(capacity));

        ptr->type = VX_TYPE_ARRAY;
        ptr->dim.array.item_type = VX_TYPE_KEYPOINT;
        ptr->dim.array.capacity = capacity;

        status = VX_SUCCESS;

        vxReleaseArray(&arr);
        vxReleaseParameter(&param);
    }
    return status;
}

static vx_param_description_t optpyrlk_kernel_params[] = {
    {VX_INPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
};

#ifdef __cplusplus
extern "C"
#endif
vx_kernel_description_t optpyrlk_kernel = {
    VX_KERNEL_OPTICAL_FLOW_PYR_LK,
    "org.khronos.openvx.opticalflow_pyr_lk",
    vxOpticalFlowPyrLKKernel,
    optpyrlk_kernel_params, dimof(optpyrlk_kernel_params),
    vxOpticalFlowPyrLKInputValidator,
    vxOpticalFlowPyrLKOutputValidator,
};
