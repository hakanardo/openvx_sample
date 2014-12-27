# Copyright (c) 2012-2014 The Khronos Group Inc.

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and/or associated documentation files (the
# "Materials"), to deal in the Materials without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Materials, and to
# permit persons to whom the Materials are furnished to do so, subject to
# the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Materials.

# THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.

## \file vxWriteNodeHeaders.py
## \author Frank Brill <brill@ti.com>

"""
Simple script that pulls in the node table defined in vxNodes.py, and
uses the code in vxNodeUtils.py to check that it's clean, and write
out a C header file for the nodes defined in the node table.  The name
of the output file is provided on the command line.
"""

import sys

nodeFilename = None
kernelFilename = None

# Get the output file name
if len(sys.argv) > 2:
    nodeFilename = sys.argv[1]
    kernelFilename = sys.argv[2]
else:
    print ("Usage: python %s <vx_nodes.h> <vx_kernels.h>" % sys.argv[0], file=sys.stderr)
    exit()

# Pull in the node table and utility functions
from vxNodes import vxNodeTable
from vxNodeUtils import checkNodeTable, writeNodeHeaderFile, writeKernelHeaderFile

# Try to write the file and inform the user of success or failure
if checkNodeTable(vxNodeTable):
    try: # write the node header file
        writeNodeHeaderFile(vxNodeTable, nodeFilename)
    except:
        print("Error writing file %s...exiting." % nodeFilename, file=sys.stderr)
        raise

    print("Node header file successfully written to", nodeFilename, file=sys.stdout)

    try: # write the kernel header file
        writeKernelHeaderFile(vxNodeTable, kernelFilename)
    except:
        print("Error writing file %s...exiting." % kernelFilename, file=sys.stderr)
        raise

    print("Kernel header file successfully written to", kernelFilename, file=sys.stdout)
