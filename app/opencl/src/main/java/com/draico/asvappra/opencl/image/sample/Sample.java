package com.draico.asvappra.opencl.image.sample;

import com.draico.asvappra.opencl.Context;

public class Sample {
    private Context context;
    private int currentSample;
    private int addressingMode;
    private int filterMode;
    private boolean isNormalizedCoords;
    public static int ADDRESS_NONE = 0x1130;
    public static int ADDRESS_CLAMP_TO_EDGE = 0x1131;
    public static int ADDRESS_CLAMP = 0x1132;
    public static int ADDRESS_REPEAT = 0x1133;
    public static int ADDRESS_MIRRORED_REPEAT = 0x1134;
    public static int FILTER_NEAREST = 0x1140;
    public static int FILTER_LINEAR = 0x1141;
    public static native Sample createSampleWithProperties(Context context, boolean isNormalizedCoords, int addressingMode, int filterMode);
    public native Context getContext();
    public native boolean isNormalizedCoords();
    public native int getAddressingMode();
    public native int getFilterMode();
    public native void release();
    public native String toString();
}
