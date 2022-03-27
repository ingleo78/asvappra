package com.draico.asvappra.opencl;

public class Platform {
    private int currentPlatform;
    public static native Platform[] getPlatforms();
    public native String getProfile();
    public native String getName();
    public native String getVendor();
    public native String getVersion();
    public native String getExtensions();
    public native long getHostTimerResolution();
    public native String toString();
    static {
        System.loadLibrary("OpenCL");
    }
}
