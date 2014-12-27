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

#define LOG_TAG "Image_JNI"
#include "openvx_jni.h"

namespace android
{
    //**************************************************************************
    // LOCAL VARIABLES
    //**************************************************************************

    //**************************************************************************
    // EXPORTED FUNCTIONS
    //**************************************************************************
    static void Initialize(JNIEnv *env, jobject obj, jlong c, jint w, jint h, jint f)
    {
        vx_context context = (vx_context)c;
        vx_image i = vxCreateImage(context, w, h, f);
        SetHandle(env, obj, ImageClass, parentName, c);
        SetHandle(env, obj, ImageClass, handleName, (jlong)i);
    }

    static void Finalize(JNIEnv *env, jobject obj)
    {
        vx_image i = (vx_image)GetHandle(env, obj, ImageClass, handleName);
        vxReleaseImage(&i);
        SetHandle(env, obj, ImageClass, handleName, 0);
    }

    static jint getWidth(JNIEnv *env, jobject obj)
    {
        vx_image i = (vx_image)GetHandle(env, obj, ImageClass, handleName);
        jint value = 0;
        vx_status status = vxQueryImage(i, VX_IMAGE_ATTRIBUTE_WIDTH, &value, sizeof(value));
        if (status != VX_SUCCESS)
            value = 0;
        return value;
    }

    static jint getHeight(JNIEnv *env, jobject obj)
    {
        vx_image i = (vx_image)GetHandle(env, obj, ImageClass, handleName);
        jint value = 0;
        vx_status status = vxQueryImage(i, VX_IMAGE_ATTRIBUTE_HEIGHT, &value, sizeof(value));
        if (status != VX_SUCCESS)
            value = 0;
        return value;
    }

    static jint getFormat(JNIEnv *env, jobject obj)
    {
        vx_image i = (vx_image)GetHandle(env, obj, ImageClass, handleName);
        jint value = 0;
        vx_status status = vxQueryImage(i, VX_IMAGE_ATTRIBUTE_HEIGHT, &value, sizeof(value));
        if (status != VX_SUCCESS)
            value = 0;
        return value;
    }

    static jint computePatchSize(JNIEnv *env, jobject obj, jint sx, jint sy, jint ex, jint ey, jint p)
    {
        vx_image i = (vx_image)GetHandle(env, obj, ImageClass, handleName);
        vx_rectangle_t rect = {sx, sy, ex, ey};
        jint size = vxComputeImagePatchSize(i, &rect, p);
        return size;
    }

    static jint getPatch(JNIEnv *env, jobject obj, jint sx, jint sy, jint ex, jint ey, jint p, jbyteArray data)
    {
        void *patch = NULL;
        vx_status status = VX_FAILURE;
        vx_imagepatch_addressing_t addr;
        vx_image i = (vx_image)GetHandle(env, obj, ImageClass, handleName);
        jbyte *ptr = env->GetByteArrayElements(data, NULL);
        if (ptr)
        {
            vx_rectangle_t rect = {sx, sy, ex, ey};
            status = vxAccessImagePatch(i, &rect, p, &addr, (void **)&ptr, VX_READ_AND_WRITE);
            env->ReleaseByteArrayElements(data, ptr, 0);
        }
        return (jint)status;
    }

    static jint setPatch(JNIEnv *env, jobject obj, jint sx, jint sy, jint ex, jint ey, jint p, jbyteArray data)
    {
        void *patch = NULL;
        vx_status status = VX_FAILURE;
        vx_imagepatch_addressing_t addr;
        vx_image i = (vx_image)GetHandle(env, obj, ImageClass, handleName);
        jbyte *ptr = env->GetByteArrayElements(data, NULL);
        if (ptr)
        {
            vx_rectangle_t rect = {sx, sy, ex, ey};
            status = vxCommitImagePatch(i, &rect, p, &addr, ptr);
            env->ReleaseByteArrayElements(data, ptr, 0);
        }
        return (jint)status;
    }

    static JNINativeMethod method_table[] = {
        // { name,                         signature,                 function_pointer }
        { "create",                        "(JIII)V",                 (void *)Initialize },
        { "destroy",                       "()V",                     (void *)Finalize },

        { "getWidth",                      "()I",                     (void *)getWidth },
        { "getHeight",                     "()I",                     (void *)getHeight },
        { "getFormat",                     "()I",                     (void *)getFormat },

        { "computePatchSize",              "(IIIII)I",                (void *)computePatchSize },
        { "getPatch",                      "(IIIII[B)I",              (void *)getPatch },
        { "setPatch",                      "(IIIII[B)I",              (void *)setPatch },

    };

    int register_org_khronos_OpenVX_Image(JNIEnv *env)
    {
        PrintJNITable(LOG_TAG, ImageClass, method_table, NELEM(method_table));
        return jniRegisterNativeMethods(env, ImageClass, method_table, NELEM(method_table));
    }
};



