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

## \file vxXML2C.py
## \author Frank Brill <brill@ti.com>


# Get the I/O file names from the command line.  Make sure they're
# provided before doing anything else.
import sys
if len(sys.argv) > 2:
    XMLfile = sys.argv[1]
    Cfile = sys.argv[2]
else:
    print ("Usage: python %s <infile.xml> <outfile.c>" % sys.argv[0],
           file=sys.stderr)
    exit()

# Load an XML representation of the graph from the given file.
# Parse the file to create a Python ElementTree.
from xml.etree import ElementTree as etree
try:
    tree = etree.parse(XMLfile)
except:
    print("Graph XML Load", "Cannot load file "+filename, file=sys.stderr)
    exit()

# Convert the elementTree into a vxGraph
from vxGraph import Graph
graph = Graph.fromTree(tree)

# Write the resulting graph to the given output file
from vxCodeWriter import GraphCodeWriter
writer = GraphCodeWriter("context", graph)
writer.writeCfile(Cfile)
