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

#define LOG_TAG "Scalar_JNI"
#include "openvx_jni.h"

namespace android
{
    //**************************************************************************
    // LOCAL VARIABLES
    //**************************************************************************

    //**************************************************************************
    // EXPORTED FUNCTIONS
    //**************************************************************************
    static void InitializeByte(JNIEnv *env, jobject obj, jlong c, jbyte v)
    {
        vx_context context = (vx_context)c;
        vx_scalar s = vxCreateScalar(context, VX_TYPE_INT8, &v);
        SetHandle(env, obj, ScalarClass, parentName, c);
        SetHandle(env, obj, ScalarClass, handleName, (jlong)s);
    }

    static void InitializeShort(JNIEnv *env, jobject obj, jlong c, jshort v)
    {
        vx_context context = (vx_context)c;
        vx_scalar s = vxCreateScalar(context, VX_TYPE_INT16, &v);
        SetHandle(env, obj, ScalarClass, parentName, c);
        SetHandle(env, obj, ScalarClass, handleName, (jlong)s);
    }

    static void InitializeInt(JNIEnv *env, jobject obj, jlong c, jint v)
    {
        vx_context context = (vx_context)c;
        vx_scalar s = vxCreateScalar(context, VX_TYPE_INT32, &v);
        SetHandle(env, obj, ScalarClass, parentName, c);
        SetHandle(env, obj, ScalarClass, handleName, (jlong)s);
    }

        static void InitializeLong(JNIEnv *env, jobject obj, jlong c, jlong v)
    {
        vx_context context = (vx_context)c;
        vx_scalar s = vxCreateScalar(context, VX_TYPE_INT64, &v);
        SetHandle(env, obj, ScalarClass, parentName, c);
        SetHandle(env, obj, ScalarClass, handleName, (jlong)s);
    }

        static void InitializeFloat(JNIEnv *env, jobject obj, jlong c, jfloat v)
    {
        vx_context context = (vx_context)c;
        vx_scalar s = vxCreateScalar(context, VX_TYPE_FLOAT32, &v);
        SetHandle(env, obj, ScalarClass, parentName, c);
        SetHandle(env, obj, ScalarClass, handleName, (jlong)s);
    }

        static void InitializeDouble(JNIEnv *env, jobject obj, jlong c, jdouble v)
    {
        vx_context context = (vx_context)c;
        vx_scalar s = vxCreateScalar(context, VX_TYPE_FLOAT64, &v);
        SetHandle(env, obj, ScalarClass, parentName, c);
        SetHandle(env, obj, ScalarClass, handleName, (jlong)s);
    }

    static void Finalize(JNIEnv *env, jobject obj)
    {
        vx_scalar s = (vx_scalar)GetHandle(env, obj, ScalarClass, handleName);
        vxReleaseScalar(&s);
        SetHandle(env, obj, ScalarClass, handleName, 0);
    }

    static jbyte getByteValue(JNIEnv *env, jobject obj)
    {
        vx_scalar s = (vx_scalar)GetHandle(env, obj, ScalarClass, handleName);
        jbyte v = 0;
        vxAccessScalarValue(s, &v);
        return v;
    }

    static jshort getShortValue(JNIEnv *env, jobject obj)
    {
        vx_scalar s = (vx_scalar)GetHandle(env, obj, ScalarClass, handleName);
        jshort v = 0;
        vxAccessScalarValue(s, &v);
        return v;
    }

    static jint getIntValue(JNIEnv *env, jobject obj)
    {
        vx_scalar s = (vx_scalar)GetHandle(env, obj, ScalarClass, handleName);
        jint v = 0;
        vxAccessScalarValue(s, &v);
        return v;
    }

    static jlong getLongValue(JNIEnv *env, jobject obj)
    {
        vx_scalar s = (vx_scalar)GetHandle(env, obj, ScalarClass, handleName);
        jlong v = 0;
        vxAccessScalarValue(s, &v);
        return v;
    }

    static jfloat getFloatValue(JNIEnv *env, jobject obj)
    {
        vx_scalar s = (vx_scalar)GetHandle(env, obj, ScalarClass, handleName);
        jfloat v = 0;
        vxAccessScalarValue(s, &v);
        return v;
    }

    static jdouble getDoubleValue(JNIEnv *env, jobject obj)
    {
        vx_scalar s = (vx_scalar)GetHandle(env, obj, ScalarClass, handleName);
        jdouble v = 0;
        vxAccessScalarValue(s, &v);
        return v;
    }

    static void setByteValue(JNIEnv *env, jobject obj, jbyte v)
    {
        vx_scalar s = (vx_scalar)GetHandle(env, obj, ScalarClass, handleName);
        vxCommitScalarValue(s, &v);
    }

    static void setShortValue(JNIEnv *env, jobject obj, jshort v)
    {
        vx_scalar s = (vx_scalar)GetHandle(env, obj, ScalarClass, handleName);
        vxCommitScalarValue(s, &v);
    }

    static void setIntValue(JNIEnv *env, jobject obj, jint v)
    {
        vx_scalar s = (vx_scalar)GetHandle(env, obj, ScalarClass, handleName);
        vxCommitScalarValue(s, &v);
    }

    static void setLongValue(JNIEnv *env, jobject obj, jlong v)
    {
        vx_scalar s = (vx_scalar)GetHandle(env, obj, ScalarClass, handleName);
        vxCommitScalarValue(s, &v);
    }

    static void setFloatValue(JNIEnv *env, jobject obj, jfloat v)
    {
        vx_scalar s = (vx_scalar)GetHandle(env, obj, ScalarClass, handleName);
        vxCommitScalarValue(s, &v);
    }

    static void setDoubleValue(JNIEnv *env, jobject obj, jdouble v)
    {
        vx_scalar s = (vx_scalar)GetHandle(env, obj, ScalarClass, handleName);
        vxCommitScalarValue(s, &v);
    }

    static JNINativeMethod method_table[] = {
        // { name,                         signature,                 function_pointer }
        { "create",                        "(JB)V",                   (void *)InitializeByte },
        { "create",                        "(JS)V",                   (void *)InitializeShort },
        { "create",                        "(JI)V",                   (void *)InitializeInt },
        { "create",                        "(JJ)V",                   (void *)InitializeLong },
        { "create",                        "(JF)V",                   (void *)InitializeFloat },
        { "create",                        "(JD)V",                   (void *)InitializeDouble },

        { "destroy",                       "()V",                     (void *)Finalize },

        { "getByteValue",                  "()B",                     (void *)getByteValue },
        { "getShortValue",                 "()S",                     (void *)getShortValue },
        { "getIntValue",                   "()I",                     (void *)getIntValue },
        { "getLongValue",                  "()J",                     (void *)getLongValue },
        { "getFloatValue",                 "()F",                     (void *)getFloatValue },
        { "getDoubleValue",                "()D",                     (void *)getDoubleValue },
        { "setValue",                      "(B)V",                    (void *)setByteValue },
        { "setValue",                      "(S)V",                    (void *)setShortValue },
        { "setValue",                      "(I)V",                    (void *)setIntValue },
        { "setValue",                      "(J)V",                    (void *)setLongValue },
        { "setValue",                      "(F)V",                    (void *)setFloatValue },
        { "setValue",                      "(D)V",                    (void *)setDoubleValue },
    };

    int register_org_khronos_OpenVX_Scalar(JNIEnv *env)
    {
        PrintJNITable(LOG_TAG, ScalarClass, method_table, NELEM(method_table));
        return jniRegisterNativeMethods(env, ScalarClass, method_table, NELEM(method_table));
    }
};



