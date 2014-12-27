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

#include <openvx.h>

//******************************************************************************
// PROTOTYPES
//******************************************************************************

static VALUE OpenVX_kernels(VALUE self);
static VALUE OpenVX_modules(VALUE self);
static VALUE OpenVX_targets(VALUE self);
static VALUE OpenVX_references(VALUE self);
static VALUE OpenVX_load(VALUE self, VALUE name);
static VALUE OpenVX_implementation();
static void Reference_mark(void *arg);
static void Reference_free(void *arg);
static VALUE Reference_allocate(VALUE klass);
static VALUE Reference_type(VALUE self);
static VALUE Reference_count(VALUE self);
static void Graph_mark(void *arg);
static void Graph_free(void *arg);
static VALUE Graph_init(VALUE self);
static VALUE Graph_allocate(VALUE klass);
static VALUE Graph_nodes(VALUE self);
static VALUE Graph_verify(VALUE self);
static VALUE Graph_process(VALUE self);
static void Kernel_mark(void *arg);
static void Kernel_free(void *arg);
static VALUE Kernel_init(VALUE self, VALUE param);
static VALUE Kernel_allocate(VALUE klass);
static VALUE Kernel_params(VALUE self);
static VALUE Kernel_enum(VALUE self);
static VALUE Kernel_name(VALUE self);
static void Scalar_mark(void *arg);
static void Scalar_free(void *arg);
static VALUE Scalar_init(VALUE self, VALUE v);
static VALUE Scalar_allocate(VALUE klass);
static void Buffer_mark(void *arg);
static void Buffer_free(void *arg);
static VALUE Buffer_init(int argc, VALUE *args, VALUE self);
static VALUE Buffer_allocate(VALUE klass);
static void Image_mark(void *arg);
static void Image_free(void *arg);
static VALUE Image_init(int argc, VALUE *args, VALUE self);
static VALUE Image_allocate(VALUE klass);
static void Node_mark(void *arg);
static void Node_free(void *arg);
static VALUE Node_init(int argc, VALUE *args, VALUE self);
static VALUE Node_allocate(VALUE klass);
static void Parameter_mark(void *arg);
static void Parameter_free(void *arg);
static VALUE Parameter_init(VALUE self, VALUE node, VALUE index);
static VALUE Parameter_dir(VALUE self);
static VALUE Parameter_value(VALUE self);
static VALUE Parameter_allocate(VALUE klass);
static VALUE Target_init(VALUE self, VALUE index);
static VALUE Target_kernels_list(VALUE self);
static VALUE Target_index(VALUE self);
static VALUE Target_name(VALUE self);
static VALUE Target_allocate(VALUE klass);

//******************************************************************************
// LOCAL VARIABLES
//******************************************************************************

static vx_context context;      /*! the overall OpenVX Context */
static VALUE rb_mOpenVXString;  /*! Ruby Object which holds the implementation string */
static char implementation[VX_MAX_IMPLEMENTATION_NAME];

static rext_method_t openvx_global_methods[] = {
    {"num_kernels",     0, &OpenVX_kernels},
    {"num_modules",     0, &OpenVX_modules},
    {"num_references",  0, &OpenVX_references},
    {"num_targets",     0, &OpenVX_targets},
    {"load",            1, &OpenVX_load},
    {"implementation",  0, &OpenVX_implementation},
};

static rext_method_t reference_methods[] = {
    {"type",            0, &Reference_type},
    {"count",           0, &Reference_count},
};

static rext_method_t graph_methods[] = {
    {"initialize",      0, &Graph_init},
    {"nodes",           0, &Graph_nodes},
    {"process",         0, &Graph_process},
    {"verify",          0, &Graph_verify},
//    {"schedule",        0, &Graph_schedule},
//    {"wait",            0, &Graph_wait},
};

static rext_method_t kernel_methods[] = {
    {"initialize",      1, &Kernel_init},
    {"params",          0, &Kernel_params},
    {"name",            0, &Kernel_name},
    {"enum",            0, &Kernel_enum},
};

