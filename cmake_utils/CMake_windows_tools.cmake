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

# Compiler switches that CANNOT be modified during makefile generation
set (ADD_C_FLAGS         "/Oi -D WINDOWS_ENABLE_CPLUSPLUS /GS")
set (ADD_C_FLAGS_DEBUG   "-D _DEBUG /RTC1 /MTd")  #/MTd /Gm
set (ADD_C_FLAGS_RELEASE "/Zi /Gy -D NDEBUG /MT")# /Ob0") #/GL") #MT

# Compiler switches that CAN be modified during makefile generation and configuration-independent
add_definitions( -DWIN32 -D_WIN32 )

if (BUILD_X64)
    add_definitions(-DARCH_64)
else (BUILD_X64)
    add_definitions(-DARCH_32)
endif (BUILD_X64)

# Linker switches
if (BUILD_X64)
    set (INIT_LINKER_FLAGS        "/MACHINE:X64 /OPT:REF /INCREMENTAL:NO /NXCOMPAT")
else (BUILD_X64)
    set (INIT_LINKER_FLAGS        "/MACHINE:X86 /OPT:REF /INCREMENTAL:NO /NXCOMPAT /SAFESEH")
endif (BUILD_X64)

set (ADD_LINKER_FLAGS_DEBUG "/DEBUG /NODEFAULTLIB:LIBCMT /NODEFAULTLIB:LIBCPMT")
set (ADD_LINKER_FLAGS_RELEASE "/OPT:REF /OPT:ICF /NODEFAULTLIB:LIBCMTD /NODEFAULTLIB:LIBCPMTD")


# setup
enable_language( C )
enable_language( CXX )

set( OPENVX_BUILDING_EXPORT_DEF -DVX_API_ENTRY=__declspec\(dllexport\) )

add_definitions( -D_CRT_SECURE_NO_WARNINGS=1 )

# remove CRT DLL option from default C/C++ switches
string( REPLACE /MDd "" CMAKE_C_FLAGS_DEBUG       ${CMAKE_C_FLAGS_DEBUG} )
string( REPLACE /MD  "" CMAKE_C_FLAGS_RELEASE     ${CMAKE_C_FLAGS_RELEASE} )
string( REPLACE /MDd "" CMAKE_CXX_FLAGS_DEBUG     ${CMAKE_CXX_FLAGS_DEBUG} )
string( REPLACE /MD  "" CMAKE_CXX_FLAGS_RELEASE   ${CMAKE_CXX_FLAGS_RELEASE} )

# remove /INCREMENTAL:YES option from DEBUG Linker switches
string( REPLACE /INCREMENTAL:YES "" CMAKE_EXE_LINKER_FLAGS_DEBUG    ${CMAKE_EXE_LINKER_FLAGS_DEBUG} )
string( REPLACE /INCREMENTAL:YES "" CMAKE_SHARED_LINKER_FLAGS_DEBUG ${CMAKE_SHARED_LINKER_FLAGS_DEBUG} )

# C switches
set( CMAKE_C_FLAGS         "${CMAKE_C_FLAGS}         ${ADD_C_FLAGS}")
set( CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

# C++ switches
set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         ${ADD_C_FLAGS}")
set( CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

# Linker switches - EXE
set( CMAKE_EXE_LINKER_FLAGS           ${INIT_LINKER_FLAGS})
set( CMAKE_EXE_LINKER_FLAGS_DEBUG     "${CMAKE_EXE_LINKER_FLAGS_DEBUG}   ${ADD_LINKER_FLAGS_DEBUG}")
set( CMAKE_EXE_LINKER_FLAGS_RELEASE   "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${ADD_LINKER_FLAGS_RELEASE}")

# Linker switches - DLL
set( CMAKE_SHARED_LINKER_FLAGS          ${INIT_LINKER_FLAGS})
set( CMAKE_SHARED_LINKER_FLAGS_DEBUG   "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}   ${ADD_LINKER_FLAGS_DEBUG}")
set( CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} ${ADD_LINKER_FLAGS_RELEASE}")

