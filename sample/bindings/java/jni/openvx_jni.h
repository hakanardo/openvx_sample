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

#include <VX/vx.h>
#include <JNIHelp.h>
#include <jni.h>
#include <utils/Log.h>
#include <utils/misc.h>
#include <cutils/properties.h>
#include <android_runtime/AndroidRuntime.h>

#define VXPATH     "org/khronos/openvx/"
#define OBJECT(c)  "L"VXPATH #c";"

extern const char *ContextClass;
extern const char *ScalarClass;
extern const char *BufferClass;
extern const char *ImageClass;
extern const char *KernelClass;
extern const char *GraphClass;
extern const char *NodeClass;
extern const char *ParameterClass;
extern const char *ReferenceClass;
extern const char *TargetClass;
extern const char *handleName;
extern const char *parentName;

/*! \brief A helper function across all the JNI objects */
jlong GetHandle(JNIEnv *env, jobject obj, const char *className, const char *fieldName);

/*! \brief A helper function across all the JNI objects */
void SetHandle(JNIEnv *env, jobject obj, const char *className, const char *fieldName, jlong value);

/*! \brief Debugging method to show published JNI method table. */
void PrintJNITable(const char *tag, const char *className, JNINativeMethod *table, uint32_t numElem);

namespace android {
    int register_org_khronos_OpenVX_Context(JNIEnv *env);
    int register_org_khronos_OpenVX_Scalar(JNIEnv *env);
    int register_org_khronos_OpenVX_Buffer(JNIEnv *env);
    int register_org_khronos_OpenVX_Image(JNIEnv *env);
    int register_org_khronos_OpenVX_Kernel(JNIEnv *env);
    int register_org_khronos_OpenVX_Graph(JNIEnv *env);
    int register_org_khronos_OpenVX_Node(JNIEnv *env);
    int register_org_khronos_OpenVX_Parameter(JNIEnv *env);
    int register_org_khronos_OpenVX_Reference(JNIEnv *env);
    int register_org_khronos_OpenVX_Target(JNIEnv *env);
};
