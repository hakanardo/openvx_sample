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

# set target name
set( TARGET_NAME vx_test )

include_directories( BEFORE
                     ${CMAKE_CURRENT_SOURCE_DIR}
					 ${CMAKE_SOURCE_DIR}/include )
					 
# add a target named ${TARGET_NAME}
add_executable (${TARGET_NAME}  vx_test.c)

target_link_libraries( ${TARGET_NAME} vx_xyz_lib openvx-debug-lib openvx-extras-lib openvx-helper openvx vxu )

install ( TARGETS ${TARGET_NAME} 
          RUNTIME DESTINATION bin
          ARCHIVE DESTINATION lib
          LIBRARY DESTINATION bin )
		  
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${SAMPLE_FOLDER} )
		  
if ((UNIX) AND (OPENVX_USE_SDL))
    
	set( TARGET_NAME_CAM_TEST vx_cam_test )

	# add a target named ${TARGET_NAME}
	add_executable (${TARGET_NAME_CAM_TEST}  vx_cam_test.c)

	target_link_libraries( ${TARGET_NAME_CAM_TEST} openvx-debug-lib openvx-extras-lib openvx-helper openvx vxu )

	install ( TARGETS ${TARGET_NAME_CAM_TEST} 
			  RUNTIME DESTINATION bin
			  ARCHIVE DESTINATION lib
			  LIBRARY DESTINATION bin )
			  
	set_target_properties( ${TARGET_NAME_CAM_TEST} PROPERTIES FOLDER ${SAMPLE_FOLDER} )

endif ((UNIX) AND (OPENVX_USE_SDL))