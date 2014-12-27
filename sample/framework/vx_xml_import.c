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

#include <vx_internal.h>

#if defined(EXPERIMENTAL_USE_XML)

#include <vx_type_pairs.h>

#if defined(__APPLE__)
typedef unsigned long ulong;
#endif

#include <libxml/tree.h>

#define XML_FOREACH_CHILD_TAG(child, tag, tags) \
        for (child = (xmlFirstElementChild(child) ? \
                      (tag=xml_match_tag(xmlFirstElementChild(child),tags,dimof(tags)),xmlFirstElementChild(child)) : \
                      (tag=0,child)); \
             tag && child; \
             child = (xmlNextElementSibling(child) ? \
                      (tag=xml_match_tag(xmlNextElementSibling(child),tags,dimof(tags)),xmlNextElementSibling(child)) : \
                      (tag=0,child->parent)))

#define XML_HAS_CHILD(child) xmlFirstElementChild(child)

#define XML_MAX_TAG_NAME (20)

#define REFNUM_ERROR VX_PRINT(VX_ZONE_ERROR, "Reference number out of bounds: \"references\" tag=%d, \"reference\"=%d\n", total, refIdx); status = VX_ERROR_INVALID_FORMAT;

typedef struct _xml_tag_t {
    xmlChar name[XML_MAX_TAG_NAME];
    int32_t tag;
} xml_tag_t;

typedef struct _xml_struct_t {
    vx_uint32 label;
    vx_enum e_struct_type;
} xml_struct_t;

static void xml_string(xmlNodePtr cur, char out[], size_t nchar)
{
    xmlChar *string = xmlNodeListGetString(cur->doc, cur->children, 1);
    strncpy(out, (char *)string, nchar);
    free(string);
}

static ulong xml_ulong(xmlNodePtr cur)
{
    xmlChar *string = xmlNodeListGetString(cur->doc, cur->children, 1);
    ulong value = 0ul;
    if (string)
        value = strtoul((const char *)string, NULL, 10);
    free(string);
    return value;
}

static ulong xml_long(xmlNodePtr cur)
{
    xmlChar *string = xmlNodeListGetString(cur->doc, cur->children, 1);
    long value = 0l;
    if (string)
        value = strtol((const char *)string, NULL, 10);
    free(string);
    return value;
}

static float xml_float(xmlNodePtr cur)
{
    xmlChar *string = xmlNodeListGetString(cur->doc, cur->children, 1);
    float value = 0.0f;
    if (string)
        sscanf((const char *)string, "%f", &value);
    free(string);
    return value;
}

static double xml_double(xmlNodePtr cur)
{
    xmlChar *string = xmlNodeListGetString(cur->doc, cur->children, 1);
    double value = 0.0f;
    if (string)
        sscanf((const char *)string, "%lf", &value);
    free(string);
    return value;
}

static ulong xml_prop_ulong(xmlNodePtr cur, char prop[])
{
    xmlChar *string = xmlGetProp(cur, (const xmlChar *)prop);
    ulong value = 0ul;
    if (string)
        value = strtoul((const char *)string, NULL, 10);
    free(string);
    return value;
}

static float xml_prop_float(xmlNodePtr cur, char prop[])
{
    xmlChar *string = xmlGetProp(cur, (const xmlChar *)prop);
    float value = 0.0f;
    if (string)
        sscanf((const char *)string, "%f", &value);
    free(string);
    return value;
}
/*
static double xml_prop_double(xmlNodePtr cur, char prop[])
{
    xmlChar *string = xmlGetProp(cur, (const xmlChar *)prop);
    double value = 0.0f;
    if (string)
        sscanf((const char *)string, "%lf", &value);
    free(string);
    return value;
}
*/
static void xml_prop_string(xmlNodePtr cur, char prop[], char out[], size_t nchar)
{
    xmlChar *string = xmlGetProp(cur, (const xmlChar *)prop);
    if (string) {
        strncpy(out, (char *)string, nchar);
        free(string);
    }
}

static int32_t xml_match_tag(xmlNodePtr cur, xml_tag_t tags[], size_t num_tags)
{
    int32_t tag = 0;
    if (cur)
    {
        uint32_t i = 0u;
        for (i = 0u; i < num_tags; i++)
        {
            if (!xmlStrcasecmp(cur->name, tags[i].name))
            {
                tag = tags[i].tag;
                break;
            }
        }
    }
    return tag;
}

/* Used by delay object import to reserve space in the ref table for the
 * delay object before creating the exemplar object, so that when the
 * exemplar object is released, there is no hole in the ref table, causing
 * a different order of reftable than what was in the xml file
 * (this is for convenience, not functionality) */
static vx_status vxReserveReferences(vx_context context, vx_uint32 num)
{
    vx_status status = VX_FAILURE;
    vx_uint32 r, count = 0;
    for (r = 0u; (r < VX_INT_MAX_REF) && (count < num); r++)
    {
        if (context->reftable[r] == NULL) {
            /* 1 is used as a flag that this is reserved since it is not a valid handle */
            context->reftable[r] = (vx_reference)1;
            count++;
        }
    }
    if (count == num)
        status = VX_SUCCESS;

    return status;
}

/* Used by delay object import to release the previously reserved spaces in
 * the ref table so that the delay object can then be created in these spaces */
static vx_status vxReleaseReferences(vx_context context, vx_uint32 num)
{
    vx_status status = VX_FAILURE;
    vx_uint32 r, count = 0;
    for (r = 0u; (r < VX_INT_MAX_REF) && (count < num); r++)
    {
        if (context->reftable[r] == (vx_reference)1) {
            /* 1 is used as a flag that this is reserved since it is not a valid handle */
            context->reftable[r] = NULL;
            count++;
        }
    }
    if (count == num)
        status = VX_SUCCESS;

    return status;
}

/* Convert external reference to an internal reference */
static void vxInternalizeReference(vx_reference ref)
{
    vxIncrementReference(ref, VX_INTERNAL);
    vxDecrementReference(ref, VX_EXTERNAL);
}

/* Adds the numeric label for a user struct enumeration to a local table so that it can be correlated to usage in
 * arrays that reference the label from the XML */
static vx_status vxStructSetEnum(xml_struct_t ptable[VX_INT_MAX_USER_STRUCTS], vx_char *slabel, vx_enum e_struct_type)
{
    vx_status status = VX_FAILURE;
    vx_uint32 r;
    vx_uint32 label;

    if(sscanf(slabel, "USER_STRUCT_%u", &label) != 1)
        return status;

    for (r = 0u; r < VX_INT_MAX_USER_STRUCTS; r++)
    {
        if (ptable[r].e_struct_type == (vx_enum)0) {
            ptable[r].e_struct_type = e_struct_type;
            ptable[r].label = label;
            status = VX_SUCCESS;
            break;
        }
    }

    return status;
}

/* Gets the enumeration associated with a numeric label for a user struct from a local table */
static vx_status vxStructGetEnum(xml_struct_t ptable[VX_INT_MAX_USER_STRUCTS], vx_uint32 label, vx_enum *e_struct_type)
{
    vx_status status = VX_FAILURE;
    vx_uint32 r;

    for (r = 0u; r < VX_INT_MAX_USER_STRUCTS; r++)
    {
        if (ptable[r].e_struct_type == (vx_enum)0) {
            break;
        }
        else if (ptable[r].label == label) {
            *e_struct_type = ptable[r].e_struct_type;
            status = VX_SUCCESS;
            break;
        }
    }

    return status;
}


typedef enum _vx_xml_tag_e {
    UNKNOWN_TAG = 0,
    OPENVX_TAG,
    LIBRARY_TAG,
    STRUCT_TAG,
    GRAPH_TAG,
    NODE_TAG,
    KERNEL_TAG,
    PARAMETER_TAG,

    BOOL_TAG,
    CHAR_TAG,
    UINT8_TAG,
    UINT16_TAG,
    UINT32_TAG,
    UINT64_TAG,
    INT8_TAG,
    INT16_TAG,
    INT32_TAG,
    INT64_TAG,
    ENUM_TAG,
    DF_IMAGE_TAG,
    SIZE_TAG,
    FLOAT32_TAG,
    FLOAT64_TAG,

    KEYPOINT_TAG,
    COORDINATES2D_TAG,
    COORDINATES3D_TAG,
    RECTANGLE_TAG,
    USER_TAG,

    ARRAY_TAG,
    CONVOLUTION_TAG,
    DELAY_TAG,
    DISTRIBUTION_TAG,
    IMAGE_TAG,
    LUT_TAG,
    MATRIX_TAG,
    PYRAMID_TAG,
    REMAP_TAG,
    SCALAR_TAG,
    THRESHOLD_TAG,

    /* fields */
    START_X_TAG,
    START_Y_TAG,
    END_X_TAG,
    END_Y_TAG,
    X_TAG,
    Y_TAG,
    Z_TAG,
    STRENGTH_TAG,
    SCALE_TAG,
    ORIENTATION_TAG,
    TRACKING_STATUS_TAG,
    ERROR_TAG,
    FREQUENCY_TAG,
    POINT_TAG,
    BINARY_TAG,
    RANGE_TAG,
    PIXELS_TAG,

    /* PIXEL FORMAT */
    RGB_TAG,
    RGBA_TAG,
    YUV_TAG,
} vx_xml_tag_e;

static xml_tag_t tags[] = {
    {"OPENVX", OPENVX_TAG},
    {"LIBRARY",LIBRARY_TAG},
    {"STRUCT", STRUCT_TAG},
    {"GRAPH",  GRAPH_TAG},
    {"NODE",   NODE_TAG},
    {"PARAMETER", PARAMETER_TAG},
    {"KERNEL", KERNEL_TAG},

    {"CHAR",   CHAR_TAG},
    {"UINT8",  UINT8_TAG},
    {"UINT16", UINT16_TAG},
    {"UINT32", UINT32_TAG},
    {"UINT64", UINT64_TAG},
    {"INT8",   INT8_TAG},
    {"INT16",  INT16_TAG},
    {"INT32",  INT32_TAG},
    {"INT64",  INT64_TAG},
    {"FLOAT32",FLOAT32_TAG},
    {"FLOAT64",FLOAT64_TAG},
    {"BOOL",   BOOL_TAG},
    {"ENUM",   ENUM_TAG},
    {"SIZE",   SIZE_TAG},
    {"DF_IMAGE", DF_IMAGE_TAG},

    {"KEYPOINT", KEYPOINT_TAG},
    {"RECTANGLE", RECTANGLE_TAG},
    {"COORDINATES2D", COORDINATES2D_TAG},
    {"COORDINATES3D", COORDINATES3D_TAG},
    {"USER", USER_TAG},

    {"SCALAR", SCALAR_TAG},
    {"CONVOLUTION", CONVOLUTION_TAG},
    {"DELAY",  DELAY_TAG},
    {"LUT",    LUT_TAG},
    {"DISTRIBUTION", DISTRIBUTION_TAG},
    {"IMAGE",  IMAGE_TAG},
    {"ARRAY",  ARRAY_TAG},
    {"MATRIX", MATRIX_TAG},
    {"PYRAMID",PYRAMID_TAG},
    {"REMAP",  REMAP_TAG},
    {"THRESHOLD", THRESHOLD_TAG},

    {"START_X", START_X_TAG},
    {"START_Y", START_Y_TAG},
    {"END_X",   END_X_TAG},
    {"END_Y",   END_Y_TAG},
    {"X",       X_TAG},
    {"Y",       Y_TAG},
    {"Z",       Z_TAG},
    {"STRENGTH",STRENGTH_TAG},
    {"SCALE",   SCALE_TAG},
    {"ORIENTATION", ORIENTATION_TAG},
    {"TRACKING_STATUS", TRACKING_STATUS_TAG},
    {"ERROR",   ERROR_TAG},
    {"FREQUENCY",FREQUENCY_TAG},
    {"POINT",   POINT_TAG},
    {"BINARY",  BINARY_TAG},
    {"RANGE",   RANGE_TAG},
    {"PIXELS",  PIXELS_TAG},

    {"RGB",     RGB_TAG},
    {"RGBA",    RGBA_TAG},
    {"YUV",     YUV_TAG},
};