static rext_method_t scalar_methods[] = {
    {"initialize",      2, &Scalar_init},
};

static rext_method_t buffer_methods[] = {
    {"initialize",      -1, &Buffer_init},
};

static rext_method_t image_methods[] = {
    {"initialize",      -1, &Image_init},
};

static rext_method_t node_methods[] = {
    {"initialize",      -1, &Node_init},
};

static rext_method_t parameter_methods[] = {
    {"initialize",      2,  &Parameter_init},
    {"direction",       0,  &Parameter_dir},
    {"value",           0,  &Parameter_value},
};

static rext_method_t target_methods[] = {
    {"initialize",      1,  &Target_init },
    {"kernel_names",    0,  &Target_kernels_list },
    {"index",           0,  &Target_index },
    {"name",            0,  &Target_name },
};

static rext_const_t image_constants[] = {
    {"VX_DF_IMAGE_RGB",      T_FIXNUM, .value.l=VX_DF_IMAGE_RGB},
    {"VX_DF_IMAGE_RGBX",     T_FIXNUM, .value.l=VX_DF_IMAGE_RGBX},
    {"VX_DF_IMAGE_NV12",     T_FIXNUM, .value.l=VX_DF_IMAGE_NV12},
    {"VX_DF_IMAGE_NV21",     T_FIXNUM, .value.l=VX_DF_IMAGE_NV21},
    {"VX_DF_IMAGE_UYVY",     T_FIXNUM, .value.l=VX_DF_IMAGE_UYVY},
    {"VX_DF_IMAGE_YUYV",     T_FIXNUM, .value.l=VX_DF_IMAGE_YUYV},
    {"VX_DF_IMAGE_IYUV",     T_FIXNUM, .value.l=VX_DF_IMAGE_IYUV},
    {"VX_DF_IMAGE_YUV4",     T_FIXNUM, .value.l=VX_DF_IMAGE_YUV4},
    {"VX_DF_IMAGE_U8",       T_FIXNUM, .value.l=VX_DF_IMAGE_U8},
    {"VX_DF_IMAGE_U16",      T_FIXNUM, .value.l=VX_DF_IMAGE_U16},
    {"VX_DF_IMAGE_U32",      T_FIXNUM, .value.l=VX_DF_IMAGE_U32},
    {"VX_DF_IMAGE_S16",      T_FIXNUM, .value.l=VX_DF_IMAGE_S16},
    {"VX_DF_IMAGE_S32",      T_FIXNUM, .value.l=VX_DF_IMAGE_S32},

};

static rext_const_t parameter_constants[] = {
    {"VX_INPUT",        T_FIXNUM, .value.l=VX_INPUT},
    {"VX_OUTPUT",       T_FIXNUM, .value.l=VX_OUTPUT},
    {"VX_BIDIRECTIONAL",T_FIXNUM, .value.l=VX_BIDIRECTIONAL},
};

static rext_object_t objects[] = {
    {0, &rb_cObject, "Reference", &Reference_allocate, NULL, 0, NULL, 0, reference_methods, dimof(reference_methods)},
    {0, &rb_cObject, "Kernel", &Kernel_allocate, NULL, 0, NULL, 0, kernel_methods, dimof(kernel_methods)},
    {0, &rb_cObject, "Graph", &Graph_allocate, NULL, 0, NULL, 0, graph_methods, dimof(graph_methods)},
    {0, &rb_cObject, "Image", &Image_allocate, NULL, 0, image_constants, dimof(image_constants), image_methods, dimof(image_methods)},
    {0, &rb_cObject, "Node", &Node_allocate, NULL, 0, NULL, 0, node_methods, dimof(node_methods)},
    {0, &rb_cObject, "Parameter", &Parameter_allocate, NULL, 0, parameter_constants, dimof(parameter_constants), parameter_methods, dimof(parameter_methods)},
    {0, &rb_cObject, "Buffer", &Buffer_allocate, NULL, 0, NULL, 0, buffer_methods, dimof(buffer_methods)},
    {0, &rb_cObject, "Scalar", &Scalar_allocate, NULL, 0, NULL, 0, scalar_methods, dimof(scalar_methods)},
    {0, &rb_cObject, "Target", &Target_allocate, NULL, 0, NULL, 0, target_methods, dimof(target_methods)},
    // Distribution
    // Convolution
    // Matrix
    // Array?
    //
};

