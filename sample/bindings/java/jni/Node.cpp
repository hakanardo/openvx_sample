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

#define LOG_TAG "Node_JNI"
#include "openvx_jni.h"

namespace android
{
    //**************************************************************************
    // LOCAL VARIABLES
    //**************************************************************************

    //**************************************************************************
    // EXPORTED FUNCTIONS
    //**************************************************************************
    static void Initialize(JNIEnv *env, jobject obj, jlong g, jlong k)
    {
        vx_graph graph = (vx_graph)g;
        vx_kernel kernel = (vx_kernel)k;
        vx_node n = vxCreateGenericNode(graph, kernel);
        SetHandle(env, obj, NodeClass, parentName, g);
        SetHandle(env, obj, NodeClass, handleName, (jlong)n);
    }

    static void Finalize(JNIEnv *env, jobject obj)
    {
        vx_node n = (vx_node)GetHandle(env, obj, NodeClass, handleName);
        vxReleaseNode(&n);
        SetHandle(env, obj, NodeClass, handleName, 0);
    }

    static jobject getParameter(JNIEnv *env, jobject obj, jint i)
    {
        vx_node n = (vx_node)GetHandle(env, obj, NodeClass, handleName);
        jclass c = env->FindClass(ParameterClass);
        jmethodID id = env->GetMethodID(c, "<init>", "(JI)"OBJECT(Parameter));
        jobject p = env->NewObject(c, id, (jlong)n, i);
        return p;
    }

    static jint setParameter(JNIEnv *env, jobject obj, jint index, jlong ref)
    {
        vx_node n = (vx_node)GetHandle(env, obj, NodeClass, handleName);
        return vxSetParameterByIndex(n, index, (vx_reference)ref);
    }

    static JNINativeMethod method_table[] = {
        // { name,                         signature,                 function_pointer }
        { "create",                        "(JJ)V",                   (void *)Initialize },
        { "destroy",                       "()V",                     (void *)Finalize },
        { "getParameter",                  "(I)"OBJECT(Parameter),    (void *)getParameter },
        { "_setParameter",                 "(IJ)I",                   (void *)setParameter },
    };

    int register_org_khronos_OpenVX_Node(JNIEnv *env)
    {
        PrintJNITable(LOG_TAG, NodeClass, method_table, NELEM(method_table));
        return jniRegisterNativeMethods(env, NodeClass, method_table, NELEM(method_table));
    }
};



