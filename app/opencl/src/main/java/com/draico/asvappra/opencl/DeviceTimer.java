package com.draico.asvappra.opencl;

public class DeviceTimer {
    private Device device;
    public static native DeviceTimer getDeviceTimer(Device device);
    public native long[] getDeviceAndHostTimestamp();
    public native long getHostTimestamp();
    public native String toString();
}
