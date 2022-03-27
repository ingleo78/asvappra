package com.draico.asvappra.opencl.image;

import com.draico.asvappra.opencl.CommandQueue;
import com.draico.asvappra.opencl.Context;
import com.draico.asvappra.opencl.listeners.Event;
import com.draico.asvappra.opencl.memory.buffer.Buffer;

public class Image {
    private Context context;
    private ImageFormat imageFormat;
    private ImageDescriptor imageDescriptor;
    private Buffer buffer;
    private byte[] dataImage;
    private int currentImage;
    private int flags;
    private int flagMap;
    private int imageType;
    public static int IMAGE_READ_WRITE = 1 << 0;
    public static int IMAGE_WRITE_ONLY = 1 << 1;
    public static int IMAGE_READ_ONLY = 1 << 2;
    public static int IMAGE_USE_HOST_PTR = 1 << 3;
    public static int IMAGE_COPY_HOST_PTR = 1 << 5;
    public static int MAP_READ = 1 << 0;
    public static int MAP_WRITE = 1 << 1;
    public static int MAP_WRITE_INVALIDATE_REGION = 1 << 2;
    public static int MIGRATE_IMAGE_HOST = 1 << 0;
    public static int MIGRATE_IMAGE_CONTENT_UNDEFINED = 1 << 1;
    public static native Image createImage(Context context, ImageFormat imageFormat, ImageDescriptor imageDescriptor, byte[] dataImage, int flags);
    public native Event readImage(CommandQueue commandQueue, Buffer buffer, boolean isBlockingRead, int[] origin, int[] region, int numberBytesPerRow,
                                  int numberBytesByLayer);
    public native Event writeImage(CommandQueue commandQueue, Buffer buffer, boolean isBlockingWrite, int[] origin, int[] region, int numberBytesPerRow,
                                   int numberBytesByLayer);
    public native Event copyImage(CommandQueue commandQueue, Image imageSrc, int[] srcOrigin, int[] dstOrigin, int[] region);
    public native Event fillImage(CommandQueue commandQueue, Object fillColor, int[] origin, int[] region);
    public native Event copyImageToBuffer(CommandQueue commandQueue, Buffer buffer, int[] origin, int[] region);
    public native Event copyBufferToImage(CommandQueue commandQueue, Buffer buffer, int[] origin, int[] region);
    public native Event mapImage(CommandQueue commandQueue, int[] origin, int[] region, int[] numberBytesPerRow,
                                 int[] numberBytesByLayer, boolean isBlockingMap, int flagMap);
    public native Event unmapImage(CommandQueue commandQueue);
    public native Event migrateImage(CommandQueue commandQueue, Image[] images, int flagMigrate);
    ;   public native void release();
    public native ImageFormat getImageFormat();
    public native int getSizePixel();
    public native int getNumberBytesPerRow();
    public native int getNumberBytesPerLayer();
    public native int getWidth();
    public native int getHeight();
    public native int getDepth();
    public native int getNumberImages();
    public native String toString();
}
