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

public class Scalar extends Reference {
    protected Scalar(long c, byte v)   { create(c, v); }
    protected Scalar(long c, short v)  { create(c, v); }
    protected Scalar(long c, int v)    { create(c, v); }
    protected Scalar(long c, long v)   { create(c, v); }
    protected Scalar(long c, float v)  { create(c, v); }
    protected Scalar(long c, double v) { create(c, v); }

    private native void create(long c, byte v);
    private native void create(long c, short v);
    private native void create(long c, int v);
    private native void create(long c, long v);
    private native void create(long c, float v);
    private native void create(long c, double v);

    public native void destroy();

    public native void setValue(byte value);
    public native void setValue(short value);
    public native void setValue(int value);
    public native void setValue(long value);
    public native void setValue(float value);
    public native void setValue(double value);

    public native byte getByteValue();
    public native short getShortValue();
    public native int getIntValue();
    public native long getLongValue();
    public native float getFloatValue();
    public native double getDoubleValue();
}
