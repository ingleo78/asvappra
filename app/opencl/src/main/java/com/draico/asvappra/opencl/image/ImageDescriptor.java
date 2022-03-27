package com.draico.asvappra.opencl.image;

import com.draico.asvappra.opencl.memory.buffer.Buffer;

public class ImageDescriptor {
    private int currentImageDescriptor;
    private ImageFormat imageFormat;
    private ImageDescriptor imageDescriptor;
    private Buffer buffer;
    private int imageType;
    private int imageWidth;
    private int imageHeight;
    private int imageDepth;
    private int numberBytesPerRow;
    private int numberBytesPerLayer;
    private int numberImages;
    public static int TYPE_IMAGE2D = 0x10F1;
    public static int TYPE_IMAGE3D = 0x10F2;
    public static int TYPE_IMAGE2D_ARRAY = 0x10F3;
    public static int TYPE_IMAGE1D = 0x10F4;
    public static int TYPE_IMAGE1D_ARRAY = 0x10F5;
    public static int TYPE_IMAGE1D_BUFFER = 0x10F6;
    public static native ImageDescriptor createImageDescriptor(ImageFormat imageFormat, Buffer buffer, int imageType, int width, int height, int depth,
                                                               int numberImages);
    public native String toString();
}