static rext_module_t modules[] = {
    {0, "OpenVX", NULL, 0, objects, dimof(objects), NULL, 0, openvx_global_methods, dimof(openvx_global_methods)},
};

//******************************************************************************
// INITIALIZATION
//******************************************************************************

void Init_openvx()
{
    context = vxCreateContext();
    vx_status status = vxQueryContext(context, VX_CONTEXT_ATTRIBUTE_IMPLEMENTATION, implementation, sizeof(implementation));
    if (status != VX_SUCCESS)
    {
        REXT_PRINT("context = "VX_FMT_REF" status = %d\n", context, status);
        strncpy(implementation, "khronos.sample (bad)", sizeof(implementation));
    }
    rb_mOpenVXString = rb_str_new2(implementation);
    rubyext_modules(Qnil, modules, dimof(modules));
    // there is no "unload"
}

//******************************************************************************
// MODULE FUNCTIONS
//******************************************************************************

static VALUE OpenVX_kernels(VALUE self)
{
    vx_uint32 value = 0;
    vx_status status = VX_FAILURE;
    status = vxQueryContext(context, VX_CONTEXT_ATTRIBUTE_UNIQUE_KERNELS, &value, sizeof(value));
    REXT_PRINT("status = %d, numkernels = %d\n", status, value);
    if (status == VX_SUCCESS)
        return INT2FIX(value);
    else
        return INT2FIX(0);
}


static VALUE OpenVX_modules(VALUE self)
{
    vx_uint32 value = 0;
    vx_status status = VX_FAILURE;
    status = vxQueryContext(context, VX_CONTEXT_ATTRIBUTE_MODULES, &value, sizeof(value));
    REXT_PRINT("status = %d, modules = %d\n", status, value);
    if (status == VX_SUCCESS)
        return INT2FIX(value);
    else
        return INT2FIX(0);
}

static VALUE OpenVX_targets(VALUE self)
{
    vx_uint32 value = 0;
    vx_status status = VX_FAILURE;
    status = vxQueryContext(context, VX_CONTEXT_ATTRIBUTE_TARGETS, &value, sizeof(value));
    REXT_PRINT("status = %d, targets = %d\n", status, value);
    if (status == VX_SUCCESS)
        return INT2FIX(value);
    else
        return INT2FIX(0);
}

static VALUE OpenVX_targets_list(VALUE self)
{
    vx_uint32 t,value = 0;
    vx_status status = VX_FAILURE;
    vx_char targetname[VX_MAX_TARGET_NAME];
    vx_target target = 0;
    VALUE array;
    status = vxQueryContext(context, VX_CONTEXT_ATTRIBUTE_TARGETS, &value, sizeof(value));
    REXT_PRINT("status = %d, targets = %d\n", status, value);
    if (status == VX_SUCCESS)
    {
        array = rb_ary_new2(value);
        for (t = 0; t < value; t++)
        {
            target = vxGetTargetByIndex(context, t);
            if (target)
            {
                status = vxQueryTarget(target, VX_TARGET_ATTRIBUTE_NAME, targetname, sizeof(targetname));
                if (status == VX_SUCCESS)
                    rb_ary_push(array, rb_str_new2(targetname));
            }
        }
    }
    return array;
}


static VALUE OpenVX_references(VALUE self)
{
    vx_uint32 value = 0;
    vx_status status = VX_FAILURE;
    status = vxQueryContext(context, VX_CONTEXT_ATTRIBUTE_REFERENCES, &value, sizeof(value));
    REXT_PRINT("status = %d, references = %d\n", status, value);
    if (status == VX_SUCCESS)
        return INT2FIX(value);
    else
        return INT2FIX(0);
}


