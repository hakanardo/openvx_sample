#
# Copyright (c) 2011-2014 The Khronos Group Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and/or associated documentation files (the
# "Materials"), to deal in the Materials without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Materials, and to
# permit persons to whom the Materials are furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Materials.
#
# THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
#

include_directories( BEFORE
                     ${CMAKE_CURRENT_SOURCE_DIR} 
					 ${CMAKE_SOURCE_DIR}/include )
					 
# set target name
set( TARGET_NAME_XYZ_LIB vx_xyz_lib )
# set target name
set( TARGET_NAME_XYZ xyz )
# set target name
set( TARGET_NAME_EXAMPLE vx_example )
# set target name
set( TARGET_NAME_EXAMPLE_CODE vx_example_code )
if (OPENVX_USE_TILING)
    # set target name
    set( TARGET_NAME_EXAMPLE_TILING openvx-tiling )
    # set target name
    set( TARGET_NAME_TILING_TEST tiling_test )
endif (OPENVX_USE_TILING)

if ((WIN32) OR (CYGWIN))
   add_definitions( ${OPENVX_BUILDING_EXPORT_DEF} )
endif ((WIN32) OR (CYGWIN))

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME_XYZ_LIB}  vx_xyz_lib.c)

target_link_libraries( ${TARGET_NAME_XYZ_LIB} openvx )

if ((WIN32) OR (CYGWIN))
   set( DEF_FILE xyz.def )
endif ((WIN32) OR (CYGWIN))
# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME_XYZ} SHARED vx_xyz_module.c ${DEF_FILE})

if (CYGWIN)
    set_target_properties( ${TARGET_NAME_XYZ} PROPERTIES LINK_FLAGS ${CMAKE_CURRENT_SOURCE_DIR}/${DEF_FILE} )
endif (CYGWIN)

target_link_libraries( ${TARGET_NAME_XYZ} openvx )

# add a target named ${TARGET_NAME}
add_executable (${TARGET_NAME_EXAMPLE} vx_graph_factory.c vx_factory_corners.c vx_factory_pipeline.c vx_factory_edge.c)

target_link_libraries( ${TARGET_NAME_EXAMPLE} vx_xyz_lib openvx-debug-lib openvx-helper openvx )

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME_EXAMPLE_CODE}  vx_imagepatch.c vx_delaygraph.c vx_super_res.c  vx_independent.c vx_matrix_access.c vx_parameters.c vx_kernels.c vx_single_node_graph.c vx_introspection.c vx_multi_node_graph.c vx_convolution.c vx_warps.c vx_callback.c vx_extensions.c)

if (OPENVX_USE_TILING)
    # add a target named ${TARGET_NAME}
    add_library (${TARGET_NAME_EXAMPLE_TILING} SHARED vx_tiling_add.c vx_tiling_alpha.c vx_tiling_gaussian.c vx_tiling_box.c vx_tiling_ext.c)

    target_link_libraries( ${TARGET_NAME_EXAMPLE_TILING} openvx )

    # add a target named ${TARGET_NAME}
    add_executable (${TARGET_NAME_TILING_TEST} vx_tiling_main.c)

    target_link_libraries( ${TARGET_NAME_TILING_TEST} openvx-debug-lib openvx-helper openvx )
endif (OPENVX_USE_TILING)

install ( TARGETS ${TARGET_NAME_XYZ_LIB} ${TARGET_NAME_XYZ} ${TARGET_NAME_EXAMPLE} ${TARGET_NAME_EXAMPLE_CODE} ${TARGET_NAME_EXAMPLE_TILING} ${TARGET_NAME_TILING_TEST}
          RUNTIME DESTINATION bin
          ARCHIVE DESTINATION bin
          LIBRARY DESTINATION bin )
		  
set_target_properties( ${TARGET_NAME_XYZ_LIB} PROPERTIES FOLDER ${EXAMPLES_FOLDER} )
set_target_properties( ${TARGET_NAME_XYZ} PROPERTIES FOLDER ${EXAMPLES_FOLDER} )
set_target_properties( ${TARGET_NAME_EXAMPLE} PROPERTIES FOLDER ${EXAMPLES_FOLDER} )
set_target_properties( ${TARGET_NAME_EXAMPLE_CODE} PROPERTIES FOLDER ${EXAMPLES_FOLDER} )
set_target_properties( ${TARGET_NAME_EXAMPLE_TILING} PROPERTIES FOLDER ${EXAMPLES_FOLDER} )
set_target_properties( ${TARGET_NAME_TILING_TEST} PROPERTIES FOLDER ${EXAMPLES_FOLDER} )