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

#define LOG_TAG "Buffer_JNI"
#include "openvx_jni.h"

namespace android
{
    //**************************************************************************
    // LOCAL VARIABLES
    //**************************************************************************

    //**************************************************************************
    // EXPORTED FUNCTIONS
    //**************************************************************************
    static jboolean Initialize(JNIEnv *env, jobject obj, jlong c, jint us, jint nu)
    {
        vx_context context = (vx_context)c;
        vx_buffer b = vxCreateBuffer(context, us, nu);
        SetHandle(env, obj, BufferClass, parentName, c);
        SetHandle(env, obj, BufferClass, handleName, (jlong)b);
        return true;
    }

    static void Finalize(JNIEnv *env, jobject obj)
    {
        vx_buffer b = (vx_buffer)GetHandle(env, obj, BufferClass, handleName);
        vxReleaseBuffer(&b);
        SetHandle(env, obj, BufferClass, handleName, 0);
    }

    static jint getUnitSize(JNIEnv *env, jobject obj)
    {
        vx_buffer b = (vx_buffer)GetHandle(env, obj, BufferClass, handleName);
        jint value = 0;
        vx_status status = vxQueryBuffer(b, VX_BUFFER_ATTRIBUTE_UNITSIZE, &value, sizeof(value));
        if (status != VX_SUCCESS)
            value = 0;
        return value;
    }

    static jint getNumUnits(JNIEnv *env, jobject obj)
    {
        vx_buffer b = (vx_buffer)GetHandle(env, obj, BufferClass, handleName);
        jint value = 0;
        vx_status status = vxQueryBuffer(b, VX_BUFFER_ATTRIBUTE_NUMUNITS, &value, sizeof(value));
        if (status != VX_SUCCESS)
            value = 0;
        return value;
    }

    static jint computeRangeSize(JNIEnv *env, jobject obj, jint start, jint end)
    {
        vx_buffer b = (vx_buffer)GetHandle(env, obj, BufferClass, handleName);
        return vxComputeBufferRangeSize(b, start, end);
    }

    static jint getRangeByte(JNIEnv *env, jobject obj, jint start, jint end, jbyteArray data)
    {
        vx_status status = VX_FAILURE;
        vx_buffer b = (vx_buffer)GetHandle(env, obj, BufferClass, handleName);
        jbyte *ptr = env->GetByteArrayElements(data, NULL);
        if (ptr)
        {
            status = vxAccessBufferRange(b, start, end, (void **)&ptr, VX_READ_AND_WRITE);
            env->ReleaseByteArrayElements(data, ptr, 0);
        }
        return (jint)status;
    }

    static jint setRangeByte(JNIEnv *env, jobject obj, jint start, jint end, jbyteArray data)
    {
        vx_status status = VX_FAILURE;
        vx_buffer b = (vx_buffer)GetHandle(env, obj, BufferClass, handleName);
        jbyte *ptr = env->GetByteArrayElements(data, NULL);
        if (ptr)
        {
            status = vxCommitBufferRange(b, start, end, ptr);
            env->ReleaseByteArrayElements(data, ptr, 0);
        }
        return (jint)status;
    }

    static jint getRangeShort(JNIEnv *env, jobject obj, jint start, jint end, jshortArray data)
    {
        vx_status status = VX_FAILURE;
        vx_buffer b = (vx_buffer)GetHandle(env, obj, BufferClass, handleName);
        jshort *ptr = env->GetShortArrayElements(data, NULL);
        if (ptr)
        {
            status = vxAccessBufferRange(b, start, end, (void **)&ptr, VX_READ_AND_WRITE);
            env->ReleaseShortArrayElements(data, ptr, 0);
        }
        return (jint)status;
    }

    static jint setRangeShort(JNIEnv *env, jobject obj, jint start, jint end, jshortArray data)
    {
        vx_status status = VX_FAILURE;
        vx_buffer b = (vx_buffer)GetHandle(env, obj, BufferClass, handleName);
        jshort *ptr = env->GetShortArrayElements(data, NULL);
        if (ptr)
        {
            status = vxCommitBufferRange(b, start, end, ptr);
            env->ReleaseShortArrayElements(data, ptr, 0);
        }
        return (jint)status;
    }

    static jint getRangeInt(JNIEnv *env, jobject obj, jint start, jint end, jintArray data)
    {
        vx_status status = VX_FAILURE;
        vx_buffer b = (vx_buffer)GetHandle(env, obj, BufferClass, handleName);
        jint *ptr = env->GetIntArrayElements(data, NULL);
        if (ptr)
        {
            status = vxAccessBufferRange(b, start, end, (void **)&ptr, VX_READ_AND_WRITE);
            env->ReleaseIntArrayElements(data, ptr, 0);
        }
        return (jint)status;
    }

    static jint setRangeInt(JNIEnv *env, jobject obj, jint start, jint end, jintArray data)
    {
        vx_status status = VX_FAILURE;
        vx_buffer b = (vx_buffer)GetHandle(env, obj, BufferClass, handleName);
        jint *ptr = env->GetIntArrayElements(data, NULL);
        if (ptr)
        {
            status = vxCommitBufferRange(b, start, end, ptr);
            env->ReleaseIntArrayElements(data, ptr, 0);
        }
        return (jint)status;
    }

