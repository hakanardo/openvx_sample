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

import os
import sys
import subprocess
import shutil
from optparse import OptionParser

gProjName = "OpenVX"

class os_enum(object):
    Linux = 1
    Win = 2
    Android = 3

    @staticmethod
    def toString(eVal):
        if (eVal == os_enum.Linux):
            return "Linux"
        elif (eVal == os_enum.Win):
            return "Win"
        elif (eVal == os_enum.Android):
            return "Android"
        return ""

class arch_enum(object):
    x64 = 1
    x32 = 2

    @staticmethod
    def toString(eVal):
        if (eVal == arch_enum.x64):
            return "x64"
        elif (eVal == arch_enum.x32):
            return "x32"
        return ""

class configuration_enum(object):
    Release = 1
    Debug = 2

    @staticmethod
    def toString(eVal):
        if (eVal == configuration_enum.Release):
            return "Release"
        elif (eVal == configuration_enum.Debug):
            return "Debug"
        return ""

def main():
    parser = OptionParser(usage ='usage: %prog [options]', description = "Generate build make / sln files")
    parser.add_option("--os", dest="os", help="Set the operating system (Linux [For Linux or Cygwin]/ Windows / Android)", default='')
    parser.add_option("--arch", dest="arch", help="Set the architecture (32 / 64 bit) [Default 64]", default='64')
    parser.add_option("--conf", dest="conf", help="Set the configuration (Release / Debug) [Default Release]", default='Release')
    parser.add_option("--c", dest="c_compiler", help="Set the C compiler [Default: the default in system (Optional)]", default='')
    parser.add_option("--cpp", dest="cpp_compiler", help="Set the CPP compiler (Default: the default in system (Optional)]", default='')
    parser.add_option("--gen", dest="generator", help="Set CMake generator [Default: [WIN]:\"Visual Studio 12\" [Linux]:Cmake default gernerator]", default='')
    parser.add_option("--env", dest="env_vars", help="Print supported environment variable [Default False]", default='False')
    parser.add_option("--out", dest="out_path", help="Set the path to generated build / install files [Default root directory]", default='')
    parser.add_option("--build", dest="build", help="Build and install the targets (change to non 'true' value in order to generate solution / make files only) [Default True]", default='True')
    parser.add_option("--rebuild", dest="rebuild", help="Rebuild the targets (Use it when you add source file) [Default False]", default='False')
    parser.add_option("--openmp", dest="openmp", help="Build also OpenMP target [Default False]", default='False')
    parser.add_option("--opencl", dest="opencl", help="Build also OpenCL target [Default False]", default='False')

    (options, args) = parser.parse_args()
    if (options.env_vars <> "False"):
        print "VX_OPENCL_INCLUDE_PATH - OpenCL include files location"
        print "VX_OPENCL_LIB_PATH - OpenCL libraries location"
        print "ANDROID_NDK_TOOLCHAIN_ROOT - ANDROID toolchain installation path"
        return
    operatingSys = os_enum.Linux
    if (options.os.lower() == "windows"):
        operatingSys = os_enum.Win
    elif (options.os.lower() == "android"):
        operatingSys = os_enum.Android
    elif (options.os.lower() <> "linux"):
        sys.exit("Error: Please define the OS (Linux / Windows / Android)")

    cmake_gernerator = options.generator
    if (operatingSys == os_enum.Win) and (len(cmake_gernerator) == 0):
        cmake_gernerator = "Visual Studio 12"

    arch = arch_enum.x64
    if (options.arch == "32"):
        arch = arch_enum.x32

    conf = configuration_enum.Release
    if (options.conf.lower() == "debug"):
        conf = configuration_enum.Debug

    userDir = os.getcwd()
    cmakeDir = os.path.dirname(os.path.abspath(__file__))
    outputDir = cmakeDir
    if (options.out_path <> ''):
        outputDir = options.out_path
        if (not os.path.isdir(outputDir)):
            sys.exit("Error: " + outputDir + " is not directory")

    # chdir to buildRootDir
    os.chdir(outputDir)

    # create build directory
    if (not os.path.isdir("build")):
        os.makedirs("build")
    os.chdir("build")
    if (not os.path.isdir(os_enum.toString(operatingSys))):
        os.makedirs(os_enum.toString(operatingSys))
    os.chdir(os_enum.toString(operatingSys))
    if (not os.path.isdir(arch_enum.toString(arch))):
        os.makedirs(arch_enum.toString(arch))
    os.chdir(arch_enum.toString(arch))
    if (operatingSys <> os_enum.Win):
        if not os.path.isdir(configuration_enum.toString(conf)):
            os.makedirs(configuration_enum.toString(conf))
        os.chdir(configuration_enum.toString(conf))

    # Delete the old build directory in case of rebuild
    if ((options.rebuild.lower() <> "false") and (len(os.listdir(os.getcwd())) > 0)):
        rmDir = os.getcwd()
        (head, tail) = os.path.split(os.getcwd())
        os.chdir(head)
        shutil.rmtree(rmDir)
        os.makedirs(tail)
        os.chdir(tail)

    installDir = os.path.join(outputDir, "install", os_enum.toString(operatingSys), arch_enum.toString(arch))
    if (operatingSys == os_enum.Win):
        # Add \\${BUILD_TYPE} in order to support "Debug" \ "Release" build in visual studio
        installDir = installDir + '\\${BUILD_TYPE}'
    else:
        installDir = os.path.join(installDir, configuration_enum.toString(conf))
    cmake_generator_command = ""
    # if the user set generator
    if len(cmake_gernerator) > 0:
        cmake_generator_command = "-G \"" + cmake_gernerator
        if (operatingSys == os_enum.Win) and (arch == arch_enum.x64) and (False == cmake_gernerator.endswith("Win64")):
            cmake_generator_command = cmake_generator_command + " Win64"
        cmake_generator_command = cmake_generator_command + "\""
    cmd = "cmake "
    cmd += cmakeDir + " "
    cmd += "-DCMAKE_BUILD_TYPE=" + configuration_enum.toString(conf) + " "
    cmd += "-DCMAKE_INSTALL_PREFIX=" + installDir + " "
    if (operatingSys == os_enum.Android):
        cmd += "-DANDROID=1 "
    if (arch == arch_enum.x64):
        cmd += "-DBUILD_X64=1 "
    cmd += cmake_generator_command + " "
    if (options.c_compiler <> ''):
        cmd += "-DCMAKE_C_COMPILER=" + options.c_compiler + " "
    if (options.cpp_compiler <> ''):
        cmd += "-DCMAKE_CXX_COMPILER=" + options.c_compiler + " "
    if (options.openmp.lower() <> "false"):
        cmd += "-DEXPERIMENTAL_USE_OPENMP=1 "
    if (options.opencl.lower() <> "false"):
        cmd += "-DEXPERIMENTAL_USE_OPENCL=1 "
    print cmd
    subprocess.Popen(cmd, shell=True).wait()

    if (options.build.lower() == "true"):
        if (operatingSys == os_enum.Win):
            print "DEVENV " + gProjName + ".sln /build " + configuration_enum.toString(conf) + " /project INSTALL"
            subprocess.Popen("DEVENV " + gProjName + ".sln /build " + configuration_enum.toString(conf) + " /project INSTALL", shell=True).wait()
        else:
            subprocess.Popen("make install -j", shell=True).wait()

    # chdir back to user directory
    os.chdir(userDir)

main()