static VALUE OpenVX_load(VALUE self, VALUE name)
{
    vx_status status = VX_FAILURE;
    Check_Type(name, T_STRING);
    status = vxLoadKernels(context, RSTRING_PTR(name));
    REXT_PRINT("status = %d\n", status);
    return Qnil;
}

static VALUE OpenVX_implementation()
{
    return rb_mOpenVXString;
}

//******************************************************************************

static void Reference_mark(void *arg) {}

static void Reference_free(void *arg) {}

static VALUE Reference_allocate(VALUE klass)
{
    return rb_data_object_alloc(klass, (void *)NULL, Reference_mark, Reference_free);
}

static VALUE Reference_type(VALUE self)
{
    vx_enum t;
    vx_reference ref = (vx_reference)DATA_PTR(self);
    vxQueryReference(ref, VX_REF_ATTRIBUTE_TYPE, &t, sizeof(t));
    return INT2FIX(t);
}

static VALUE Reference_count(VALUE self)
{
    vx_uint32 t;
    vx_reference ref = (vx_reference)DATA_PTR(self);
    vxQueryReference(ref, VX_REF_ATTRIBUTE_COUNT, &t, sizeof(t));
    return INT2FIX(t);
}

//******************************************************************************


static void Graph_mark(void *arg) {}

static void Graph_free(void *arg)
{
    vxReleaseGraph((vx_graph *)&arg);
}

static VALUE Graph_init(VALUE self)
{
    Check_Type(self, T_DATA);
    DATA_PTR(self) = (void *)vxCreateGraph(context);
    return Qnil;
}
static VALUE Graph_allocate(VALUE klass)
{
    return rb_data_object_alloc(klass, NULL, Graph_mark, Graph_free);
}

static VALUE Graph_nodes(VALUE self)
{
    vx_graph graph = 0;
    vx_uint32 numNodes = 0;
    vx_status status;
    Check_Type(self, T_DATA);
    graph = (vx_graph)DATA_PTR(self);
    status = vxQueryGraph(graph, VX_GRAPH_ATTRIBUTE_NUMNODES, &numNodes, sizeof(numNodes));
    REXT_PRINT("status = %d, numNodes = %u\n", status, numNodes);
    return INT2FIX(numNodes);
}

static VALUE Graph_verify(VALUE self)
{
    vx_graph graph = 0;
    vx_status status = VX_FAILURE;
    Check_Type(self, T_DATA);
    graph = (vx_graph)DATA_PTR(self);
    status = vxVerifyGraph(graph);
    REXT_PRINT("status = %d\n", status);
    switch (status)
    {
        case VX_SUCCESS:
            break;
        default:
            rb_raise(rb_eException, "Verify failed.");
            break;
    }
    return Qnil;
}

static VALUE Graph_process(VALUE self)
{
    vx_graph graph = 0;
    vx_status status = VX_FAILURE;
    Check_Type(self, T_DATA);
    graph = (vx_graph)DATA_PTR(self);
    status = vxProcessGraph(graph);
    REXT_PRINT("status = %d\n", status);
    switch (status)
    {
        case VX_SUCCESS:
            break;
        default:
            rb_raise(rb_eException, "Process failed.");
            break;
    }
    return Qnil;
}

//******************************************************************************

static void Kernel_mark(void *arg) {}

static void Kernel_free(void *arg)
{
    vxReleaseKernel((vx_kernel *)&arg);
}

static VALUE Kernel_init(VALUE self, VALUE param)
{
    Check_Type(self, T_DATA);
    switch (TYPE(param))
    {
        case T_FIXNUM:
            DATA_PTR(self) = (void *)vxGetKernelByEnum(context, FIX2INT(param));
            break;
        case T_STRING:
            DATA_PTR(self) = (void *)vxGetKernelByName(context, RSTRING_PTR(param));
            break;
        default:
            REXT_PRINT("TYPE(param) = %d\n", TYPE(param));
            rb_raise(rb_eTypeError, "wrong type");
            break;
    }
    return Qnil;
}

