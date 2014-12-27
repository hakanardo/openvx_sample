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

## \file vxCodeWriter.py
## \author Frank Brill <brill@ti.com>


from vxGraph import *

class GraphCodeWriter:
    """
    Class to write the C-code for a given graph to a given filename.
    Indents the code appropriately using its own printLine function.
    """

    def __init__(self, context, graph):
        "Initialized the context name and graph, no indentation."
        self.context = context
        self.graph = graph
        self.printLineIndent = 0

    def indent(self, amount):
        """Increase the level of indentation.  Use a negative amount to
        dedent."""
        self.printLineIndent = self.printLineIndent+amount

    def printLine(self, *args, sep='', end='\n'):
        """Print a given line tot he current output file with the
        correct level of indentation."""
        for i in range(0, self.printLineIndent):
            print("  ", end='', file=self.outfile)
        print(*args, sep=sep, end=end, file=self.outfile)

    def printNodes(self, nodes, header, printed):
        """Print the C constructors for a list of nodes with the given
        header.  Make note of the fact that each node has been
        constructed."""
        if len(nodes) > 0:
            self.printLine()
            self.printLine("// %s" % header)
        for node in nodes:
            node.printCConstructor(self)
            printed[node] = True

    def sortNodeList(self, nodes, printed):
        """Get a list of nodes and return a sorted version of it such
        that every node in the returned list comes *after* its
        predecessors in the graph that contains them."""
        returnList = []
        while len(nodes) > 0:
            to_idx = 0
            for node in nodes:
                if not node.isReady(printed):
                    nodes[to_idx] = node
                    to_idx += 1
                else:
                    returnList.append(node)
                    printed[node] = True
            del nodes[to_idx:]
        return returnList

    def writeCfile(self, filename):
        """Write the C code to build the given OpenVX graph to the
        given file."""

        # open the output file
        self.graphName, fileext = os.path.splitext(filename)
        try:
            self.outfile = open(filename, "w")
        except:
            print("Failed to open", filename, file=sys.stderr)
            raise

        # print the prologue
        self.printLine('{')
        self.indent(1)
        self.printLine("// The Context")
        self.printLine("vx_context %s = vxCreateContext();" %
                       self.context)
        self.printLine()
        self.printLine("// The Graph")
        self.printLine("vx_graph %s = vxCreateGraph(%s);" %
                       (self.graphName, self.context))

        # separate out the various node types
        printed = dict()
        heads = []
        tails = []
        intermediates = []
        functions = []
        for node in self.graph.nodes.values():
            if isinstance(node, DataNode):
                if len(node.inports[0].links) == 0: heads.append(node)
                elif len(node.outports[0].links) == 0: tails.append(node)
                else: intermediates.append(node)
            else: functions.append(node)

        # print the input data nodes
        self.printNodes(heads, "Inputs", printed)

        # print the intermediate data and links
        self.printNodes(intermediates, "Intermediate data", printed)
        if len(self.graph.links) > 0:
            self.printLine()
            self.printLine("// Links")
        for link in self.graph.links.values():
            link.printCConstructor(self)

        # print the output data nodes
        self.printNodes(tails, "Results", printed)

        # print the functions nodes
        functions = self.sortNodeList(functions, printed)
        self.printNodes(functions, "Functions", printed)

        # print the epilogue
        self.indent(-1)
        self.printLine('}')
