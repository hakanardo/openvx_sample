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

public class Buffer extends Reference {
    protected Buffer(long c, int unitSize, int numUnits) {
        create(c, unitSize, numUnits);
    }
    private native void create(long c, int us, int nu);
    public native void destroy();
    public native int getUnitSize();
    public native int getNumUnits();
    public native int computeRangeSize(int start, int end);
    public native int getRange(int start, int end, byte[] data);
    public native int setRange(int start, int end, byte[] data);
    public native int getRange(int start, int end, short[] data);
    public native int setRange(int start, int end, short[] data);
    public native int getRange(int start, int end, int[] data);
    public native int setRange(int start, int end, int[] data);
    public native int getRange(int start, int end, long[] data);
    public native int setRange(int start, int end, long[] data);
    public native int getRange(int start, int end, float[] data);
    public native int setRange(int start, int end, float[] data);
    public native int getRange(int start, int end, double[] data);
    public native int setRange(int start, int end, double[] data);
}
