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

#define LOG_TAG "Kernel_JNI"
#include "openvx_jni.h"

namespace android
{
    //**************************************************************************
    // LOCAL VARIABLES
    //**************************************************************************

    //**************************************************************************
    // EXPORTED FUNCTIONS
    //**************************************************************************
    static void GetByName(JNIEnv *env, jobject obj, jlong c, jstring name)
    {
        const char *str = env->GetStringUTFChars(name, NULL);
        vx_context context = (vx_context)c;
        vx_kernel k = vxGetKernelByName(context, (vx_char *)str);
        env->ReleaseStringUTFChars(name, str);
        SetHandle(env, obj, KernelClass, parentName, c);
        SetHandle(env, obj, KernelClass, handleName, (jlong)k);
    }

    static void GetByEnum(JNIEnv *env, jobject obj, jlong c, jint kenum)
    {
        vx_context context = (vx_context)c;
        vx_kernel k = vxGetKernelByEnum(context, kenum);
        SetHandle(env, obj, KernelClass, parentName, c);
        SetHandle(env, obj, KernelClass, handleName, (jlong)k);
    }

    static void Release(JNIEnv *env, jobject obj)
    {
        vx_kernel k = (vx_kernel)GetHandle(env, obj, KernelClass, handleName);
        vxReleaseKernel(&k);
        SetHandle(env, obj, KernelClass, handleName, 0);
    }

    static jint getNumParameters(JNIEnv *env, jobject obj)
    {
        vx_kernel k = (vx_kernel)GetHandle(env, obj, KernelClass, handleName);
        jint value = 0;
        vx_status status = vxQueryKernel(k, VX_KERNEL_ATTRIBUTE_PARAMETERS, &value, sizeof(value));
        if (status != VX_SUCCESS)
            value = 0;
        return value;
    }

    static jstring GetName(JNIEnv *env, jobject obj)
    {
        vx_kernel k = (vx_kernel)GetHandle(env, obj, KernelClass, handleName);
        vx_char name[VX_MAX_KERNEL_NAME];
        vx_status status = vxQueryKernel(k, VX_KERNEL_ATTRIBUTE_NAME, name, sizeof(name));
        if (status == VX_SUCCESS)
            return env->NewStringUTF(name);
        else
            return env->NewStringUTF("");
    }

    static JNINativeMethod method_table[] = {
        // { name,                          signature,                 function_pointer }
        { "get",                            "(JLjava/lang/String;)V",  (void *)GetByName },
        { "get",                            "(JI)V",                   (void *)GetByEnum },
        { "release",                        "()V",                     (void *)Release },
        { "getNumParameters",               "()I",                     (void *)getNumParameters },
        { "name",                           "()Ljava/lang/String;",    (void *)GetName },
    };

    int register_org_khronos_OpenVX_Kernel(JNIEnv *env)
    {
        PrintJNITable(LOG_TAG, KernelClass, method_table, NELEM(method_table));
        return jniRegisterNativeMethods(env, KernelClass, method_table, NELEM(method_table));
    }
};



