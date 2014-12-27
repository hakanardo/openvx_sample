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

#define LOG_TAG "Graph_JNI"
#include "openvx_jni.h"

namespace android
{
    //**************************************************************************
    // LOCAL VARIABLES
    //**************************************************************************

    //**************************************************************************
    // EXPORTED FUNCTIONS
    //**************************************************************************
    static void Initialize(JNIEnv *env, jobject obj, jlong c)
    {
        vx_context context = (vx_context)c;
        vx_graph g = vxCreateGraph(context);
        SetHandle(env, obj, GraphClass, parentName, c);
        SetHandle(env, obj, GraphClass, handleName, (jlong)g);
    }

    static void Finalize(JNIEnv *env, jobject obj)
    {
        vx_graph g = (vx_graph)GetHandle(env, obj, GraphClass, handleName);
        vxReleaseGraph(&g);
        SetHandle(env, obj, GraphClass, handleName, 0);
    }

    static jint getNumNodes(JNIEnv *env, jobject obj)
    {
        vx_graph g = (vx_graph)GetHandle(env, obj, GraphClass, handleName);
        jint value = 0;
        vx_status status = vxQueryGraph(g, VX_GRAPH_ATTRIBUTE_NUMNODES, &value, sizeof(value));
        if (status != VX_SUCCESS)
            value = 0;
        return value;
    }

    static jobject createNode(JNIEnv *env, jobject obj, jlong k)
    {
        vx_graph g = (vx_graph)GetHandle(env, obj, GraphClass, handleName);
        jclass c = env->FindClass(NodeClass);
        jmethodID id = env->GetMethodID(c, "<init>", "(JJ)V");
        jobject node = env->NewObject(c, id, (jlong)g, k);
        return node;
    }

    static jint Verify(JNIEnv *env, jobject obj)
    {
        vx_graph g = (vx_graph)GetHandle(env, obj, GraphClass, handleName);
        vx_status status = vxVerifyGraph(g);
        return (jint)status;
    }

    static jint Process(JNIEnv *env, jobject obj)
    {
        vx_graph g = (vx_graph)GetHandle(env, obj, GraphClass, handleName);
        vx_status status = vxProcessGraph(g);
        return (jint)status;
    }

    static JNINativeMethod method_table[] = {
        // { name,                         signature,                 function_pointer }
        { "create",                        "(J)V",                    (void *)Initialize },
        { "destroy",                       "()V",                     (void *)Finalize },
        { "getNumNodes",                   "()I",                     (void *)getNumNodes },
        { "_createNode",                   "(J)"OBJECT(Node),         (void *)createNode },
        { "verify",                        "()I",                     (void *)Verify },
        { "process",                       "()I",                     (void *)Process },
    };

    int register_org_khronos_OpenVX_Graph(JNIEnv *env)
    {
        PrintJNITable(LOG_TAG, GraphClass, method_table, NELEM(method_table));
        return jniRegisterNativeMethods(env, GraphClass, method_table, NELEM(method_table));
    }
};



