package com.draico.asvappra.neuralnetworks.hardware;

public class BufferDescription {
    public static int FORMAT_R8G8B8A8_UNORM = 1;
    public static int FORMAT_R8G8B8X8_UNORM = 2;
    public static int FORMAT_R8G8B8_UNORM = 3;
    public static int FORMAT_R5G6B5_UNORM = 4;
    public static int FORMAT_R16G16B16A16_FLOAT = 0x16;
    public static int FORMAT_R10G10B10A2_UNORM = 0x2B;
    public static int FORMAT_BLOB = 0x21;
    public static int FORMAT_D16_UNORM = 0x30;
    public static int FORMAT_D24_UNORM = 0x31;
    public static int FORMAT_D24_UNORM_S8_UINT = 0x32;
    public static int FORMAT_D32_FLOAT = 0x33;
    public static int FORMAT_D32_FLOAT_S8_UINT = 0x34;
    public static int FORMAT_S8_UINT = 0x35;
    public static int FORMAT_Y8Cb8Cr8_420 = 0x23;
    private int format;
    private int height;
    private int width;
    private int layers;
    private int rfu0;
    private int rfu1;
    private int stride;
    private long usage;
    public BufferDescription(int format, int height, int width, int layers, int rfu0, int rfu1, int stride, long usage) {
        this.format = format;
        this.height = height;
        this.width = width;
        this.layers = layers;
        this.rfu0 = rfu0;
        this.rfu1 = rfu1;
        this.stride = stride;
        this.usage = usage;
    }
}