    static jint getRangeLong(JNIEnv *env, jobject obj, jint start, jint end, jlongArray data)
    {
        vx_status status = VX_FAILURE;
        vx_buffer b = (vx_buffer)GetHandle(env, obj, BufferClass, handleName);
        jlong *ptr = env->GetLongArrayElements(data, NULL);
        if (ptr)
        {
            status = vxAccessBufferRange(b, start, end, (void **)&ptr, VX_READ_AND_WRITE);
            env->ReleaseLongArrayElements(data, ptr, 0);
        }
        return (jint)status;
    }

    static jint setRangeLong(JNIEnv *env, jobject obj, jint start, jint end, jlongArray data)
    {
        vx_status status = VX_FAILURE;
        vx_buffer b = (vx_buffer)GetHandle(env, obj, BufferClass, handleName);
        jlong *ptr = env->GetLongArrayElements(data, NULL);
        if (ptr)
        {
            status = vxCommitBufferRange(b, start, end, ptr);
            env->ReleaseLongArrayElements(data, ptr, 0);
        }
        return (jint)status;
    }

    static jint getRangeFloat(JNIEnv *env, jobject obj, jint start, jint end, jfloatArray data)
    {
        vx_status status = VX_FAILURE;
        vx_buffer b = (vx_buffer)GetHandle(env, obj, BufferClass, handleName);
        jfloat *ptr = env->GetFloatArrayElements(data, NULL);
        if (ptr)
        {
            status = vxAccessBufferRange(b, start, end, (void **)&ptr, VX_READ_AND_WRITE);
            env->ReleaseFloatArrayElements(data, ptr, 0);
        }
        return (jint)status;
    }

    static jint setRangeFloat(JNIEnv *env, jobject obj, jint start, jint end, jfloatArray data)
    {
        vx_status status = VX_FAILURE;
        vx_buffer b = (vx_buffer)GetHandle(env, obj, BufferClass, handleName);
        jfloat *ptr = env->GetFloatArrayElements(data, NULL);
        if (ptr)
        {
            status = vxCommitBufferRange(b, start, end, ptr);
            env->ReleaseFloatArrayElements(data, ptr, 0);
        }
        return (jint)status;
    }

    static jint getRangeDouble(JNIEnv *env, jobject obj, jint start, jint end, jdoubleArray data)
    {
        vx_status status = VX_FAILURE;
        vx_buffer b = (vx_buffer)GetHandle(env, obj, BufferClass, handleName);
        jdouble *ptr = env->GetDoubleArrayElements(data, NULL);
        if (ptr)
        {
            status = vxAccessBufferRange(b, start, end, (void **)&ptr, VX_READ_AND_WRITE);
            env->ReleaseDoubleArrayElements(data, ptr, 0);
        }
        return (jint)status;
    }

    static jint setRangeDouble(JNIEnv *env, jobject obj, jint start, jint end, jdoubleArray data)
    {
        vx_status status = VX_FAILURE;
        vx_buffer b = (vx_buffer)GetHandle(env, obj, BufferClass, handleName);
        jdouble *ptr = env->GetDoubleArrayElements(data, NULL);
        if (ptr)
        {
            status = vxCommitBufferRange(b, start, end, ptr);
            env->ReleaseDoubleArrayElements(data, ptr, 0);
        }
        return (jint)status;
    }

    static JNINativeMethod method_table[] = {
        // { name,                         signature,                 function_pointer }
        { "create",                        "(JII)V",                  (void *)Initialize },
        { "destroy",                       "()V",                     (void *)Finalize },

        { "getNumUnits",                   "()I",                     (void *)getNumUnits },
        { "getUnitSize",                   "()I",                     (void *)getUnitSize },

        { "computeRangeSize",             "(II)I",                    (void *)computeRangeSize },
        { "getRange",                     "(II[B)I",                  (void *)getRangeByte },
        { "getRange",                     "(II[S)I",                  (void *)getRangeShort },
        { "getRange",                     "(II[I)I",                  (void *)getRangeInt },
        { "getRange",                     "(II[J)I",                  (void *)getRangeLong },
        { "getRange",                     "(II[F)I",                  (void *)getRangeFloat },
        { "getRange",                     "(II[D)I",                  (void *)getRangeDouble },
        { "setRange",                     "(II[B)I",                  (void *)setRangeByte },
        { "setRange",                     "(II[S)I",                  (void *)setRangeShort },
        { "setRange",                     "(II[I)I",                  (void *)setRangeInt },
        { "setRange",                     "(II[J)I",                  (void *)setRangeLong },
        { "setRange",                     "(II[F)I",                  (void *)setRangeFloat },
        { "setRange",                     "(II[D)I",                  (void *)setRangeDouble },
    };

    int register_org_khronos_OpenVX_Buffer(JNIEnv *env)
    {
        PrintJNITable(LOG_TAG, BufferClass, method_table, NELEM(method_table));
        return jniRegisterNativeMethods(env, BufferClass, method_table, NELEM(method_table));
    }
};



