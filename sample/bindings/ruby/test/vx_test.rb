#!/usr/bin/ruby -w

# Copyright (c) 2012-2014 The Khronos Group Inc.
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

require 'openvx'

def openvx_simple_demo
    graph = OpenVX::Graph.new
    input = OpenVX::Image.new(640, 480, OpenVX::Image::VX_DF_IMAGE_U8)
    output = OpenVX::Image.new(640, 480, OpenVX::Image::VX_DF_IMAGE_U8)
    node = OpenVX::CopyImageNode(graph, input, output)
    graph.verify
    graph.process
end

def openvx_debugging
    puts "OpenVX is a #{OpenVX.class}"
    OpenVX.constants.each {|c|
        puts "\tclass #{c}"
    }
    OpenVX::load("xyz")
    puts "There are #{OpenVX.num_kernels} kernels in OpenVX"
    puts "There are #{OpenVX.num_targets} targets in OpenVX"
    puts "There are #{OpenVX.num_modules} targets in OpenVX"
    kernel = OpenVX::Kernel.new("org.khronos.copy_image")
    puts "Copy image has #{kernel.params} parameters"
    graph = OpenVX::Graph.new
    puts "There are #{graph.nodes} nodes in the graph"
    images = [] # new array
    puts "VX_DF_IMAGE_Y800 = #{OpenVX::Image::VX_DF_IMAGE_Y800}"
    # Image from named hash
    images << OpenVX::Image.new({:width => 640,
                                 :height => 480,
                                 :format => OpenVX::Image::VX_DF_IMAGE_Y800})
    # explicit typed parameters
    images << OpenVX::Image.new(640, 480, OpenVX::Image::VX_DF_IMAGE_Y800)
    # "Virtual"
    images << OpenVX::Image.new
    # node construction takes an array
    node = OpenVX::CopyImageNode(graph, images[0], images[1])
    graph.verify
    graph.process
    node = nil
    images.map! {|i| i=nil }
    graph = nil
end

if __FILE__ == $0
    if (ARGV[0] == '-s')
        openvx_simple_demo()
    else
        openvx_debugging()
    end
end
