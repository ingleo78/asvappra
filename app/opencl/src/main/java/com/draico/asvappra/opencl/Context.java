package com.draico.asvappra.opencl;

import com.draico.asvappra.opencl.listeners.CallbackContext;

public class Context {
    private int currentContext;
    private Device[] devices;
    public static native Context createContext(Device[] devices, CallbackContext callbackContext);
    public static native Context createContextFromType(Platform platform, CallbackContext callbackContext, long deviceType);
    public native int getReferenceCount();
    public native Device[] getDevices();
    public native void releaseContext();
    public native String toString();
}