static vx_char refNameStr[VX_MAX_REFERENCE_NAME];

static void vxSetName(vx_reference ref, xmlNodePtr cur)
{
    refNameStr[0] = 0;
    xml_prop_string(cur, "name", refNameStr, sizeof(refNameStr));

    if(refNameStr[0]) {
        vxSetReferenceName(ref, refNameStr);
    }
}

static xml_struct_t *user_struct_table = NULL;

static vx_status vxLoadDataForImage(vx_image image, xmlNodePtr cur)
{
    vx_status status = VX_SUCCESS;
    vx_xml_tag_e tag = UNKNOWN_TAG;
    XML_FOREACH_CHILD_TAG (cur, tag, tags) {
        if (tag == RECTANGLE_TAG) {
            vx_rectangle_t rect = {0};
            vx_uint32 pIdx = xml_prop_ulong(cur, "plane");
            XML_FOREACH_CHILD_TAG (cur, tag, tags) {
                if (tag == START_X_TAG) {
                    rect.start_x = xml_ulong(cur);
                } else if (tag == START_Y_TAG) {
                    rect.start_y = xml_ulong(cur);
                } else if (tag == END_X_TAG) {
                    rect.end_x = xml_ulong(cur);
                } else if (tag == END_Y_TAG) {
                    rect.end_y = xml_ulong(cur);
                } else if (tag == PIXELS_TAG) {
                    void *base = NULL;
                    vx_imagepatch_addressing_t addr = {0};
                    status |= vxAccessImagePatch(image, &rect, pIdx, &addr, &base, VX_WRITE_ONLY);
                    if (status == VX_SUCCESS) {
                        XML_FOREACH_CHILD_TAG(cur, tag, tags) {
                            vx_uint32 x = xml_prop_ulong(cur, "x");
                            vx_uint32 y = xml_prop_ulong(cur, "y");
                            if (tag == UINT8_TAG) {
                                vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                ptr[0] = (vx_uint8)xml_ulong(cur);
                            } else if (tag == UINT16_TAG) {
                                vx_uint16 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                ptr[0] = (vx_uint16)xml_ulong(cur);
                            } else if (tag == UINT32_TAG) {
                                vx_uint32 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                ptr[0] = (vx_uint32)xml_ulong(cur);
                            } else if (tag == INT16_TAG) {
                                vx_int16 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                ptr[0] = (vx_int16)xml_long(cur);
                            } else if (tag == INT32_TAG) {
                                vx_int32 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                ptr[0] = (vx_int32)xml_long(cur);
                            } else if (tag == RGB_TAG) {
                                vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                vx_uint32 tmp[3];
                                vx_char values[13] = {0};
                                xml_string(cur, values, sizeof(values));
                                if (values[0] == '#') {
                                    sscanf(values, "#%02x%02x%02x", &tmp[0], &tmp[1], &tmp[2]);
                                    ptr[0] = (vx_uint8)tmp[0];
                                    ptr[1] = (vx_uint8)tmp[1];
                                    ptr[2] = (vx_uint8)tmp[2];
                                } else {
                                    vx_char *tmp = NULL;
                                    tmp = strtok(values, " \t\n\r");
                                    sscanf(tmp, "%hhu", &ptr[0]);
                                    tmp = strtok(NULL, " \t\n\r");
                                    sscanf(tmp, "%hhu", &ptr[1]);
                                    tmp = strtok(NULL, " \t\n\r");
                                    sscanf(tmp, "%hhu", &ptr[2]);
                                }
                            } else if (tag == RGBA_TAG) {
                                vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                vx_uint32 tmp[4];
                                vx_char values[17] = {0};
                                xml_string(cur, values, sizeof(values));
                                if (values[0] == '#') {
                                    sscanf(values, "#%02x%02x%02x%02x", &tmp[0], &tmp[1], &tmp[2], &tmp[3]);
                                    ptr[0] = (vx_uint8)tmp[0];
                                    ptr[1] = (vx_uint8)tmp[1];
                                    ptr[2] = (vx_uint8)tmp[2];
                                    ptr[3] = (vx_uint8)tmp[3];
                                } else {
                                    vx_char *tmp = NULL;
                                    tmp = strtok(values, " \t\n\r");
                                    sscanf(tmp, "%hhu", &ptr[0]);
                                    tmp = strtok(NULL, " \t\n\r");
                                    sscanf(tmp, "%hhu", &ptr[1]);
                                    tmp = strtok(NULL, " \t\n\r");
                                    sscanf(tmp, "%hhu", &ptr[2]);
                                    tmp = strtok(NULL, " \t\n\r");
                                    sscanf(tmp, "%hhu", &ptr[3]);
                                }
                            } else if (tag == YUV_TAG) {
                                vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                                vx_char values[8], *tmp = NULL;
                                xml_string(cur, values, sizeof(values));
                                tmp = strtok(values, " \t\n\r");
                                sscanf(tmp, "%hhu", &ptr[0]);
                                tmp = strtok(NULL, " \t\n\r");
                                sscanf(tmp, "%hhu", &ptr[1]);
                            }
                        }
                        status |= vxCommitImagePatch(image, &rect, pIdx, &addr, base);
                    }
                }
            }
        }
    }
    return status;
}

static vx_status vxLoadDataForArray(vx_array array, xmlNodePtr cur)
{
    vx_status status = VX_SUCCESS;
    vx_xml_tag_e tag = UNKNOWN_TAG;
    XML_FOREACH_CHILD_TAG (cur, tag, tags) {
        if (tag == CHAR_TAG) {
            char *str = (char *)xmlNodeListGetString(cur->doc, cur->children, 1);
            status |= vxAddArrayItems(array, strlen(str), str, 0);
            free(str);
        } else if (tag == KEYPOINT_TAG) {
            vx_keypoint_t kp = {0};
            XML_FOREACH_CHILD_TAG(cur, tag, tags) {
                if (tag == X_TAG) {
                    kp.x = (vx_int32)xml_long(cur);
                } else if (tag == Y_TAG) {
                    kp.y = (vx_int32)xml_long(cur);
                } else if (tag == STRENGTH_TAG) {
                    kp.strength = xml_float(cur);
                } else if (tag == SCALE_TAG) {
                    kp.scale = xml_float(cur);
                } else if (tag == ORIENTATION_TAG) {
                    kp.orientation = xml_float(cur);
                } else if (tag == TRACKING_STATUS_TAG) {
                    kp.tracking_status = (vx_int32)xml_long(cur);
                } else if (tag == ERROR_TAG) {
                    kp.error = xml_float(cur);
                }
            }
            status |= vxAddArrayItems(array, 1, &kp, 0);
        } else if (tag == COORDINATES2D_TAG) {
            vx_coordinates2d_t coord = {0};
            XML_FOREACH_CHILD_TAG(cur, tag, tags) {
                if (tag == X_TAG) {
                    coord.x = (vx_uint32)xml_ulong(cur);
                } else if (tag == Y_TAG) {
                    coord.y = (vx_uint32)xml_ulong(cur);
                }
            }
            status |= vxAddArrayItems(array, 1, &coord, 0);
        } else if (tag == COORDINATES3D_TAG) {
            vx_coordinates3d_t coord = {0};
            XML_FOREACH_CHILD_TAG(cur, tag, tags) {
                if (tag == X_TAG) {
                    coord.x = (vx_uint32)xml_ulong(cur);
                } else if (tag == Y_TAG) {
                    coord.y = (vx_uint32)xml_ulong(cur);
                } else if (tag == Z_TAG) {
                    coord.z = (vx_uint32)xml_ulong(cur);
                }
            }
            status |= vxAddArrayItems(array, 1, &coord, 0);
        } else if (tag == RECTANGLE_TAG) {
            vx_rectangle_t rect = {0};
            XML_FOREACH_CHILD_TAG(cur, tag, tags) {
                if (tag == START_X_TAG) {
                    rect.start_x = (vx_uint32)xml_ulong(cur);
                } else if (tag == START_Y_TAG) {
                    rect.start_y = (vx_uint32)xml_ulong(cur);
                } else if (tag == END_X_TAG) {
                    rect.end_x = (vx_uint32)xml_ulong(cur);
                } else if (tag == END_Y_TAG) {
                    rect.end_y = (vx_uint32)xml_ulong(cur);
                }
            }
            status |= vxAddArrayItems(array, 1, &rect, 0);
        } else if (tag == USER_TAG) {
            char *string = (char *)xmlNodeListGetString(cur->doc, cur->children, 1);
            char tokens[5] = " \t\n\r";
            char *tmp = strtok(string, tokens);
            vx_uint8 *v = calloc(array->item_size, sizeof(vx_uint8));
            vx_uint32 j = 0;
            while (tmp) {
                sscanf(tmp, "%hhu", &v[j++]);
                tmp = strtok(NULL, tokens);
            }
            status |= vxAddArrayItems(array, 1, v, 0);
            free(v);
            free(string);
        } else {
            char *string = (char *)xmlNodeListGetString(cur->doc, cur->children, 1);
            char tokens[5] = " \t\n\r";
            char *tmp = strtok(string, tokens);
            while (tmp) {
                //if (tag == CHAR_TAG) {
                //    vx_char v;
                //    sscanf(tmp, "%s", &v);
                //    status |= vxAddArrayItems(array, 1, &v, 0);
                //} else
                if (tag == UINT8_TAG) {
                    vx_uint8 v;
                    sscanf(tmp, "%hhu", &v);
                    status |= vxAddArrayItems(array, 1, &v, 0);
                } else if (tag == UINT16_TAG) {
                    vx_uint16 v;
                    sscanf(tmp, "%hu", &v);
                    status |= vxAddArrayItems(array, 1, &v, 0);
                } else if (tag == UINT32_TAG) {
                    vx_uint32 v;
                    sscanf(tmp, "%u", &v);
                    status |= vxAddArrayItems(array, 1, &v, 0);
                } else if (tag == UINT64_TAG) {
                    vx_uint64 v;
                    sscanf(tmp, "%lu", &v);
                    status |= vxAddArrayItems(array, 1, &v, 0);
                } else if (tag == INT8_TAG) {
                    vx_int8 v;
                    sscanf(tmp, "%hhd", &v);
                    status |= vxAddArrayItems(array, 1, &v, 0);
                } else if (tag == INT16_TAG) {
                    vx_int16 v;
                    sscanf(tmp, "%hd", &v);
                    status |= vxAddArrayItems(array, 1, &v, 0);
                } else if (tag == INT32_TAG) {
                    vx_int32 v;
                    sscanf(tmp, "%d", &v);
                    status |= vxAddArrayItems(array, 1, &v, 0);
                } else if (tag == INT64_TAG) {
                    vx_int64 v;
                    sscanf(tmp, "%ld", &v);
                    status |= vxAddArrayItems(array, 1, &v, 0);
                } else if (tag == FLOAT32_TAG) {
                    vx_float32 v;
                    sscanf(tmp, "%f", &v);
                    status |= vxAddArrayItems(array, 1, &v, 0);
                } else if (tag == FLOAT64_TAG) {
                    vx_float64 v;
                    sscanf(tmp, "%lf", &v);
                    status |= vxAddArrayItems(array, 1, &v, 0);
                } else if (tag == DF_IMAGE_TAG) {
                    vx_df_image v = VX_DF_IMAGE(tmp[0],tmp[1],tmp[2],tmp[3]);
                    status |= vxAddArrayItems(array, 1, &v, 0);
                } else if (tag == ENUM_TAG) {
                    vx_enum v;
                    sscanf(tmp, "%d", &v);
                    status |= vxAddArrayItems(array, 1, &v, 0);
                } else if (tag == SIZE_TAG) {
                    vx_size v;
                    sscanf(tmp, VX_FMT_SIZE, &v);
                    status |= vxAddArrayItems(array, 1, &v, 0);
                } else if (tag == BOOL_TAG) {
                    vx_bool v = vx_false_e;
                    if (strcmp(tmp, "true") == 0)
                    {
                        v = vx_true_e;
                    }
                    status |= vxAddArrayItems(array, 1, &v, 0);
                }
                tmp = strtok(NULL, tokens);
            }
            free(string);
        }
    }
    return status;
}

