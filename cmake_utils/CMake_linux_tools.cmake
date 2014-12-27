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

# Options common to all projects.
message(STATUS "** ** ** Enable Languages ** ** **")

enable_language( C )
enable_language( CXX )

if (CYGWIN)
    add_definitions( -DCYGWIN )
    set( OPENVX_BUILDING_EXPORT_DEF -DVX_API_ENTRY=__attribute__\(\(dllexport\)\) )
endif (CYGWIN)

if(BUILD_X64)
  set(ARCH_BIT -m64 )
else()
  if (TARGET_CPU STREQUAL "Atom")
    # architecture will be according to ATOM
    set(ARCH_BIT -m32 )
  else ()
    # need to force a more modern architecture than the degault m32 (i386).
    set(ARCH_BIT "-m32 -march=core2" )
  endif (TARGET_CPU STREQUAL "Atom")
endif()

# Compiler switches that CANNOT be modified during makefile generation
if (NOT CYGWIN)
    set (ADD_COMMON_C_FLAGS         "${ARCH_BIT} -fPIC" )
endif (NOT CYGWIN)

set (ADD_C_FLAGS                "${ADD_COMMON_C_FLAGS} -std=gnu99" )
set (ADD_C_FLAGS_DEBUG          "-O0 -ggdb3 -D _DEBUG" )
set (ADD_C_FLAGS_RELEASE        "-O2 -ggdb2 -U _DEBUG")
set (ADD_C_FLAGS_RELWITHDEBINFO "-O2 -ggdb3 -U _DEBUG")

set (ADD_CXX_FLAGS              "${ADD_COMMON_C_FLAGS}" )

# Linker switches

if (NOT CYGWIN)
    set (INIT_LINKER_FLAGS          "-Wl,--enable-new-dtags" ) # --enable-new-dtags sets RUNPATH to the same value as RPATH
endif (NOT CYGWIN)

# embed RPATH and RUNPATH to the binaries that assumes that everything is installed in the same directory
#
# Description:
#   RPATH is used to locate dynamically load shared libraries/objects (DLLs) for the non-standard OS
#   locations without need of relinking DLLs during installation. The algorithm is the following:
#
#   1. If RPATH is present in the EXE/DLL and RUNPATH is NOT present, search through it.
#   2. If LD_LIBRARY_PATH env variable is present, search through it
#   3. If RUNPATH is present in the EXE/DLL, search through it
#   4. Search through locations, configured by system admin and cached in /etc/ld.so.cache
#   5. Search through /lib and /usr/lib
#
#   RUNPATH influences only the immediate dependencies, while RPATH influences the whole subtree of dependencies
#   RPATH is concidered deprecated in favor of RUNPATH, but RUNPATH does not supported by some Linux systems.
#   If RUNPATH is not supported, system loader may report error - remove "-Wl,--enable-new-dtags" above to
#   disable RUNPATH generation.
#
#   If RPATH or RUNPATH contains string $ORIGIN it is substituted by the full path to the containing EXE/DLL.
#   Security issue 1: if EXE/DLL is marked as set-uid or set-gid, $ORIGIN is ignored.
#   Security issue 2: if RPATH/RUNPATH references relative subdirs, intruder may fool it by using softlinks
#
SET(CMAKE_BUILD_WITH_INSTALL_RPATH    TRUE )   # build rpath as if already installed
SET(CMAKE_INSTALL_RPATH               "$ORIGIN::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::" ) # the rpath to use - search through installation dir only
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)   # do not use static link paths as rpath

# C switches
set( CMAKE_C_FLAGS                          "${CMAKE_C_FLAGS}                         ${ADD_C_FLAGS}")
set( CMAKE_C_FLAGS_DEBUG                    "${CMAKE_C_FLAGS_DEBUG}                   ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_C_FLAGS_RELEASE                  "${CMAKE_C_FLAGS_RELEASE}                 ${ADD_C_FLAGS_RELEASE}")
set( CMAKE_C_FLAGS_RELWITHDEBINFO           "${CMAKE_C_FLAGS_RELWITHDEBINFO}          ${ADD_C_FLAGS_RELWITHDEBINFO}")

# C++ switches
set( CMAKE_CXX_FLAGS                        "${CMAKE_CXX_FLAGS}                       ${ADD_CXX_FLAGS}")
set( CMAKE_CXX_FLAGS_DEBUG                  "${CMAKE_CXX_FLAGS_DEBUG}                 ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_CXX_FLAGS_RELEASE                "${CMAKE_CXX_FLAGS_RELEASE}               ${ADD_C_FLAGS_RELEASE}")
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO         "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}        ${ADD_C_FLAGS_RELWITHDEBINFO}")

# Linker switches - EXE
set( CMAKE_EXE_LINKER_FLAGS                 "${INIT_LINKER_FLAGS}")

# Linker switches - DLL
set( CMAKE_SHARED_LINKER_FLAGS              "${INIT_LINKER_FLAGS}                     ${ADD_CMAKE_EXE_LINKER_FLAGS}")

message(STATUS "\n\n** ** ** COMPILER Definitions ** ** **")
message(STATUS "CMAKE_C_COMPILER        = ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_C_FLAGS           = ${CMAKE_C_FLAGS}")
message(STATUS "")
message(STATUS "CMAKE_CXX_COMPILER      = ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_FLAGS         = ${CMAKE_CXX_FLAGS}")
message(STATUS "")
message(STATUS "CMAKE_EXE_LINKER_FLAGS  = ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "")
message(STATUS "CMAKE_BUILD_TOOL        = ${CMAKE_BUILD_TOOL}")

