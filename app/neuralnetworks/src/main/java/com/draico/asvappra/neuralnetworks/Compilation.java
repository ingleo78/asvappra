package com.draico.asvappra.neuralnetworks;

import java.io.File;

public class Compilation {
    private int currentCompilation;
    private boolean isCreated;
    public static int PRIORITY_LOW = 90;
    public static int PRIORITY_MEDIUM = 100;
    public static int PRIORITY_HIGH = 110;
    public static int PRIORITY_DEFAULT = 100;
    public static int PREFER_LOW_POWER = 0;
    public static int PREFER_FAST_SINGLE_ANSWER = 1;
    public static int PREFER_SUSTAINED_SPEED = 2;
    public static int SIZE_OF_CACHE_TOKEN = 32;
    public static native Compilation newCompilation(Model model);
    public static native Compilation newCompilationForDevices(Model model, Device[] devices);
    public native void finish();
    public native void delete();
    public native void setCaching(File cacheDir, byte[] token);
    public native void setPreference(int preference);
    public native void setPriority(int priority);
    public native void setTimeout(long duration);
}