static vx_status vxLoadDataForPyramid(vx_pyramid pyr, xmlNodePtr cur, vx_reference refs[], vx_size total, vx_size levels)
{
    vx_status status = VX_SUCCESS;
    vx_xml_tag_e tag = UNKNOWN_TAG;
    XML_FOREACH_CHILD_TAG (cur, tag, tags) {
        if (tag == IMAGE_TAG) {
            vx_uint32 refIdx = xml_prop_ulong(cur, "reference");
            vx_uint32 width = xml_prop_ulong(cur,"width");
            vx_uint32 height = xml_prop_ulong(cur,"height");
            /* level is optional, so may not be there */
            vx_int32 level = -1; /* use -1 to indicate not specified */
            {
                xmlChar *string = xmlGetProp(cur, (const xmlChar *)"level");
                if (string)
                    level = strtoul((const char *)string, NULL, 10);
                free(string);
            }
            if (refIdx < total)
            {
                if((level >= 0) && (level < levels)) {
                    if(pyr->levels[level]->width == width &&
                       pyr->levels[level]->height == height) {
                        refs[refIdx] = (vx_reference)pyr->levels[level];
                        vxSetName(refs[refIdx], cur);
                        vxIncrementReference(refs[refIdx], VX_INTERNAL);
                    } else {
                        VX_PRINT(VX_ZONE_ERROR, "Pyramid image settings doesn't match generated pyramid image!\n");
                        return VX_ERROR_INVALID_PARAMETERS;
                    }
                } else {
                    vx_uint32 i;
                    for(i = 0; i < levels; i++) {
                        if(pyr->levels[i]->width == width &&
                           pyr->levels[i]->height == height) {
                            refs[refIdx] = (vx_reference)pyr->levels[i];
                            vxSetName(refs[refIdx], cur);
                            vxIncrementReference(refs[refIdx], VX_INTERNAL);
                            break;
                        }
                    }
                    if(i == levels) {
                        /* This image doesn't match width/height of generated pyramid images */
                        VX_PRINT(VX_ZONE_ERROR, "Pyramid image settings doesn't match generated pyramid images!\n");
                        return VX_ERROR_INVALID_PARAMETERS;
                    }
                }
                status |= vxLoadDataForImage((vx_image)refs[refIdx], cur);
            } else {
                status |= VX_ERROR_INVALID_PARAMETERS;
            }
        } else {
            status |= VX_ERROR_NOT_SUPPORTED;
        }
    }
    return status;
}

static vx_status vxLoadDataForMatrix(vx_matrix matrix, xmlNodePtr cur, vx_size cols, vx_size rows, vx_enum type)
{
    vx_status status = VX_SUCCESS;
    vx_xml_tag_e tag = UNKNOWN_TAG;

    if (XML_HAS_CHILD(cur))
    {
        void *ptr = calloc(rows*cols,vxMetaSizeOfType(type));
        if (ptr)
        {
            if( (status = vxAccessMatrix(matrix, ptr)) == VX_SUCCESS)
            {
                XML_FOREACH_CHILD_TAG (cur, tag, tags) {
                    vx_uint32 row = xml_prop_ulong(cur, "row");
                    vx_uint32 col = xml_prop_ulong(cur, "column");
                    vx_uint32 off = (row*cols + col);
                    if (row < rows && col < cols)
                    {
                        if (tag == FLOAT32_TAG) {
                            vx_float32 *tmp = (vx_float32 *)ptr;
                            tmp[off] = xml_float(cur);
                            //VX_PRINT(VX_ZONE_INFO, "Read mat[%u][%u]=%lf\n", row, col, tmp[off]);
                        } else if (tag == INT32_TAG) {
                            vx_uint32 *tmp = (vx_uint32 *)ptr;
                            tmp[off] = (vx_int32)xml_ulong(cur);
                            //VX_PRINT(VX_ZONE_INFO, "Read mat[%u][%u]=%d\n", row, col, tmp[off]);
                        }
                    } else {
                        VX_PRINT(VX_ZONE_ERROR, "Invalid location!\n");
                        return VX_ERROR_INVALID_VALUE;
                    }
                }
                status = vxCommitMatrix(matrix, ptr);
            }
            free(ptr);
        } else {
            status = VX_ERROR_NO_MEMORY;
        }
    }
    return status;
}

static vx_status vxLoadDataForLut(vx_lut lut, xmlNodePtr cur, vx_enum type, vx_size count)
{
    vx_status status = VX_SUCCESS;
    vx_xml_tag_e tag = UNKNOWN_TAG;

    if (XML_HAS_CHILD(cur)) {
        void *ptr = NULL;
        if( (status = vxAccessLUT(lut, &ptr, VX_WRITE_ONLY)) == VX_SUCCESS)
        {
            XML_FOREACH_CHILD_TAG (cur, tag, tags) {
                if (tag == UINT8_TAG) {
                    vx_uint32 index = xml_prop_ulong(cur, "index");
                    vx_uint8 *tmp = (vx_uint8 *)ptr;
                    tmp[index] = (vx_uint8)xml_ulong(cur);
                }
            }
            status = vxCommitLUT(lut, ptr);
        }
    }
    return status;
}

static vx_status vxLoadDataForConvolution(vx_convolution conv, xmlNodePtr cur, vx_size cols, vx_size rows)
{
    vx_status status = VX_SUCCESS;
    vx_xml_tag_e tag = UNKNOWN_TAG;

    if (XML_HAS_CHILD(cur))
    {
        void *ptr = calloc(rows*cols,sizeof(vx_int16));
        if (ptr)
        {
            if( (status = vxAccessConvolutionCoefficients(conv, ptr)) == VX_SUCCESS)
            {
                XML_FOREACH_CHILD_TAG (cur, tag, tags) {
                    vx_uint32 row = xml_prop_ulong(cur, "row");
                    vx_uint32 col = xml_prop_ulong(cur, "column");
                    vx_uint32 off = (row*cols) + col;
                    if (row < rows && col < cols)
                    {
                        if (tag == INT16_TAG) {
                            vx_int16 *tmp = (vx_int16 *)ptr;
                            tmp[off] = (vx_int16)xml_ulong(cur);
                            //VX_PRINT(VX_ZONE_INFO, "Read conv[%u][%u]=%hd\n", row, col, tmp[off]);
                        }
                    } else {
                        VX_PRINT(VX_ZONE_ERROR, "Invalid location!\n");
                        return VX_ERROR_INVALID_VALUE;
                    }
                }
                status = vxCommitConvolutionCoefficients(conv, ptr);
            }
            free(ptr);
        } else {
            status = VX_ERROR_NO_MEMORY;
        }
    }
    return status;
}

static vx_status vxLoadDataForRemap(vx_remap remap, xmlNodePtr cur)
{
    vx_status status = VX_SUCCESS;
    vx_xml_tag_e tag = UNKNOWN_TAG;

    XML_FOREACH_CHILD_TAG (cur, tag, tags) {
        if (tag == POINT_TAG) {
            vx_float32 src_x = xml_prop_float(cur, "src_x");
            vx_float32 src_y = xml_prop_float(cur, "src_y");
            vx_uint32 dst_x = xml_prop_ulong(cur, "dst_x");
            vx_uint32 dst_y = xml_prop_ulong(cur, "dst_y");
            status |= vxSetRemapPoint(remap, dst_x, dst_y, src_x, src_y);
            if (status != VX_SUCCESS) {
                return status;
            }
        }
    }
    return status;
}

static vx_status vxLoadDataForDistribution(vx_distribution dist, xmlNodePtr cur, vx_size bins)
{
    vx_status status = VX_SUCCESS;
    vx_xml_tag_e tag = UNKNOWN_TAG;

    if (XML_HAS_CHILD(cur))
    {
        void *ptr = NULL;
        if ((status = vxAccessDistribution(dist, &ptr, VX_WRITE_ONLY)) == VX_SUCCESS) {
            XML_FOREACH_CHILD_TAG (cur, tag, tags) {
                if (tag == FREQUENCY_TAG) {
                    vx_uint32 bin = xml_prop_ulong(cur, "bin");
                    if (bin < bins) {
                        vx_int32 *tmp = (vx_int32 *)ptr;
                        vx_char value[11];
                        xml_string(cur, value, sizeof(value));
                        sscanf(value,"%d", &tmp[bin]);
                    } else {
                        return VX_ERROR_INVALID_VALUE;
                    }
                }
            }
            status = vxCommitDistribution(dist, ptr);
        }
    }
    return status;
}

