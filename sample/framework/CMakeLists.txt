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
set( TARGET_NAME openvx )

include_directories( BEFORE
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}/../include
					 ${CMAKE_SOURCE_DIR}/include 
					 ${CMAKE_SOURCE_DIR}/debug 
					 ${OPENCL_INCLUDE_PATH} )
					 
if ((WIN32) OR (CYGWIN))
   add_definitions( ${OPENVX_BUILDING_EXPORT_DEF} )
endif ((WIN32) OR (CYGWIN))
					 
FIND_SOURCES()

if ((WIN32) OR (CYGWIN))
    set( DEF_FILE openvx.def )
endif ((WIN32) OR (CYGWIN))

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${DEF_FILE})

set( LINK_ADDITIONAL )

if (NOT WIN32)
    list( APPEND LINK_ADDITIONAL dl m )
	if (ANDROID)
	    list( APPEND LINK_ADDITIONAL log )
	else (ANDROID)
	    list( APPEND LINK_ADDITIONAL pthread rt )
	endif (ANDROID)
endif (NOT WIN32)

if (EXPERIMENTAL_USE_OPENCL)
    if (OPENCL_LIB_PATH)
	    find_library(LIB_OPENCL OpenCL HINTS ${OPENCL_LIB_PATH})
		if (NOT LIB_OPENCL)
		    message(FATAL_ERROR "Could not find OpenCL library")
		endif (NOT LIB_OPENCL)
		list( APPEND LINK_ADDITIONAL ${LIB_OPENCL} )
	endif (OPENCL_LIB_PATH)
endif (EXPERIMENTAL_USE_OPENCL)

if (CYGWIN)
    set_target_properties( ${TARGET_NAME} PROPERTIES LINK_FLAGS ${CMAKE_CURRENT_SOURCE_DIR}/${DEF_FILE} )
endif (CYGWIN)

target_link_libraries( ${TARGET_NAME} openvx-helper vx_debug ${LINK_ADDITIONAL} )

install ( TARGETS ${TARGET_NAME} 
          RUNTIME DESTINATION bin
          ARCHIVE DESTINATION lib
          LIBRARY DESTINATION bin )
		  
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${SAMPLE_FOLDER} )
