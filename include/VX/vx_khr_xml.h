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

#ifndef _VX_KHR_XML_H_
#define _VX_KHR_XML_H_

/*! \file
 * \brief The OpenVX XML Schema Extension Header.
 *
 * \defgroup group_xml XML API
 * \brief The OpenVX XML Schema Export and Import API.
 */

#define OPENVX_KHR_XML  "vx_khr_xml"

#include <VX/vx.h>

/*! \brief Defines the maximum number of characters in a reference name string.
 * \ingroup group_xml
 * \see vxSetReferenceName
 * \see vxGetReferenceByName
 */
#define VX_MAX_REFERENCE_NAME (64)

/*! \brief Extended reference attribute list.
 *          This extension adds new attributes that can be queried by the
 *          \ref <tt>vxQueryReference</tt> function.
 * \ingroup group_xml
 */
enum vx_ext_import_reference_attribute_e {
    /*! \brief Used to query the reference for its name. Use a <tt>\ref *vx_char</tt> parameter. */
    VX_REF_ATTRIBUTE_NAME = VX_ATTRIBUTE_BASE(VX_ID_KHRONOS, VX_TYPE_REFERENCE) + 0x2,
};

/*! \brief The Object Type Enumeration for Imports.
 * \ingroup group_xml
 */
enum vx_ext_import_type_e {
    VX_TYPE_IMPORT = 0x814,/*!< \brief A <tt>\ref vx_import</tt> */
};

/*! \brief The import type enumeration.
 * \ingroup group_xml
 * \see VX_IMPORT_ATTRIBUTE_TYPE
 */
enum vx_ext_import_types_e {
    VX_IMPORT_TYPE_XML = 0,/*!< \brief The XML import type */
};

/*! \brief The import attributes list
 * \ingroup group_xml
 * \see vxQueryImport
 */
enum vx_import_attribute_e {
    /*! \brief Returns the number of references in the import object. Use a <tt>\ref vx_uint32</tt> parameter.*/
    VX_IMPORT_ATTRIBUTE_COUNT = VX_ATTRIBUTE_BASE(VX_ID_KHRONOS, VX_TYPE_IMPORT) + 0x0,
    /*! \brief Returns the type of import. Use a <tt>\ref vx_ext_import_types_e </tt> parameter */
    VX_IMPORT_ATTRIBUTE_TYPE = VX_ATTRIBUTE_BASE(VX_ID_KHRONOS, VX_TYPE_IMPORT) + 0x1,
};

/*! \brief An abstract handle to an import object.
 * \ingroup group_xml
 * \extends vx_reference
 */
typedef struct _vx_import *vx_import;


#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Exports all objects in the context to an XML file which uses the OpenVX
 * XML Schema.
 * \param [in] context The context to export.
 * \param [in] xmlfile The file name to write the XML into.
 * \note The reference numbers contained in the xml file can appear in any order but
 * should be inclusive from index number 0 to [number of references - 1]. For example,
 * if there are 20 references in the xml file, none of the reference indices should be >= 20.
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \see http://www.khronos.org/openvx/OpenVX.xsd
 * \ingroup group_xml
 */
VX_API_ENTRY vx_status VX_API_CALL vxExportToXML(vx_context context, vx_char xmlfile[]);


/*! \brief Imports all framework and data objects from an XML file into the given context.
 * \param [in] context The context to import into.
 * \param [in] xmlfile The XML file to read.
 * \note The reference indices in the import object corresponds with the reference numbers in the
 * XML file. It is assumed that the program has some means to know which references to use from
 * imported list (either by name: <tt>\ref vxGetReferenceByName</tt>, or by index from looking at the XML
 * file (debug use case): <tt>\ref vxGetReferenceByIndex</tt>).  Alternativly, the program can use
 * <tt>\ref vxGetReferenceByIndex</tt> in a loop and query each one to understand what was imported. After
 * all references of interest have been retrieved, this import obects should be released using
 * <tt>\ref vxReleaseImport</tt>.
 * \return \ref vx_import object containing references to the imported objects in the context
 * \see http://www.khronos.org/openvx/OpenVX.xsd
 * \ingroup group_xml
 */
VX_API_ENTRY vx_import VX_API_CALL vxImportFromXML(vx_context context, vx_char xmlfile[]);

