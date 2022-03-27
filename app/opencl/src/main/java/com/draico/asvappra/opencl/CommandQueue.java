package com.draico.asvappra.opencl;

import java.util.HashMap;

public class CommandQueue {
    private int currentCommandQueue;
    private Context currentContext;
    private Device currentDevice;
    private long propertiesSet;
    private boolean isSetDeviceDefault;
    public HashMap<String, Long> properties;
    public long sizeCommandQueue;
    public static String QUEUE_PROPERTIES = "QUEUE_PROPERTIES";
    public static String QUEUE_SIZE = "QUEUE_SIZE";
    public static long QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE = 1 << 0;
    public static long QUEUE_PROFILING_ENABLE = 1 << 1;
    public static long QUEUE_ON_DEVICE = 1 << 2;
    public static long QUEUE_ON_DEVICE_DEFAULT = 1 << 3;
    public static native CommandQueue getCommandQueueWithProperties(Context context, Device device, HashMap<String, Long> properties);
    public native void setDefaultDeviceCommandQueue(Context context, Device device);
    public native void releaseCommandQueue();
    public native void flush();
    public native void finish();
    public native Context getContext();
    public native Device getDevice();
    public native int getReferenceCount();
    public native Device getDeviceDefault();
    public native String toString();
}
