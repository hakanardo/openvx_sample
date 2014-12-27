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

#define LOG_TAG "Reference_JNI"
#include "openvx_jni.h"

namespace android
{
    //**************************************************************************
    // LOCAL VARIABLES
    //**************************************************************************

    //**************************************************************************
    // EXPORTED FUNCTIONS
    //**************************************************************************

    static jint getType(JNIEnv *env, jobject obj)
    {
        vx_reference r = (vx_reference)GetHandle(env, obj, ReferenceClass, handleName);
        vx_enum e;
        vxQueryReference(r, VX_REF_ATTRIBUTE_TYPE, &e, sizeof(e));
        return (jint)e;
    }

    static jint getCount(JNIEnv *env, jobject obj)
    {
        vx_reference r = (vx_reference)GetHandle(env, obj, ReferenceClass, handleName);
        vx_uint32 c;
        vxQueryReference(r, VX_REF_ATTRIBUTE_COUNT, &c, sizeof(c));
        return (jint)c;
    }

    static JNINativeMethod method_table[] = {
        // { name,                         signature,                 function_pointer }
        { "getType",                       "()I",                   (void *)getType },
        { "getCount",                      "()I",                   (void *)getCount },
    };

    int register_org_khronos_OpenVX_Reference(JNIEnv *env)
    {
        PrintJNITable(LOG_TAG, ReferenceClass, method_table, NELEM(method_table));
        return jniRegisterNativeMethods(env, ReferenceClass, method_table, NELEM(method_table));
    }
};



