package com.draico.asvappra.neuralnetworks;

public class Device {
    private int currentDevice;
    private boolean isCreated;
    private static boolean isCreatedDeviceList;
    private static int numberDevicesAvailable;
    public static int TYPE_UNKNOWN = 0;
    public static int TYPE_OTHER = 1;
    public static int TYPE_CPU = 2;
    public static int TYPE_GPU = 3;
    public static int TYPE_ACCELERATOR = 4;
    public static native Device[] getDevices();
    public native int getType();
    public native String getName();
    public native String getVersion();
    public native long getNeuralNetworksAPIVersion();
    public native boolean isLiveState();
    public static native int devicesAvailable();
    public native void delete();
}