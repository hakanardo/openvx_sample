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


require 'openvx/openvx'

module OpenVX
    VERSION = "1.0.0"
    def self.kernel_names
        khash = {}
        OpenVX.targets.each do |target|
            target.kernels.each do |kernel|
                khash[kernel.name] ||= 0
                khash[kernel.name] += 1
            end
            target = nil
        end
        return khash.keys.sort
    end

    def self.targets
        array = []
        for index in 0..OpenVX.num_targets-1 do
            array << OpenVX::Target.new(index)
        end
        return array
    end

    class Target
        def kernels
            array = []
            kernel_names.each do |name|
                array << OpenVX::Kernel.new(name)
            end
            return array
        end
    end

    def self.CopyImageNode(graph, input, output)
        return nil if graph.class != OpenVX::Graph
        return nil if input.class != OpenVX::Image
        return nil if output.class != OpenVX::Image

        kernel = OpenVX::Kernel.new("org.khronos.copy_image")
        node = OpenVX::Node.new(graph, kernel, [{:ref => input},
                                                {:ref => output}])
        kernel = nil
        return node
    end
end