/*! \brief Name a reference
 * \ingroup group_xml
 *
 * This function is used to associate a name to a reference. This name
 * can be used by the OpenVX implementation in log messages and any
 * other reporting mechanisms.  It is also intended to be used by
 * <tt>\ref vxGetReferenceByName</tt> to retrieve a named reference from
 * a \ref vx_import object.
 *
 * The OpenVX implementation will not check if the name is unique in
 * the reference scope (context or graph). Several references can then
 * have the same name.
 *
 * \param [in] ref The reference to name.
 * \param [in] name Pointer to the '\0' terminated string that identifies
 *             the reference.
 *             The string is copied by the function so that it
 *             stays the property of the caller.
 *             NULL means that the reference is not named.
 * \return A \ref vx_status_e enumeration.
 * \retval VX_SUCCESS No errors.
 * \retval VX_ERROR_INVALID_REFERENCE if reference is not valid.
 */
VX_API_ENTRY vx_status VX_API_CALL vxSetReferenceName(vx_reference ref, const vx_char *name);

/*! \brief Used to retrieve a reference by name from the import when the name is known beforehand.  If
 *  multiple references have the same name, then *any* one of them may be returned.
 * \param [in] import The reference to the import object.
 * \param [in] name The reference string name.
 * \return <tt>\ref vx_reference</tt>
 * \retval 0 Invalid import object or name does not match a reference in the import object.
 * \retval * The reference matching the requested name.
 * \note Use <tt>\ref vxReleaseReference</tt> to release the reference before releasing the context.
 * \pre <tt>\ref vxImportFromXML</tt>
 * \ingroup group_xml
 */
VX_API_ENTRY vx_reference VX_API_CALL vxGetReferenceByName(vx_import import, const vx_char *name);

/*! \brief Used to retrieve a reference by the index from the import.
 * \param [in] import The reference to the import object.
 * \param [in] index The index of the reference in the import object to return.
 * \return <tt>\ref vx_reference</tt>
 * \retval 0 Invalid import object or index.
 * \retval * The reference at the requested index number.
 * \note Use <tt>\ref vxQueryImport</tt> with <tt>\ref VX_IMPORT_ATTRIBUTE_COUNT</tt> to retrieve
 * the upper limit of references in the import.
 * \note Use <tt>\ref vxReleaseReference</tt> to release the reference before releasing the context.
 * \pre <tt>\ref vxImportFromXML</tt>
 * \ingroup group_xml
 */
VX_API_ENTRY vx_reference VX_API_CALL vxGetReferenceByIndex(vx_import import, vx_uint32 index);

/*! \brief Used to query the import about its properties.
 * \param [in] import The reference to the import object.
 * \param [in] attribute The <tt>\ref vx_import_attribute_e</tt> value to query for.
 * \param [out] ptr The location at which the resulting value will be stored.
 * \param [in] size The size of the container to which ptr points.
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \pre <tt>\ref vxImportFromXML</tt>
 * \ingroup group_xml
 */
VX_API_ENTRY vx_status VX_API_CALL vxQueryImport(vx_import import, vx_enum attribute, void *ptr, vx_size size);

/*! \brief Releases a reference to an import object.
 * Also internally releases its references to its imported objects. These
 * imported objects may not be garbage collected until their total reference
 * counts are zero.
 * \param [in] import The pointer to the import object to release.
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS No errors.
 * \retval VX_ERROR_INVALID_REFERENCE If import is not a <tt>\ref vx_import</tt>.
 * \note After returning from this function the reference will be zeroed.
 * \pre <tt>\ref vxImportFromXML</tt>
 * \ingroup group_xml
 */
VX_API_ENTRY vx_status VX_API_CALL vxReleaseImport(vx_import *import);

/*! \brief Releases a reference.
 * The object may not be garbage collected until its total reference count is zero.
 * \param [in] ref The pointer to the reference to release.
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS No errors.
 * \retval VX_ERROR_INVALID_REFERENCE If reference is not a <tt>\ref vx_reference</tt>.
 * \note After returning from this function the reference will be zeroed.
 * \pre <tt>\ref vxGetReferenceByName</tt> or <tt>\ref vxGetReferenceByIndex</tt>
 * \ingroup group_xml
 */
VX_API_ENTRY vx_status VX_API_CALL vxReleaseReference(vx_reference *ref);

#ifdef __cplusplus
}
#endif

#endif

