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

#define LOG_TAG "Context_JNI"
#include "openvx_jni.h"

namespace android
{
    //**************************************************************************
    // LOCAL VARIABLES
    //**************************************************************************

    //**************************************************************************
    // EXPORTED FUNCTIONS
    //**************************************************************************
    static void Initialize(JNIEnv *env, jobject obj)
    {
        vx_context context = vxCreateContext();
        SetHandle(env, obj, ContextClass, handleName, (jlong)context);
    }

    static void Finalize(JNIEnv *env, jobject obj)
    {
        vx_context context = (vx_context)GetHandle(env, obj, ContextClass, handleName);
        vxReleaseContext(&context);
    }

    static jboolean loadKernels(JNIEnv *env, jobject obj,
                                jstring nameStr)
    {
        const char *name = env->GetStringUTFChars(nameStr, NULL);
        vx_context context = (vx_context)GetHandle(env, obj, ContextClass, handleName);
        vx_status status = vxLoadKernels(context, (vx_char *)name);
        env->ReleaseStringUTFChars(nameStr, name);
        if (status == VX_SUCCESS)
            return true;
        else
            return false;
    }

    static jint getNumKernels(JNIEnv *env, jobject obj)
    {
        vx_context context = (vx_context)GetHandle(env, obj, ContextClass, handleName);
        jint value = 0;
        vx_status status = vxQueryContext(context, VX_CONTEXT_ATTRIBUTE_UNIQUE_KERNELS, &value, sizeof(value));
        if (status != VX_SUCCESS)
            value = 0;
        return value;
    }

    static jint getNumModules(JNIEnv *env, jobject obj)
    {
        vx_context context = (vx_context)GetHandle(env, obj, ContextClass, handleName);
        jint value = 0;
        vx_status status = vxQueryContext(context, VX_CONTEXT_ATTRIBUTE_MODULES, &value, sizeof(value));
        if (status != VX_SUCCESS)
            value = 0;
        return value;
    }

    static jint getNumReferences(JNIEnv *env, jobject obj)
    {
        vx_context context = (vx_context)GetHandle(env, obj, ContextClass, handleName);
        jint value = 0;
        vx_status status = vxQueryContext(context, VX_CONTEXT_ATTRIBUTE_REFERENCES, &value, sizeof(value));
        if (status != VX_SUCCESS)
            value = 0;
        return value;
    }

    static jint getNumTargets(JNIEnv *env, jobject obj)
    {
        vx_context context = (vx_context)GetHandle(env, obj, ContextClass, handleName);
        jint value = 0;
        vx_status status = vxQueryContext(context, VX_CONTEXT_ATTRIBUTE_TARGETS, &value, sizeof(value));
        if (status != VX_SUCCESS)
            value = 0;
        return value;
    }

    static jobject createImage(JNIEnv *env, jobject obj, jint w, jint h, jint f)
    {
        vx_context context = (vx_context)GetHandle(env, obj, ContextClass, handleName);
        jclass c = env->FindClass(ImageClass);
        jmethodID m = env->GetMethodID(c, "<init>", "(JIII)V");
        jobject img = env->NewObject(c, m, (jlong)context, w, h, f);
        return img;
    }

    static jobject createBuffer(JNIEnv *env, jobject obj, jint nu, jint us)
    {
        vx_context context = (vx_context)GetHandle(env, obj, ContextClass, handleName);
        jclass c = env->FindClass(BufferClass);
        jmethodID m = env->GetMethodID(c, "<init>", "(JII)V");
        jobject buf = env->NewObject(c, m, (jlong)context, nu, us);
        return buf;
    }

    static jobject createByteScalar(JNIEnv *env, jobject obj, jbyte v)
    {
        vx_context context = (vx_context)GetHandle(env, obj, ContextClass, handleName);
        jclass c = env->FindClass(ScalarClass);
        jmethodID m = env->GetMethodID(c, "<init>", "(JB)V");
        jobject scalar = env->NewObject(c, m, (jlong)context, v);
        return scalar;
    }

    static jobject createShortScalar(JNIEnv *env, jobject obj, jshort v)
    {
        vx_context context = (vx_context)GetHandle(env, obj, ContextClass, handleName);
        jclass c = env->FindClass(ScalarClass);
        jmethodID m = env->GetMethodID(c, "<init>", "(JS)V");
        jobject scalar = env->NewObject(c, m, (jlong)context, v);
        return scalar;
    }

    static jobject createIntScalar(JNIEnv *env, jobject obj, jint v)
    {
        vx_context context = (vx_context)GetHandle(env, obj, ContextClass, handleName);
        jclass c = env->FindClass(ScalarClass);
        jmethodID m = env->GetMethodID(c, "<init>", "(JI)V");
        jobject scalar = env->NewObject(c, m, (jlong)context, v);
        return scalar;
    }