static vx_status vxLoadDataForScalar(vx_scalar scalar, xmlNodePtr cur)
{
    vx_status status = VX_SUCCESS;
    vx_xml_tag_e tag = UNKNOWN_TAG;

    XML_FOREACH_CHILD_TAG (cur, tag, tags) {
        vx_char value[100];
        if (tag == CHAR_TAG) {
            vx_char c;
            xml_string(cur, value, sizeof(value));
            sscanf(value, "%c", &c);
            vxCommitScalarValue(scalar, &c);
        } else if (tag == BOOL_TAG) {
            vx_bool v = vx_false_e;
            xml_string(cur, value, sizeof(value));
            if (strncmp(value, "true", sizeof(value)) == 0)
                v = vx_true_e;
            vxCommitScalarValue(scalar, &v);
        } else if (tag == UINT8_TAG) {
            vx_uint8 v = 0u;
            xml_string(cur, value, sizeof(value));
            sscanf(value, "%hhu", &v);
            vxCommitScalarValue(scalar, &v);
        } else if (tag == UINT16_TAG) {
            vx_uint16 v = 0u;
            xml_string(cur, value, sizeof(value));
            sscanf(value, "%hu", &v);
            vxCommitScalarValue(scalar, &v);
        } else if (tag == UINT32_TAG) {
            vx_uint32 v = 0u;
            xml_string(cur, value, sizeof(value));
            sscanf(value, "%u", &v);
            vxCommitScalarValue(scalar, &v);
        } else if (tag == UINT64_TAG) {
            vx_uint64 v = 0u;
            xml_string(cur, value, sizeof(value));
            sscanf(value, "%lu", &v);
            vxCommitScalarValue(scalar, &v);
        } else if (tag == INT8_TAG) {
            vx_int8 v = 0u;
            xml_string(cur, value, sizeof(value));
            sscanf(value, "%hhd", &v);
            vxCommitScalarValue(scalar, &v);
        } else if (tag == INT16_TAG) {
            vx_int16 v = 0u;
            xml_string(cur, value, sizeof(value));
            sscanf(value, "%hd", &v);
            vxCommitScalarValue(scalar, &v);
        } else if (tag == INT32_TAG) {
            vx_int32 v = 0u;
            xml_string(cur, value, sizeof(value));
            sscanf(value, "%d", &v);
            vxCommitScalarValue(scalar, &v);
        } else if (tag == INT64_TAG) {
            vx_int64 v = 0u;
            xml_string(cur, value, sizeof(value));
            sscanf(value, "%ld", &v);
            vxCommitScalarValue(scalar, &v);
        } else if (tag == SIZE_TAG) {
            vx_size v = xml_ulong(cur);
            vxCommitScalarValue(scalar, &v);
        } else if (tag == ENUM_TAG) {
            vx_enum v = 0;
            xml_string(cur, value, sizeof(value));
            sscanf(value, "%d", &v);
            vxCommitScalarValue(scalar, &v);
        } else if (tag == FLOAT32_TAG) {
            vx_float32 f = xml_float(cur);
            vxCommitScalarValue(scalar, &f);
        } else if (tag == FLOAT64_TAG) {
            vx_float64 f = xml_double(cur);
            vxCommitScalarValue(scalar, &f);
        } else if (tag == DF_IMAGE_TAG) {
            vx_df_image value = 0;
            xml_string(cur, (vx_char *)&value, sizeof(value));
            vxCommitScalarValue(scalar, &value);
        }
    }
    return status;
}

static vx_status vxImportFromXMLImage(vx_reference ref, xmlNodePtr cur, vx_reference refs[], vx_size total, vx_bool is_virtual)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 width = xml_prop_ulong(cur,"width");
    vx_uint32 height = xml_prop_ulong(cur,"height");
    vx_df_image format = VX_DF_IMAGE_VIRT;
    vx_uint32 refIdx = xml_prop_ulong(cur, "reference");
    vx_image image = 0;
    xml_prop_string(cur, "format", (vx_char *)&format, 4);
    if (is_virtual == vx_false_e && format == VX_DF_IMAGE_VIRT)
        return VX_ERROR_INVALID_FORMAT;
    if (refIdx >= total)
        return VX_ERROR_INVALID_PARAMETERS;
    if (is_virtual == vx_true_e)
    {
        image = vxCreateVirtualImage((vx_graph)ref, width, height, format);
        status = vxGetStatus((vx_reference)image);
    }
    else
    {
        image = vxCreateImage((vx_context)ref, width, height, format);
        status = vxGetStatus((vx_reference)image);
        if (status == VX_SUCCESS) {
            status = vxLoadDataForImage(image, cur);
        }
    }
    if (status == VX_SUCCESS) {
        refs[refIdx] = (vx_reference)image;
        vxSetName(refs[refIdx], cur);
        vxInternalizeReference(refs[refIdx]);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to create Image! status=%d\n", status);
    }
    return status;
}

static vx_status vxImportFromXMLArray(vx_reference ref, xmlNodePtr cur, vx_reference refs[], vx_size total, vx_bool is_virtual)
{
    vx_status status = VX_SUCCESS;
    vx_char typename[32];
    vx_uint32 refIdx = xml_prop_ulong(cur, "reference");
    vx_size capacity = xml_prop_ulong(cur, "capacity");
    vx_array array = 0;
    vx_enum type = VX_TYPE_INVALID;
    vx_uint32 userNum;
    xml_prop_string(cur, "elemType", typename, sizeof(typename));

    if(vxTypeFromString(typename, &type) != VX_SUCCESS) { /* Type was not found, check if it is a user type */
        if(sscanf(typename, "USER_STRUCT_%d", &userNum) == 1) {
            if(vxStructGetEnum(user_struct_table, userNum, &type) != VX_SUCCESS) {
                return VX_ERROR_INVALID_PARAMETERS; /* INVALID type */
            }
        } else {
            return VX_ERROR_INVALID_PARAMETERS; /* INVALID type */
        }
    }

    if (refIdx >= total)
        return VX_ERROR_INVALID_PARAMETERS;
    if (is_virtual == vx_true_e)
    {
        array = vxCreateVirtualArray((vx_graph)ref, type, capacity);
        status = vxGetStatus((vx_reference)array);
    }
    else
    {
        array = vxCreateArray((vx_context)ref, type, capacity);
        status = vxGetStatus((vx_reference)array);
        if (status == VX_SUCCESS) {
            status = vxLoadDataForArray(array, cur);
        }
    }
    if (status == VX_SUCCESS) {
        refs[refIdx] = (vx_reference)array;
        vxSetName(refs[refIdx], cur);
        vxInternalizeReference(refs[refIdx]);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to create Array! status=%d\n", status);
    }
    return status;
}

static vx_status vxImportFromXMLPyramid(vx_reference ref, xmlNodePtr cur, vx_reference refs[], vx_size total, vx_bool is_virtual)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 refIdx = xml_prop_ulong(cur, "reference");
    vx_uint32 width  = xml_prop_ulong(cur, "width");
    vx_uint32 height = xml_prop_ulong(cur, "height");
    vx_float32 scale  = xml_prop_float(cur, "scale");
    vx_size levels = xml_prop_ulong(cur, "levels");
    vx_df_image format = VX_DF_IMAGE_VIRT;
    vx_pyramid pyr = NULL;
    xml_prop_string(cur, "format", (vx_char *)&format, 4);

    if (is_virtual == vx_false_e && format == VX_DF_IMAGE_VIRT)
        return VX_ERROR_INVALID_FORMAT;
    if (refIdx >= total)
        return VX_ERROR_INVALID_PARAMETERS;
    if (is_virtual == vx_true_e)
    {
        pyr = vxCreateVirtualPyramid((vx_graph)ref, levels, scale, width, height, format);
        status = vxGetStatus((vx_reference)pyr);
    }
    else
    {
        /* Internally creates "levels" number of image objects */
        pyr = vxCreatePyramid((vx_context)ref, levels, scale, width, height, format);
        status = vxGetStatus((vx_reference)pyr);
        if (status == VX_SUCCESS) {
            status = vxLoadDataForPyramid(pyr, cur, refs, total, levels);
        }
    }
    if (status == VX_SUCCESS) {
        refs[refIdx] = (vx_reference)pyr;
        vxSetName(refs[refIdx], cur);
        vxInternalizeReference(refs[refIdx]);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to create Pyramid! status=%d\n", status);
    }
    return status;
}