static VALUE Kernel_allocate(VALUE klass)
{
    return rb_data_object_alloc(klass, NULL, Kernel_mark, Kernel_free);
}

static VALUE Kernel_params(VALUE self)
{
    vx_uint32 numParams = 0;
    vx_kernel kernel = 0;
    vx_status status = VX_FAILURE;
    Check_Type(self, T_DATA);
    kernel = (vx_kernel)DATA_PTR(self);
    status = vxQueryKernel(kernel, VX_KERNEL_ATTRIBUTE_PARAMETERS, &numParams, sizeof(numParams));
    REXT_PRINT("status = %d, numParams = %u\n", status, numParams);
    return INT2FIX(numParams);
}

static VALUE Kernel_name(VALUE self)
{
    vx_kernel kernel  = (vx_kernel)DATA_PTR(self);
    const char name[VX_MAX_KERNEL_NAME];
    vxQueryKernel(kernel, VX_KERNEL_ATTRIBUTE_NAME, (void *)name, sizeof(name));
    return rb_str_new2(name);
}

static VALUE Kernel_enum(VALUE self)
{
    vx_kernel kernel  = (vx_kernel)DATA_PTR(self);
    vx_enum kenum = VX_KERNEL_INVALID;
    vxQueryKernel(kernel, VX_KERNEL_ATTRIBUTE_ENUM, (void *)&kenum, sizeof(kenum));
    return INT2FIX(kenum);
}

//******************************************************************************

static void Scalar_mark(void *arg) {}

static void Scalar_free(void *arg) {}

static VALUE Scalar_init(VALUE self, VALUE v)
{
    vx_scalar scalar = 0;
    int type = rb_type(v);
    switch (type)
    {
        case T_FIXNUM:  // int32_t or int64_t based on ARCH depth
        {
#if defined(ARCH_16) // microcontrollers?
            vx_int16 value = FIX2INT(v);
            scalar = vxCreateScalar(context, VX_TYPE_INT16, &value);
#elif defined(ARCH_32) // ARM, mobile processors
            vx_int32 value = FIX2INT(v);
            scalar = vxCreateScalar(context, VX_TYPE_INT32, &value);
#elif defined(ARCH_64) // Desktops
            vx_int64 value = FIX2INT(v);
            scalar = vxCreateScalar(context, VX_TYPE_INT64, &value);
#endif
            break;
        }
        case T_BIGNUM:  // "infinite" length, don't know what to do with this...
            Check_Type(v, T_FIXNUM);
            break;
        case T_FLOAT: // doubles
        {
            vx_float64 value = NUM2DBL(v);
            scalar = vxCreateScalar(context, VX_TYPE_FLOAT64, &value);
            break;
        }
    }
    DATA_PTR(self) = (void *)scalar;
    return Qnil;
}

static VALUE Scalar_allocate(VALUE klass)
{
    return rb_data_object_alloc(klass, NULL, Reference_mark, Reference_free);
}


//******************************************************************************


static void Buffer_mark(void *arg) {}

static void Buffer_free(void *arg)
{
    vxReleaseBuffer((vx_buffer *)&arg);
}

static VALUE Buffer_init(int argc, VALUE *args, VALUE self)
{
    vx_size nu,su;
    vx_buffer buffer;

    Check_Type(self, T_DATA);
    Check_Type(args[0], T_FIXNUM);
    Check_Type(args[1], T_FIXNUM);

    if (argc == 2)
    {
        su = FIX2UINT(args[0]);
        nu = FIX2UINT(args[1]);
        buffer = vxCreateBuffer(context, su, nu);
        DATA_PTR(self) = (void *)buffer;
    }
    else
        rb_raise(rb_eArgError, "Incorrect number of arguments");
    return Qnil;
}

static VALUE Buffer_allocate(VALUE klass)
{
    return rb_data_object_alloc(klass, NULL, Buffer_mark, Buffer_free);
}

//******************************************************************************

static void Image_mark(void *arg) {}

static void Image_free(void *arg)
{
    vxReleaseImage((vx_image *)&arg);
}