    static jobject createLongScalar(JNIEnv *env, jobject obj, jlong v)
    {
        vx_context context = (vx_context)GetHandle(env, obj, ContextClass, handleName);
        jclass c = env->FindClass(ScalarClass);
        jmethodID m = env->GetMethodID(c, "<init>", "(JJ)V");
        jobject scalar = env->NewObject(c, m, (jlong)context, v);
        return scalar;
    }

    static jobject createFloatScalar(JNIEnv *env, jobject obj, jfloat v)
    {
        vx_context context = (vx_context)GetHandle(env, obj, ContextClass, handleName);
        jclass c = env->FindClass(ScalarClass);
        jmethodID m = env->GetMethodID(c, "<init>", "(JF)V");
        jobject scalar = env->NewObject(c, m, (jlong)context, v);
        return scalar;
    }

    static jobject createDoubleScalar(JNIEnv *env, jobject obj, jdouble v)
    {
        vx_context context = (vx_context)GetHandle(env, obj, ContextClass, handleName);
        jclass c = env->FindClass(ScalarClass);
        jmethodID m = env->GetMethodID(c, "<init>", "(JD)V");
        jobject scalar = env->NewObject(c, m, (jlong)context, v);
        return scalar;
    }

    static jobject getKernelByName(JNIEnv *env, jobject obj, jstring kname)
    {
        vx_context context = (vx_context)GetHandle(env, obj, ContextClass, handleName);
        jclass c = env->FindClass(KernelClass);
        jmethodID m = env->GetMethodID(c, "<init>", "(JLjava/lang/String;)V");
        jobject k = env->NewObject(c, m, (jlong)context, kname);
        return k;
    }

    static jobject getKernelByEnum(JNIEnv *env, jobject obj, jint kenum)
    {
        vx_context context = (vx_context)GetHandle(env, obj, ContextClass, handleName);
        jclass c = env->FindClass(KernelClass);
        jmethodID m = env->GetMethodID(c, "<init>", "(JI)V");
        jobject k = env->NewObject(c, m, (jlong)context, kenum);
        return k;
    }

    static jobject createGraph(JNIEnv *env, jobject obj)
    {
        vx_context context = (vx_context)GetHandle(env, obj, ContextClass, handleName);
        jclass c = env->FindClass(GraphClass);
        jmethodID m = env->GetMethodID(c, "<init>", "(J)V");
        jobject g = env->NewObject(c, m, (jlong)context);
        return g;
    }

    static jobject getTarget(JNIEnv *env, jobject obj, jint index)
    {
        vx_context context = (vx_context)GetHandle(env, obj, ContextClass, handleName);
        jclass c = env->FindClass(TargetClass);
        jmethodID m = env->GetMethodID(c, "<init>", "(JI)V");
        jobject t = env->NewObject(c, m, (jlong)context, index);
        return t;
    }

    static JNINativeMethod method_table[] = {
        // { name,                         signature,                 function_pointer }
        { "create",                        "()V",                     (void *)Initialize },
        { "destroy",                       "()V",                     (void *)Finalize },

        { "loadKernels",                   "(Ljava/lang/String;)Z",   (void *)loadKernels },
        { "getNumKernels",                 "()I",                     (void *)getNumKernels },
        { "getNumModules",                 "()I",                     (void *)getNumModules },
        { "getNumReferences",              "()I",                     (void *)getNumReferences },
        { "getNumTargets",                 "()I",                     (void *)getNumTargets },

        { "createImage",                   "(III)"OBJECT(Image),      (void *)createImage },
        { "createBuffer",                  "(II)"OBJECT(Buffer),      (void *)createBuffer },

        { "createScalar",                  "(B)"OBJECT(Scalar),       (void *)createByteScalar },
        { "createScalar",                  "(S)"OBJECT(Scalar),       (void *)createShortScalar },
        { "createScalar",                  "(I)"OBJECT(Scalar),       (void *)createIntScalar },
        { "createScalar",                  "(J)"OBJECT(Scalar),       (void *)createLongScalar },
        { "createScalar",                  "(F)"OBJECT(Scalar),       (void *)createFloatScalar },
        { "createScalar",                  "(D)"OBJECT(Scalar),       (void *)createDoubleScalar },

        { "getKernel",                     "(Ljava/lang/String;)"OBJECT(Kernel), (void *)getKernelByName },
        { "getKernel",                     "(I)"OBJECT(Kernel),       (void *)getKernelByEnum },

        { "createGraph",                   "()"OBJECT(Graph),         (void *)createGraph },

        { "getTarget",                     "(I)"OBJECT(Target),       (void *)getTarget },
    };

    int register_org_khronos_OpenVX_Context(JNIEnv *env)
    {
        PrintJNITable(LOG_TAG, ContextClass, method_table, NELEM(method_table));
        return jniRegisterNativeMethods(env, ContextClass, method_table, NELEM(method_table));
    }
};



