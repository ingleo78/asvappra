package com.draico.asvappra.opencl.image;

import com.draico.asvappra.opencl.Context;

public class ImageFormat {
    private Context context;
    private int currentImageFormat;
    public int imageChannelOrder;
    public int imageChannelDataType;
    public static int R = 0x10B0;
    public static int A = 0x10B1;
    public static int RG = 0x10B2;
    public static int RA = 0x10B3;
    public static int RGB = 0x10B4;
    public static int RGBA = 0x10B5;
    public static int BGRA = 0x10B6;
    public static int ARGB = 0x10B7;
    public static int INTENSITY = 0x10B8;
    public static int LUMINANCE = 0x10B9;
    public static int Rx = 0x10BA;
    public static int RGx = 0x10BB;
    public static int RGBx = 0x10BC;
    public static int DEPTH = 0x10BD;
    public static int DEPTH_STENCIL = 0x10BE;
    public static int sRGB = 0x10BF;
    public static int sRGBx = 0x10C0;
    public static int sRGBA = 0x10C1;
    public static int sBGRA = 0x10C2;
    public static int ABGR = 0x10C3;
    public static int SNORM_INT8 = 0x10D0;
    public static int SNORM_INT16 = 0x10D1;
    public static int UNORM_INT8 = 0x10D2;
    public static int UNORM_INT16 = 0x10D3;
    public static int UNORM_SHORT565 = 0x10D4;
    public static int UNORM_SHORT555 = 0x10D5;
    public static int UNORM_INT_101010 = 0x10D6;
    public static int SIGNED_INT8 = 0x10D7;
    public static int SIGNED_INT16 = 0x10D8;
    public static int SIGNED_INT32 = 0x10D9;
    public static int UNSIGNED_INT8 = 0x10DA;
    public static int UNSIGNED_INT16 = 0x10DB;
    public static int UNSIGNED_INT32 = 0x10DC;
    public static int HALF_FLOAT = 0x10DD;
    public static int FLOAT = 0x10DE;
    public static int UNORM_INT_101010_2 = 0x10E0;
    public static native ImageFormat createImageFormat(Context context, int imageChannelOrder, int imageChannelDataType);
    public static native ImageFormat[] getSupportedImageFormats(Context context, int typeAccessImage, int imageType);
    public native String toString();
}
