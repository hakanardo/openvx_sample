/*
 * Copyright (c) 2012-2014 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */

package org.khronos.openvx;

public class Node extends Reference {
    protected Node(long g, long k) { create(g, k); }
    private native void create(long g, long k);
    public native void destroy();
    //public native Perf getPerf();
    //public native int getStatus();
    public native Parameter getParameter(int index);
    public int setParameter(int index, int direction, Scalar v) {
        return _setParameter(index, direction, v.getType(), v.handle);
    }
    public int setParameter(int index, int direction, Buffer v){
        return _setParameter(index, direction, v.getType(), v.handle);
    }
    public int setParameter(int index, int direction, Image v){
        return _setParameter(index, direction, v.getType(), v.handle);
    }
    private native int _setParameter(int index, int direction, int type, long r);
}