VX_API_ENTRY vx_import VX_API_CALL vxImportFromXML(vx_context context,
                                 vx_char xmlfile[])
{
    xmlDocPtr doc = xmlParseFile(xmlfile);
    xmlNodePtr root = xmlDocGetRootElement(doc);
    xmlNodePtr cur = root;
    vx_status status = VX_SUCCESS;
    vx_xml_tag_e tag = UNKNOWN_TAG;
    vx_size total = 0ul, counted = 0ul;
    vx_enum type = VX_TYPE_INVALID;
    vx_reference *refs = NULL;
    vx_import import = NULL;

    if (doc == NULL) {
        VX_PRINT(VX_ZONE_ERROR, "Could not parse %s\n", xmlfile);
        vxAddLogEntry(&context->base, VX_ERROR_INVALID_PARAMETERS, "Could not parse %s\n", xmlfile);
        import = (vx_import)vxGetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
        return import;
    }

    user_struct_table = calloc(VX_INT_MAX_USER_STRUCTS, sizeof(xml_struct_t));
    if (user_struct_table == NULL) {
        VX_PRINT(VX_ZONE_ERROR, "Calloc failed\n");
        vxAddLogEntry(&context->base, VX_ERROR_NO_MEMORY, "Calloc failed\n");
        import = (vx_import)vxGetErrorObject(context, VX_ERROR_NO_MEMORY);
        return import;
    }

    if (root == NULL || xmlStrcmp(cur->name, (const xmlChar *)"openvx") != 0) {
        VX_PRINT(VX_ZONE_ERROR, "%s is not wellformed\n", xmlfile);
        vxAddLogEntry(&context->base, VX_ERROR_INVALID_FORMAT, "%s is not wellformed\n", xmlfile);
        import = (vx_import)vxGetErrorObject(context, VX_ERROR_INVALID_FORMAT);
        goto exit;
    }

    total = xml_prop_ulong(cur, "references");
    if (total > VX_INT_MAX_REF) {
        VX_PRINT(VX_ZONE_ERROR, "Total references = %d too high for this implementation\n", total);
        vxAddLogEntry(&context->base, VX_ERROR_INVALID_FORMAT, "Total references = %d too high for this implementation\n", total);
        import = (vx_import)vxGetErrorObject(context, VX_ERROR_INVALID_FORMAT);
        goto exit;
    }

    import = vxCreateImportInt(context, VX_IMPORT_TYPE_XML, total);
    if (import == NULL || import->base.type != VX_TYPE_IMPORT) {
        goto exit;
    }

    refs = (vx_reference*)((vx_import_t *)import)->refs;

    XML_FOREACH_CHILD_TAG (cur, tag, tags) {
        if (tag == LIBRARY_TAG) {
            vx_char library[256];
            xml_string(cur, library, sizeof(library));
            status = vxLoadKernels(context, library);
            if (status != VX_SUCCESS) {
                goto exit_error;
            }
        } else if (tag == STRUCT_TAG) {
            vx_size size = xml_prop_ulong(cur, "size");
            vx_char label[256];
            vx_enum e_userStruct;
            xml_string(cur, label, sizeof(label));
            e_userStruct = vxRegisterUserStruct(context, size);
            status = vxStructSetEnum(user_struct_table, label, e_userStruct);
        } else if (tag == IMAGE_TAG) {
            status = vxImportFromXMLImage((vx_reference)context, cur, refs, total, vx_false_e);
        } else if (tag == ARRAY_TAG) {
            status = vxImportFromXMLArray((vx_reference)context, cur, refs, total, vx_false_e);
        } else if (tag == PYRAMID_TAG) {
            status = vxImportFromXMLPyramid((vx_reference)context, cur, refs, total, vx_false_e);
        } else if (tag == GRAPH_TAG) {
            vx_uint32 refIdx = xml_prop_ulong(cur, "reference");
            vx_graph graph = vxCreateGraph(context);
            if (refIdx < total) {
                refs[refIdx] = (vx_reference)graph;
                vxSetName(refs[refIdx], cur);
                vxInternalizeReference(refs[refIdx]);
            } else
            {
                REFNUM_ERROR;
                goto exit_error;
            }
            XML_FOREACH_CHILD_TAG (cur, tag, tags) {
                if (tag == NODE_TAG) {
                    vx_kernel k = 0;
                    vx_node n = 0;
                    vx_uint32 refIdx = xml_prop_ulong(cur, "reference");
                    XML_FOREACH_CHILD_TAG (cur, tag, tags) {
                        if (tag == KERNEL_TAG) {
                            vx_char name[VX_MAX_KERNEL_NAME];
                            xml_string(cur, name, sizeof(name));
                            k = vxGetKernelByName(context, name);
                            n = vxCreateGenericNode(graph, k);
                            vxReleaseKernel(&k);
                            if (refIdx < total) {
                                refs[refIdx] = (vx_reference)n;
                                vxSetName(refs[refIdx], cur);
                                vxInternalizeReference(refs[refIdx]);
                            } else
                            {
                                REFNUM_ERROR;
                                goto exit_error;
                            }
                        } else if (tag == PARAMETER_TAG) {
                            /* need to do this on second pass */
                        }
                    }
                } else if (tag == IMAGE_TAG) {
                    status |= vxImportFromXMLImage((vx_reference)graph, cur, refs, total, vx_true_e);
                } else if (tag == ARRAY_TAG) {
                    status |= vxImportFromXMLArray((vx_reference)graph, cur, refs, total, vx_true_e);
                } else if (tag == PYRAMID_TAG) {
                    status |= vxImportFromXMLPyramid((vx_reference)graph, cur, refs, total, vx_true_e);
                }
            }
        } else if (tag == SCALAR_TAG) {
            vx_uint32 refIdx = xml_prop_ulong(cur, "reference");
            vx_size nullReference = 0;
            void *ptr = &nullReference;
            vx_char typename[20];
            xml_prop_string(cur, "elemType", typename, sizeof(typename));
            vxTypeFromString(typename, &type);
            if (refIdx >= total) {
                REFNUM_ERROR;
                goto exit_error;
            }
            refs[refIdx] = (vx_reference)vxCreateScalar(context, type, &ptr);
            vxSetName(refs[refIdx], cur);
            vxInternalizeReference(refs[refIdx]);
            if ((status |= vxGetStatus(refs[refIdx])) == VX_SUCCESS) {
                vx_enum scalartype = VX_TYPE_INVALID;
                vxQueryScalar((vx_scalar)refs[refIdx], VX_SCALAR_ATTRIBUTE_TYPE, &scalartype, sizeof(scalartype));
                if (type != scalartype)
                {
                    status = VX_ERROR_INVALID_TYPE;
                    VX_PRINT(VX_ZONE_ERROR, "Stated type mismatch!\n");
                    goto exit_error;
                }
                status |= vxLoadDataForScalar((vx_scalar)refs[refIdx], cur);
            }
        } else if (tag == MATRIX_TAG) {
            vx_size rows = xml_prop_ulong(cur, "rows");
            vx_size cols = xml_prop_ulong(cur, "columns");
            vx_uint32 refIdx = xml_prop_ulong(cur, "reference");
            vx_char typename[32];
            xml_prop_string(cur, "elemType", typename, sizeof(typename));
            vxTypeFromString(typename, &type);
            if (refIdx >= total) {
                REFNUM_ERROR;
                goto exit_error;
            }
            refs[refIdx] = (vx_reference)vxCreateMatrix(context, type, cols, rows);
            vxSetName(refs[refIdx], cur);
            vxInternalizeReference(refs[refIdx]);
            if ((status |= vxGetStatus(refs[refIdx])) == VX_SUCCESS) {
                status |= vxLoadDataForMatrix((vx_matrix)refs[refIdx], cur, cols, rows, type);
            }
        } else if (tag == CONVOLUTION_TAG) {
            vx_size rows = xml_prop_ulong(cur, "rows");
            vx_size cols = xml_prop_ulong(cur, "columns");
            vx_uint32 scale = xml_prop_ulong(cur, "scale");
            vx_uint32 refIdx = xml_prop_ulong(cur, "reference");
            if (refIdx >= total) {
                REFNUM_ERROR;
                goto exit_error;
            }
            refs[refIdx] = (vx_reference)vxCreateConvolution(context, cols, rows);
            vxSetName(refs[refIdx], cur);
            vxInternalizeReference(refs[refIdx]);
            if ((status |= vxGetStatus(refs[refIdx])) == VX_SUCCESS) {
                if(!scale) scale = 1;
                status |= vxSetConvolutionAttribute((vx_convolution)refs[refIdx], VX_CONVOLUTION_ATTRIBUTE_SCALE,
                                                    &scale, sizeof(scale));
                status |= vxLoadDataForConvolution((vx_convolution)refs[refIdx], cur, cols, rows);
            }
        } else if (tag == LUT_TAG) {
            vx_uint32 refIdx = xml_prop_ulong(cur, "reference");
            vx_size count = xml_prop_ulong(cur, "count");
            vx_char typename[32] = "VX_TYPE_UINT8";
            xml_prop_string(cur, "elemType", typename, sizeof(typename));
            vxTypeFromString(typename, &type);
            if (refIdx >= total) {
                REFNUM_ERROR;
                goto exit_error;
            }
            if (count == 0)
                count = 256;
            refs[refIdx] = (vx_reference)vxCreateLUT(context, type, count);
            vxSetName(refs[refIdx], cur);
            vxInternalizeReference(refs[refIdx]);
            if ((status |= vxGetStatus(refs[refIdx])) == VX_SUCCESS) {
                status |= vxLoadDataForLut((vx_lut)refs[refIdx], cur, type, count);
            }
        } else if (tag == REMAP_TAG) {
            vx_uint32 refIdx = xml_prop_ulong(cur, "reference");
            vx_uint32 src_width = xml_prop_ulong(cur, "src_width");
            vx_uint32 src_height= xml_prop_ulong(cur, "src_height");
            vx_uint32 dst_width = xml_prop_ulong(cur, "dst_width");
            vx_uint32 dst_height = xml_prop_ulong(cur, "dst_height");
            if (refIdx >= total) {
                REFNUM_ERROR;
                goto exit_error;
            }
            refs[refIdx] = (vx_reference)vxCreateRemap(context, src_width, src_height, dst_width, dst_height);
            vxSetName(refs[refIdx], cur);
            vxInternalizeReference(refs[refIdx]);
            if ((status |= vxGetStatus(refs[refIdx])) == VX_SUCCESS) {
                status |= vxLoadDataForRemap((vx_remap)refs[refIdx], cur);
            }
        } else if (tag == DISTRIBUTION_TAG) {
            vx_uint32 refIdx = xml_prop_ulong(cur, "reference");
            vx_size bins = xml_prop_ulong(cur, "bins");
            vx_size range = xml_prop_ulong(cur, "range");
            vx_size offset = xml_prop_ulong(cur, "offset");
            if (refIdx >= total) {
                REFNUM_ERROR;
                goto exit_error;
            }
            refs[refIdx] = (vx_reference)vxCreateDistribution(context, bins, offset, range);
            vxSetName(refs[refIdx], cur);
            vxInternalizeReference(refs[refIdx]);
            if ((status |= vxGetStatus(refs[refIdx])) == VX_SUCCESS) {
                status |= vxLoadDataForDistribution((vx_distribution)refs[refIdx], cur, bins);
            }
        } else if (tag == THRESHOLD_TAG) {
            vx_uint32 refIdx = xml_prop_ulong(cur, "reference");
            vx_char typename[32] = "VX_TYPE_UINT8"; // default value
            xml_prop_string(cur, "elemType", typename, sizeof(typename));
            vxTypeFromString(typename, &type);
            status = VX_SUCCESS;
            if (refIdx < total && type == VX_TYPE_UINT8) {
                vx_int32 true_value = (vx_int32)xml_prop_ulong(cur, "true_value");
                vx_int32 false_value = (vx_int32)xml_prop_ulong(cur, "false_value");
                XML_FOREACH_CHILD_TAG (cur, tag, tags) {
                    if (tag == BINARY_TAG) {
                        vx_int32 value = (vx_int32)xml_ulong(cur);
                        refs[refIdx] = (vx_reference)vxCreateThreshold(context, VX_THRESHOLD_TYPE_BINARY, type);
                        vxSetName(refs[refIdx], cur);
                        vxInternalizeReference(refs[refIdx]);
                        status |= vxSetThresholdAttribute((vx_threshold)refs[refIdx], VX_THRESHOLD_ATTRIBUTE_THRESHOLD_VALUE, &value, sizeof(value));
                    } else if (tag == RANGE_TAG) {
                        vx_int32 upper = (vx_int32)xml_prop_ulong(cur, "upper");
                        vx_int32 lower = (vx_int32)xml_prop_ulong(cur, "lower");
                        refs[refIdx] = (vx_reference)vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, type);
                        vxSetName(refs[refIdx], cur);
                        vxInternalizeReference(refs[refIdx]);
                        status |= vxSetThresholdAttribute((vx_threshold)refs[refIdx], VX_THRESHOLD_ATTRIBUTE_THRESHOLD_UPPER, &upper, sizeof(upper));
                        status |= vxSetThresholdAttribute((vx_threshold)refs[refIdx], VX_THRESHOLD_ATTRIBUTE_THRESHOLD_LOWER, &lower, sizeof(lower));
                    }
                    status |= vxSetThresholdAttribute((vx_threshold)refs[refIdx], VX_THRESHOLD_ATTRIBUTE_TRUE_VALUE , &true_value, sizeof(true_value));
                    status |= vxSetThresholdAttribute((vx_threshold)refs[refIdx], VX_THRESHOLD_ATTRIBUTE_FALSE_VALUE , &false_value, sizeof(false_value));
                }
            } else {
                status = VX_ERROR_INVALID_VALUE;
                goto exit_error;
            }
        } else if (tag == DELAY_TAG) {
            vx_uint32 refIdx = xml_prop_ulong(cur, "reference");
            vx_uint32 count  = xml_prop_ulong(cur, "count");
            vx_delay delay = NULL;
            vx_uint32 childNum = 0;
            vx_char objType[32] = "VX_TYPE_IMAGE";  // default value
            xml_prop_string(cur, "objType", objType, sizeof(objType));
            vxTypeFromString(objType, &type);
            if (refIdx < total) {
                XML_FOREACH_CHILD_TAG (cur, tag, tags) {
                    switch(tag) {
                        case IMAGE_TAG:
                        {
                            vx_uint32 width = xml_prop_ulong(cur,"width");
                            vx_uint32 height = xml_prop_ulong(cur,"height");
                            if(childNum == 0) { /* Create delay object based on first child */
                                vx_image exemplar = NULL;
                                vx_df_image format = VX_DF_IMAGE_VIRT;
                                xml_prop_string(cur, "format", (vx_char *)&format, 4);
                                status = vxReserveReferences(context, count+1);
                                exemplar = vxCreateImage(context, width, height, format);
                                status |= vxReleaseReferences(context, count+1);
                                status |= vxGetStatus((vx_reference)exemplar);
                                if (status == VX_SUCCESS) {
                                    delay = vxCreateDelay(context, (vx_reference)exemplar, count);
                                    status = vxGetStatus((vx_reference)delay);
                                    vxReleaseImage(&exemplar);
                                    refs[refIdx] = (vx_reference)delay;
                                    vxSetName(refs[refIdx], cur);
                                    vxInternalizeReference(refs[refIdx]);
                                }
                            }
                            refIdx = xml_prop_ulong(cur, "reference");
                            if (refIdx < total)
                            {
                                if(childNum < count) {
                                    if(((vx_image)delay->refs[childNum])->width == width &&
                                       ((vx_image)delay->refs[childNum])->height == height) {
                                        refs[refIdx] = (vx_reference)delay->refs[childNum];
                                        vxSetName(refs[refIdx], cur);
                                        vxIncrementReference(refs[refIdx], VX_INTERNAL);
                                    } else {
                                        status = VX_ERROR_INVALID_PARAMETERS;
                                        VX_PRINT(VX_ZONE_ERROR, "Delay image settings doesn't match generated delay image!\n");
                                        goto exit_error;
                                    }
                                } else {
                                    status = VX_ERROR_INVALID_PARAMETERS;
                                    VX_PRINT(VX_ZONE_ERROR, "Delay has more child nodes than indicated in count!\n");
                                    goto exit_error;
                                }
                            } else {
                                REFNUM_ERROR;
                                goto exit_error;
                            }
                            childNum++;
                            status |= vxLoadDataForImage((vx_image)refs[refIdx], cur);
                            break;
                        }
                        case ARRAY_TAG:
                        {
                            vx_size capacity = xml_prop_ulong(cur, "capacity");
                            vx_char typename[32];
                            vx_enum type = VX_TYPE_INVALID;
                            vx_uint32 userNum;
                            xml_prop_string(cur, "elemType", typename, sizeof(typename));

                            if(vxTypeFromString(typename, &type) != VX_SUCCESS) { /* Type was not found, check if it is a user type */
                                if(sscanf(typename, "USER_STRUCT_%d", &userNum) == 1) {
                                    if(vxStructGetEnum(user_struct_table, userNum, &type) != VX_SUCCESS) {
                                        status = VX_ERROR_INVALID_TYPE; /* INVALID type */
                                        goto exit_error;
                                    }
                                } else {
                                    status =  VX_ERROR_INVALID_TYPE; /* INVALID type */
                                    goto exit_error;
                                }
                            }

                            if(childNum == 0) { /* Create delay object based on first child */
                                vx_array exemplar = NULL;
                                status = vxReserveReferences(context, count+1);
                                exemplar = vxCreateArray(context, type, capacity);
                                status |= vxReleaseReferences(context, count+1);
                                status |= vxGetStatus((vx_reference)exemplar);
                                if (status == VX_SUCCESS) {
                                    delay = vxCreateDelay(context, (vx_reference)exemplar, count);
                                    status = vxGetStatus((vx_reference)delay);
                                    vxReleaseArray(&exemplar);
                                    refs[refIdx] = (vx_reference)delay;
                                    vxSetName(refs[refIdx], cur);
                                    vxInternalizeReference(refs[refIdx]);
                                }
                            }
                            refIdx = xml_prop_ulong(cur, "reference");
                            if (refIdx < total)
                            {
                                if(childNum < count) {
                                    if(((vx_array)delay->refs[childNum])->capacity == capacity &&
                                       ((vx_array)delay->refs[childNum])->item_type == type) {
                                        refs[refIdx] = (vx_reference)delay->refs[childNum];
                                        vxSetName(refs[refIdx], cur);
                                        vxIncrementReference(refs[refIdx], VX_INTERNAL);
                                    } else {
                                        status = VX_ERROR_INVALID_PARAMETERS;
                                        VX_PRINT(VX_ZONE_ERROR, "Delay array settings doesn't match generated delay array!\n");
                                        goto exit_error;
                                    }
                                } else {
                                    status = VX_ERROR_INVALID_PARAMETERS;
                                    VX_PRINT(VX_ZONE_ERROR, "Delay has more child nodes than indicated in count!\n");
                                    goto exit_error;
                                }
                            } else {
                                REFNUM_ERROR;
                                goto exit_error;
                            }
                            childNum++;
                            status |= vxLoadDataForArray((vx_array)refs[refIdx], cur);
                            break;
                        }
                        case PYRAMID_TAG:
                        {
                            vx_uint32 width = xml_prop_ulong(cur,"width");
                            vx_uint32 height = xml_prop_ulong(cur,"height");
                            vx_float32 scale  = xml_prop_float(cur, "scale");
                            vx_size levels = xml_prop_ulong(cur, "levels");
                            vx_df_image format = VX_DF_IMAGE_VIRT;
                            xml_prop_string(cur, "format", (vx_char *)&format, 4);
                            if(childNum == 0) { /* Create delay object based on first child */
                                vx_pyramid exemplar = NULL;
                                status = vxReserveReferences(context, count*(levels+1)+1);
                                exemplar = vxCreatePyramid(context, levels, scale, width, height, format);
                                status |= vxReleaseReferences(context, count*(levels+1)+1);
                                status |= vxGetStatus((vx_reference)exemplar);
                                if (status == VX_SUCCESS) {
                                    delay = vxCreateDelay(context, (vx_reference)exemplar, count);
                                    status = vxGetStatus((vx_reference)delay);
                                    vxReleasePyramid(&exemplar);
                                    refs[refIdx] = (vx_reference)delay;
                                    vxSetName(refs[refIdx], cur);
                                    vxInternalizeReference(refs[refIdx]);
                                }
                            }
                            refIdx = xml_prop_ulong(cur, "reference");
                            if (refIdx < total)
                            {
                                if(childNum < count) {
                                    if(((vx_pyramid)delay->refs[childNum])->width == width &&
                                       ((vx_pyramid)delay->refs[childNum])->height == height &&
                                       ((vx_pyramid)delay->refs[childNum])->numLevels == levels &&
                                       ((vx_pyramid)delay->refs[childNum])->scale == scale) {
                                        refs[refIdx] = (vx_reference)delay->refs[childNum];
                                        vxSetName(refs[refIdx], cur);
                                        vxIncrementReference(refs[refIdx], VX_INTERNAL);
                                    } else {
                                        status = VX_ERROR_INVALID_PARAMETERS;
                                        VX_PRINT(VX_ZONE_ERROR, "Delay pyramid settings doesn't match generated delay pyramid!\n");
                                        goto exit_error;
                                    }
                                } else {
                                    status = VX_ERROR_INVALID_PARAMETERS;
                                    VX_PRINT(VX_ZONE_ERROR, "Delay has more child nodes than indicated in count!\n");
                                    goto exit_error;
                                }
                            } else {
                                REFNUM_ERROR;
                                goto exit_error;
                            }
                            childNum++;
                            status |= vxLoadDataForPyramid((vx_pyramid)refs[refIdx], cur, refs, total, levels);
                            break;
                        }
                        case MATRIX_TAG:
                        {
                            vx_size rows = xml_prop_ulong(cur, "rows");
                            vx_size cols = xml_prop_ulong(cur, "columns");
                            vx_char typename[32];
                            xml_prop_string(cur, "elemType", typename, sizeof(typename));
                            vxTypeFromString(typename, &type);
                            if(childNum == 0) { /* Create delay object based on first child */
                                vx_matrix exemplar = NULL;
                                status = vxReserveReferences(context, count+1);
                                exemplar = vxCreateMatrix(context, type, cols, rows);
                                status |= vxReleaseReferences(context, count+1);
                                status |= vxGetStatus((vx_reference)exemplar);
                                if (status == VX_SUCCESS) {
                                    delay = vxCreateDelay(context, (vx_reference)exemplar, count);
                                    status = vxGetStatus((vx_reference)delay);
                                    vxReleaseMatrix(&exemplar);
                                    refs[refIdx] = (vx_reference)delay;
                                    vxSetName(refs[refIdx], cur);
                                    vxInternalizeReference(refs[refIdx]);
                                }
                            }
                            refIdx = xml_prop_ulong(cur, "reference");
                            if (refIdx < total)
                            {
                                if(childNum < count) {
                                    if(((vx_matrix)delay->refs[childNum])->rows == rows &&
                                       ((vx_matrix)delay->refs[childNum])->columns == cols) {
                                        refs[refIdx] = (vx_reference)delay->refs[childNum];
                                        vxSetName(refs[refIdx], cur);
                                        vxIncrementReference(refs[refIdx], VX_INTERNAL);
                                    } else {
                                        status = VX_ERROR_INVALID_PARAMETERS;
                                        VX_PRINT(VX_ZONE_ERROR, "Delay matrix settings doesn't match generated delay matrix!\n");
                                        goto exit_error;
                                    }
                                } else {
                                    status = VX_ERROR_INVALID_PARAMETERS;
                                    VX_PRINT(VX_ZONE_ERROR, "Delay has more child nodes than indicated in count!\n");
                                    goto exit_error;
                                }
                            } else {
                                REFNUM_ERROR;
                                goto exit_error;
                            }
                            childNum++;
                            status |= vxLoadDataForMatrix((vx_matrix)refs[refIdx], cur, cols, rows, type);
                            break;
                        }
                        case LUT_TAG:
                        {
                            vx_size lut_count = xml_prop_ulong(cur, "count");
                            vx_char typename[32] = "VX_TYPE_UINT8";
                            xml_prop_string(cur, "elemType", typename, sizeof(typename));
                            vxTypeFromString(typename, &type);
                            if (lut_count == 0)
                                lut_count = 256;
                            if(childNum == 0) { /* Create delay object based on first child */
                                vx_lut exemplar = NULL;
                                status = vxReserveReferences(context, count+1);
                                exemplar = vxCreateLUT(context, type, lut_count);
                                status |= vxReleaseReferences(context, count+1);
                                status |= vxGetStatus((vx_reference)exemplar);
                                if (status == VX_SUCCESS) {
                                    delay = vxCreateDelay(context, (vx_reference)exemplar, count);
                                    status = vxGetStatus((vx_reference)delay);
                                    vxReleaseLUT(&exemplar);
                                    refs[refIdx] = (vx_reference)delay;
                                    vxSetName(refs[refIdx], cur);
                                    vxInternalizeReference(refs[refIdx]);
                                }
                            }
                            refIdx = xml_prop_ulong(cur, "reference");
                            if (refIdx < total)
                            {
                                if(childNum < count) {
                                    if(((vx_lut_t *)delay->refs[childNum])->num_items == lut_count &&
                                       ((vx_lut_t *)delay->refs[childNum])->item_type == type) {
                                        refs[refIdx] = (vx_reference)delay->refs[childNum];
                                        vxSetName(refs[refIdx], cur);
                                        vxIncrementReference(refs[refIdx], VX_INTERNAL);
                                    } else {
                                        status = VX_ERROR_INVALID_PARAMETERS;
                                        VX_PRINT(VX_ZONE_ERROR, "Delay lut settings doesn't match generated delay lut!\n");
                                        goto exit_error;
                                    }
                                } else {
                                    status = VX_ERROR_INVALID_PARAMETERS;
                                    VX_PRINT(VX_ZONE_ERROR, "Delay has more child nodes than indicated in count!\n");
                                    goto exit_error;
                                }
                            } else {
                                REFNUM_ERROR;
                                goto exit_error;
                            }
                            childNum++;
                            status |= vxLoadDataForLut((vx_lut)refs[refIdx], cur, type, lut_count);
                            break;
                        }
                        case CONVOLUTION_TAG:
                        {
                            vx_size rows = xml_prop_ulong(cur, "rows");
                            vx_size cols = xml_prop_ulong(cur, "columns");
                            vx_uint32 scale = xml_prop_ulong(cur, "scale");
                            if(childNum == 0) { /* Create delay object based on first child */
                                vx_convolution exemplar = NULL;
                                status = vxReserveReferences(context, count+1);
                                exemplar = vxCreateConvolution(context, cols, rows);
                                status |= vxReleaseReferences(context, count+1);
                                status |= vxGetStatus((vx_reference)exemplar);
                                if (status == VX_SUCCESS) {
                                    if(!scale) scale = 1;
                                    status |= vxSetConvolutionAttribute(exemplar, VX_CONVOLUTION_ATTRIBUTE_SCALE,
                                                                        &scale, sizeof(scale));
                                    delay = vxCreateDelay(context, (vx_reference)exemplar, count);
                                    status = vxGetStatus((vx_reference)delay);
                                    vxReleaseConvolution(&exemplar);
                                    refs[refIdx] = (vx_reference)delay;
                                    vxSetName(refs[refIdx], cur);
                                    vxInternalizeReference(refs[refIdx]);
                                }
                            }
                            refIdx = xml_prop_ulong(cur, "reference");
                            if (refIdx < total)
                            {
                                if(childNum < count) {
                                    if(((vx_convolution)delay->refs[childNum])->base.rows == rows &&
                                       ((vx_convolution)delay->refs[childNum])->base.columns == cols) {
                                        refs[refIdx] = (vx_reference)delay->refs[childNum];
                                        vxSetName(refs[refIdx], cur);
                                        vxIncrementReference(refs[refIdx], VX_INTERNAL);
                                    } else {
                                        status = VX_ERROR_INVALID_PARAMETERS;
                                        VX_PRINT(VX_ZONE_ERROR, "Delay convolution settings doesn't match generated delay convolution!\n");
                                        goto exit_error;
                                    }
                                } else {
                                    status = VX_ERROR_INVALID_PARAMETERS;
                                    VX_PRINT(VX_ZONE_ERROR, "Delay has more child nodes than indicated in count!\n");
                                    goto exit_error;
                                }
                            } else {
                                REFNUM_ERROR;
                                goto exit_error;
                            }
                            childNum++;
                            status |= vxLoadDataForConvolution((vx_convolution)refs[refIdx], cur, cols, rows);
                            break;
                        }
                        case REMAP_TAG:
                        {
                            vx_uint32 src_width = xml_prop_ulong(cur, "src_width");
                            vx_uint32 src_height= xml_prop_ulong(cur, "src_height");
                            vx_uint32 dst_width = xml_prop_ulong(cur, "dst_width");
                            vx_uint32 dst_height = xml_prop_ulong(cur, "dst_height");
                            if(childNum == 0) { /* Create delay object based on first child */
                                vx_remap exemplar = NULL;
                                status = vxReserveReferences(context, count+1);
                                exemplar = vxCreateRemap(context, src_width, src_height, dst_width, dst_height);
                                status |= vxReleaseReferences(context, count+1);
                                status |= vxGetStatus((vx_reference)exemplar);
                                if (status == VX_SUCCESS) {
                                    delay = vxCreateDelay(context, (vx_reference)exemplar, count);
                                    status = vxGetStatus((vx_reference)delay);
                                    vxReleaseRemap(&exemplar);
                                    refs[refIdx] = (vx_reference)delay;
                                    vxSetName(refs[refIdx], cur);
                                    vxInternalizeReference(refs[refIdx]);
                                }
                            }
                            refIdx = xml_prop_ulong(cur, "reference");
                            if (refIdx < total)
                            {
                                if(childNum < count) {
                                    if(((vx_remap)delay->refs[childNum])->src_width == src_width &&
                                       ((vx_remap)delay->refs[childNum])->src_height == src_height &&
                                       ((vx_remap)delay->refs[childNum])->dst_width == dst_width &&
                                       ((vx_remap)delay->refs[childNum])->dst_height == dst_height) {
                                        refs[refIdx] = (vx_reference)delay->refs[childNum];
                                        vxSetName(refs[refIdx], cur);
                                        vxIncrementReference(refs[refIdx], VX_INTERNAL);
                                    } else {
                                        status = VX_ERROR_INVALID_PARAMETERS;
                                        VX_PRINT(VX_ZONE_ERROR, "Delay remap settings doesn't match generated delay remap!\n");
                                        goto exit_error;
                                    }
                                } else {
                                    status = VX_ERROR_INVALID_PARAMETERS;
                                    VX_PRINT(VX_ZONE_ERROR, "Delay has more child nodes than indicated in count!\n");
                                    goto exit_error;
                                }
                            } else {
                                REFNUM_ERROR;
                                goto exit_error;
                            }
                            childNum++;
                            status |= vxLoadDataForRemap((vx_remap)refs[refIdx], cur);
                            break;
                        }
                        case DISTRIBUTION_TAG:
                        {
                            vx_size bins = xml_prop_ulong(cur, "bins");
                            vx_size range = xml_prop_ulong(cur, "range");
                            vx_size offset = xml_prop_ulong(cur, "offset");
                            if(childNum == 0) { /* Create delay object based on first child */
                                vx_distribution exemplar = NULL;
                                status = vxReserveReferences(context, count+1);
                                exemplar = vxCreateDistribution(context, bins, offset, range);
                                status |= vxReleaseReferences(context, count+1);
                                status |= vxGetStatus((vx_reference)exemplar);
                                if (status == VX_SUCCESS) {
                                    delay = vxCreateDelay(context, (vx_reference)exemplar, count);
                                    status = vxGetStatus((vx_reference)delay);
                                    vxReleaseDistribution(&exemplar);
                                    refs[refIdx] = (vx_reference)delay;
                                    vxSetName(refs[refIdx], cur);
                                    vxInternalizeReference(refs[refIdx]);
                                }
                            }
                            refIdx = xml_prop_ulong(cur, "reference");
                            if (refIdx < total)
                            {
                                if(childNum < count) {
                                    if(((vx_distribution)delay->refs[childNum])->window_x == (vx_uint32)range/(vx_uint32)bins &&
                                       ((vx_distribution)delay->refs[childNum])->offset_x == offset) {
                                        refs[refIdx] = (vx_reference)delay->refs[childNum];
                                        vxSetName(refs[refIdx], cur);
                                        vxIncrementReference(refs[refIdx], VX_INTERNAL);
                                    } else {
                                        status = VX_ERROR_INVALID_PARAMETERS;
                                        VX_PRINT(VX_ZONE_ERROR, "Delay distribution settings doesn't match generated delay distribution!\n");
                                        goto exit_error;
                                    }
                                } else {
                                    status = VX_ERROR_INVALID_PARAMETERS;
                                    VX_PRINT(VX_ZONE_ERROR, "Delay has more child nodes than indicated in count!\n");
                                    goto exit_error;
                                }
                            } else {
                                REFNUM_ERROR;
                                goto exit_error;
                            }
                            childNum++;
                            status |= vxLoadDataForDistribution((vx_distribution)refs[refIdx], cur, bins);
                            break;
                        }
                        case THRESHOLD_TAG:
                        {
                            vx_char typename[32] = "VX_TYPE_UINT8"; // default value
                            xml_prop_string(cur, "elemType", typename, sizeof(typename));
                            vxTypeFromString(typename, &type);
                            vx_int32 true_value = (vx_int32)xml_prop_ulong(cur, "true_value");
                            vx_int32 false_value = (vx_int32)xml_prop_ulong(cur, "false_value");
                            if(childNum == 0) { /* Create delay object based on first child */
                                vx_threshold exemplar = NULL;
                                status = vxReserveReferences(context, count+1);
                                XML_FOREACH_CHILD_TAG (cur, tag, tags) {
                                    if (tag == BINARY_TAG) {
                                        vx_int32 value = (vx_int32)xml_ulong(cur);
                                        exemplar = vxCreateThreshold(context, VX_THRESHOLD_TYPE_BINARY, type);
                                        status |= vxSetThresholdAttribute(exemplar, VX_THRESHOLD_ATTRIBUTE_THRESHOLD_VALUE, &value, sizeof(value));
                                    } else if (tag == RANGE_TAG) {
                                        vx_int32 upper = (vx_int32)xml_prop_ulong(cur, "upper");
                                        vx_int32 lower = (vx_int32)xml_prop_ulong(cur, "lower");
                                        exemplar = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, type);
                                        status |= vxSetThresholdAttribute(exemplar, VX_THRESHOLD_ATTRIBUTE_THRESHOLD_UPPER, &upper, sizeof(upper));
                                        status |= vxSetThresholdAttribute(exemplar, VX_THRESHOLD_ATTRIBUTE_THRESHOLD_LOWER, &lower, sizeof(lower));
                                    }
                                    status |= vxSetThresholdAttribute(exemplar, VX_THRESHOLD_ATTRIBUTE_TRUE_VALUE , &true_value, sizeof(true_value));
                                    status |= vxSetThresholdAttribute(exemplar, VX_THRESHOLD_ATTRIBUTE_FALSE_VALUE , &false_value, sizeof(false_value));
                                }
                                status |= vxReleaseReferences(context, count+1);
                                status |= vxGetStatus((vx_reference)exemplar);
                                if (status == VX_SUCCESS) {
                                    delay = vxCreateDelay(context, (vx_reference)exemplar, count);
                                    status = vxGetStatus((vx_reference)delay);
                                    vxReleaseThreshold(&exemplar);
                                    refs[refIdx] = (vx_reference)delay;
                                    vxSetName(refs[refIdx], cur);
                                    vxInternalizeReference(refs[refIdx]);
                                }
                            }
                            refIdx = xml_prop_ulong(cur, "reference");
                            if (refIdx < total)
                            {
                                if(childNum < count) {
                                    vx_enum thresh_type = VX_THRESHOLD_TYPE_BINARY;
                                    XML_FOREACH_CHILD_TAG (cur, tag, tags) {
                                        if (tag == BINARY_TAG) {
                                            thresh_type = VX_THRESHOLD_TYPE_BINARY;
                                        } else if (tag == RANGE_TAG) {
                                            thresh_type = VX_THRESHOLD_TYPE_RANGE;
                                        }
                                    }
                                    if(((vx_threshold)delay->refs[childNum])->thresh_type == thresh_type) {
                                        refs[refIdx] = (vx_reference)delay->refs[childNum];
                                        vxSetName(refs[refIdx], cur);
                                        vxIncrementReference(refs[refIdx], VX_INTERNAL);
                                    } else {
                                        status = VX_ERROR_INVALID_PARAMETERS;
                                        VX_PRINT(VX_ZONE_ERROR, "Delay threshold settings doesn't match generated delay threshold!\n");
                                        goto exit_error;
                                    }
                                } else {
                                    status = VX_ERROR_INVALID_PARAMETERS;
                                    VX_PRINT(VX_ZONE_ERROR, "Delay has more child nodes than indicated in count!\n");
                                    goto exit_error;
                                }
                            } else {
                                REFNUM_ERROR;
                                goto exit_error;
                            }
                            childNum++;
                            XML_FOREACH_CHILD_TAG (cur, tag, tags) {
                                if (tag == BINARY_TAG) {
                                    vx_int32 value = (vx_int32)xml_ulong(cur);
                                    status |= vxSetThresholdAttribute((vx_threshold)refs[refIdx], VX_THRESHOLD_ATTRIBUTE_THRESHOLD_VALUE, &value, sizeof(value));
                                } else if (tag == RANGE_TAG) {
                                    vx_int32 upper = (vx_int32)xml_prop_ulong(cur, "upper");
                                    vx_int32 lower = (vx_int32)xml_prop_ulong(cur, "lower");
                                    status |= vxSetThresholdAttribute((vx_threshold)refs[refIdx], VX_THRESHOLD_ATTRIBUTE_THRESHOLD_UPPER, &upper, sizeof(upper));
                                    status |= vxSetThresholdAttribute((vx_threshold)refs[refIdx], VX_THRESHOLD_ATTRIBUTE_THRESHOLD_LOWER, &lower, sizeof(lower));
                                }
                                status |= vxSetThresholdAttribute((vx_threshold)refs[refIdx], VX_THRESHOLD_ATTRIBUTE_TRUE_VALUE , &true_value, sizeof(true_value));
                                status |= vxSetThresholdAttribute((vx_threshold)refs[refIdx], VX_THRESHOLD_ATTRIBUTE_FALSE_VALUE , &false_value, sizeof(false_value));
                            }
                            break;
                        }
                        case SCALAR_TAG:
                        {
                            vx_char typename[20];
                            xml_prop_string(cur, "elemType", typename, sizeof(typename));
                            vxTypeFromString(typename, &type);
                            vx_size nullReference = 0;
                            void *ptr = &nullReference;
                            if(childNum == 0) { /* Create delay object based on first child */
                                vx_scalar exemplar = NULL;
                                status = vxReserveReferences(context, count+1);
                                exemplar = vxCreateScalar(context, type, ptr);
                                status |= vxReleaseReferences(context, count+1);
                                status |= vxGetStatus((vx_reference)exemplar);
                                if (status == VX_SUCCESS) {
                                    delay = vxCreateDelay(context, (vx_reference)exemplar, count);
                                    status = vxGetStatus((vx_reference)delay);
                                    vxReleaseScalar(&exemplar);
                                    refs[refIdx] = (vx_reference)delay;
                                    vxSetName(refs[refIdx], cur);
                                    vxInternalizeReference(refs[refIdx]);
                                }
                            }
                            refIdx = xml_prop_ulong(cur, "reference");
                            if (refIdx < total)
                            {
                                if(childNum < count) {
                                    if(((vx_scalar)delay->refs[childNum])->data_type == type) {
                                        refs[refIdx] = (vx_reference)delay->refs[childNum];
                                        vxSetName(refs[refIdx], cur);
                                        vxIncrementReference(refs[refIdx], VX_INTERNAL);
                                    } else {
                                        status = VX_ERROR_INVALID_PARAMETERS;
                                        VX_PRINT(VX_ZONE_ERROR, "Delay scalar settings doesn't match generated delay scalar!\n");
                                        goto exit_error;
                                    }
                                } else {
                                    status = VX_ERROR_INVALID_PARAMETERS;
                                    VX_PRINT(VX_ZONE_ERROR, "Delay has more child nodes than indicated in count!\n");
                                    goto exit_error;
                                }
                            } else {
                                REFNUM_ERROR;
                                goto exit_error;
                            }
                            childNum++;
                            status |= vxLoadDataForScalar((vx_scalar)refs[refIdx], cur);
                            break;
                        }
                        default:
                            VX_PRINT(VX_ZONE_ERROR, "Unsupported delay object\n");
                            status = VX_ERROR_NOT_SUPPORTED;
                            break;
                    }
                }
            } else {
                REFNUM_ERROR;
                goto exit_error;
            }
        } else {
            VX_PRINT(VX_ZONE_ERROR, "Tag %d unhandled!\n", tag);
            status = VX_ERROR_NOT_IMPLEMENTED;
        }
    }
    /* assign the parameter list */
    XML_FOREACH_CHILD_TAG (cur, tag, tags) {
        if (tag == GRAPH_TAG) {
            vx_uint32 gidx = xml_prop_ulong(cur, "reference");
            XML_FOREACH_CHILD_TAG (cur, tag, tags) {
                if (tag == NODE_TAG) {
                    vx_uint32 nidx = xml_prop_ulong(cur, "reference");
                    XML_FOREACH_CHILD_TAG (cur, tag, tags) {
                        if (tag == PARAMETER_TAG) {
                            vx_uint32 index = xml_prop_ulong(cur, "index");
                            vx_uint32 refIdx = xml_prop_ulong(cur, "reference");

                            status = vxSetParameterByIndex((vx_node)refs[nidx], index, refs[refIdx]);
                            if (status != VX_SUCCESS)
                            {
                                VX_PRINT(VX_ZONE_ERROR, "Failed to set parameter %u on node[%u] in graph[%u] reference=%u\n", index, nidx, gidx, refIdx);
                                goto exit_error;
                            }
                        }
                    }
                }
            }
        }
    }
    /* scan for out of order graph parameters */
    XML_FOREACH_CHILD_TAG (cur, tag, tags) {
        if (tag == GRAPH_TAG) {
            vx_uint32 gidx = xml_prop_ulong(cur, "reference");
            vx_uint32 gpidx = 0u;
            for (gpidx = 0u; gpidx < VX_INT_MAX_PARAMS; gpidx++)
            {
                XML_FOREACH_CHILD_TAG (cur, tag, tags) {
                    if (tag == PARAMETER_TAG) {
                        vx_uint32 index = xml_prop_ulong(cur, "index");
                        if (gpidx == index)
                        {
                            vx_uint32 nidx = xml_prop_ulong(cur, "node");
                            vx_uint32 pidx = xml_prop_ulong(cur, "parameter");
                            status = vxAddParameterToGraphByIndex((vx_graph)refs[gidx],
                                                                  (vx_node)refs[nidx],
                                                                  pidx);
                            if (status != VX_SUCCESS) {
                                VX_PRINT(VX_ZONE_ERROR, "Failed to set graph[%u].parameter[%u] from node[%u].parameter[%u]!\n", gidx,index,nidx,pidx);
                                goto exit_error;
                            }
                        }
                    }
                }
            }
        }
    }

exit_error:
    if (status != VX_SUCCESS)
    {
        /* destroy all the references */
        vxAddLogEntry(&context->base, status, "Failure\n");
        import = (vx_import)vxGetErrorObject(context, status);
    } else {
        vx_uint32 r;
        /* scan the array for valid references */
        for (r = 0u; r < total; r++) {
            if (refs[r]) {
                counted++;
            }
        }
    }
    ((vx_import_t *)import)->count = counted;
    if( status == VX_SUCCESS && counted != total)
    {
        /* The XML file indicated that there was a different number of references than were imported */
        VX_PRINT(VX_ZONE_ERROR, "Reference count mismatch: xml \"references\" tag=%d, imported=%d\n", total, counted);
        vxAddLogEntry(&context->base, VX_ERROR_NOT_COMPATIBLE, "Reference count mismatch: xml \"references\" tag=%d, imported=%d\n", total, counted);
        import = (vx_import)vxGetErrorObject(context, VX_ERROR_NOT_COMPATIBLE);
    }

exit:
    free(user_struct_table);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return import;
}

#endif