static VALUE Image_init(int argc, VALUE *args, VALUE self)
{
    vx_uint32 width = 0, height = 0;
    vx_df_image format = VX_DF_IMAGE_VIRT;
    VALUE w,h,f;

    Check_Type(self, T_DATA);

    if (argc == 0) // Virtual Image
    {
        REXT_PRINT("Virtal Image\n");
        DATA_PTR(self) = (void *)vxCreateVirtualImage(context);
    }
    else if (argc == 1)
    {
        VALUE hash = args[0];
        Check_Type(hash, T_HASH);

        REXT_PRINT("Image from Hash\n");
        w = rb_hash_aref(hash, ID2SYM(rb_intern("width")));
        h = rb_hash_aref(hash, ID2SYM(rb_intern("height")));
        f = rb_hash_aref(hash, ID2SYM(rb_intern("format")));
        Check_Type(w, T_FIXNUM);
        Check_Type(h, T_FIXNUM);
        Check_Type(f, T_FIXNUM);
        width = FIX2UINT(w);
        height = FIX2UINT(h);
        format = FIX2UINT(f);
        DATA_PTR(self) = (void *)vxCreateImage(context, width, height, format);
    }
    else if (argc == 3)
    {
        REXT_PRINT("Image from Parameters\n");
        w = args[0];
        h = args[1];
        f = args[2];
        Check_Type(w, T_FIXNUM);
        Check_Type(h, T_FIXNUM);
        Check_Type(f, T_FIXNUM);
        width = FIX2UINT(w);
        height = FIX2UINT(h);
        format = FIX2UINT(f);
        DATA_PTR(self) = (void *)vxCreateImage(context, width, height, format);
    }
    else
        rb_raise(rb_eArgError, "incorrect number of arguments");
    return Qnil;
}

static VALUE Image_allocate(VALUE klass)
{
    return rb_data_object_alloc(klass, NULL, Image_mark, Image_free);
}

//******************************************************************************

static void Node_mark(void *arg) {}

static void Node_free(void *arg)
{
    vxReleaseNode((vx_node *)&arg);
}

static VALUE Node_init(int argc, VALUE *args, VALUE self)
{
    vx_graph graph = 0;
    vx_kernel kernel = 0;
    Check_Type(self, T_DATA);

    if (argc <= 1)
        rb_raise(rb_eArgError, "Not enough arguments");

    graph = (vx_graph)DATA_PTR(args[0]);

    if (argc == 2) // Kernel
    {
        Check_Type(args[1], T_DATA);
        kernel = (vx_kernel)DATA_PTR(args[1]);
        DATA_PTR(self) = (void *)vxCreateGenericNode(graph, kernel);
    }
    else if (argc == 3) // graph, [string|enum], array of hashes
    {
        vx_node node = 0;
        VALUE kern = args[1];
        VALUE array = args[2];
        long param = 0;

        if (TYPE(kern) == T_STRING)
            kernel = vxGetKernelByName(context, RSTRING_PTR(kern));
        else if (TYPE(kern) == T_FIXNUM)
            kernel = vxGetKernelByEnum(context, FIX2INT(kern));
        else if (TYPE(kern) == T_DATA) // a OpenVX::Kernel
            kernel = (vx_kernel)DATA_PTR(kern);
        else
            rb_raise(rb_eTypeError, "kernel must be a string, fixnum, or OpenVX::Kernel");

        if (kernel == 0)
            rb_raise(rb_eNameError, "kernel could not be found in OpenVX");

        Check_Type(array, T_ARRAY);

        node = vxCreateGenericNode(graph, kernel);
        if (node == 0)
            rb_raise(rb_eTypeError, "node could not be created!");

        REXT_PRINT("Array of parameters has len = %ld\n", RARRAY_LEN(array));
        for (param = 0; param < RARRAY_LEN(array) ; param++)
        {
            VALUE ref,hash;
            vx_reference ref2 = 0;
            vx_status status = 0;
            const char *name = NULL;

            hash = rb_ary_entry(array, param);
            Check_Type(hash, T_HASH);
            ref = rb_hash_aref(hash, ID2SYM(rb_intern("ref")));
            name = rb_obj_classname(ref);
            REXT_PRINT("ref class = %s\n", name);
            Check_Type(ref, T_DATA);
            ref2 = (vx_reference)DATA_PTR(ref);
            status = vxSetParameterByIndex(node, param, ref2);
            REXT_PRINT("status = %d\n", status);

        }
        DATA_PTR(self) = (void *)node;
    }
    else
    {
        rb_raise(rb_eArgError, "incorrect number of arguments");
    }
    return Qnil;
}

static VALUE Node_allocate(VALUE klass)
{
    return rb_data_object_alloc(klass, NULL, Node_mark, Node_free);
}

//******************************************************************************

static void Parameter_mark(void *arg) {}

static void Parameter_free(void *arg)
{
    vxReleaseParameter((vx_parameter *)&arg);
}

static VALUE Parameter_init(VALUE self, VALUE n, VALUE index)
{
    Check_Type(n, T_DATA);
    Check_Type(index, T_FIXNUM);
    vx_node node = (vx_node)DATA_PTR(n);
    vx_parameter param = vxGetParameterByIndex(node, FIX2UINT(index));
    DATA_PTR(self) = (void *)param;
    return Qnil;
}

static VALUE Parameter_dir(VALUE self)
{
    Check_Type(self, T_DATA);
    vx_enum dir;
    vx_parameter param = (vx_parameter)DATA_PTR(self);
    vx_status status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_DIRECTION, &dir, sizeof(dir));
    if (status == VX_SUCCESS)
        return INT2FIX(dir);
    else
        return INT2FIX(0);
}

static VALUE Parameter_value(VALUE self)
{
    Check_Type(self, T_DATA);
    vx_reference ref;
    vx_parameter param = (vx_parameter)DATA_PTR(self);
    vx_status status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &ref, sizeof(ref));
    if (status == VX_SUCCESS)
        return INT2FIX(ref);
    else
        return INT2FIX(0);
}

static VALUE Parameter_allocate(VALUE klass)
{
    return rb_data_object_alloc(klass, NULL, Parameter_mark, Parameter_free);
}

//******************************************************************************

static void Target_mark(void *arg) {}

static void Target_free(void *arg)
{
    vxReleaseTarget((vx_target *)&arg);
}

static VALUE Target_init(VALUE self, VALUE index)
{
    vx_target target = vxGetTargetByIndex(context, FIX2UINT(index));
    DATA_PTR(self) = (void *)target;
    return Qnil;
}

static VALUE Target_kernels_list(VALUE self)
{
    vx_target target = (vx_target)DATA_PTR(self);
    vx_uint32 k,numk = 0;
    vx_kernel_info_t *table = NULL;
    vxQueryTarget(target, VX_TARGET_ATTRIBUTE_NUMKERNELS, &numk, sizeof(numk));
    VALUE array = rb_ary_new2(numk);
    table = (vx_kernel_info_t *)calloc(numk, sizeof(*table));
    if (table)
    {
        vxQueryTarget(target, VX_TARGET_ATTRIBUTE_KERNELTABLE, table, numk*sizeof(*table));
        for (k = 0; k < numk; k++)
        {
            rb_ary_push(array, rb_str_new2(table[k].name));
        }
    }
    return array;
}

static VALUE Target_index(VALUE self)
{
    vx_target target = (vx_target)DATA_PTR(self);
    vx_uint32 index = 0;
    vxQueryTarget(target, VX_TARGET_ATTRIBUTE_INDEX, &index, sizeof(index));
    return INT2FIX(index);
}

static VALUE Target_name(VALUE self)
{
    vx_target target = (vx_target)DATA_PTR(self);
    const char name[VX_MAX_TARGET_NAME];
    vxQueryTarget(target, VX_TARGET_ATTRIBUTE_NAME, (void *)name, sizeof(name));
    return rb_str_new2(name);
}

static VALUE Target_allocate(VALUE klass)
{
    return rb_data_object_alloc(klass, NULL, Target_mark, Target_free);
}



