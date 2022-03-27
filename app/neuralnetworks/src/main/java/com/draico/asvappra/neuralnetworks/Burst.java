package com.draico.asvappra.neuralnetworks;

public class Burst {
    private int currentBurst;
    private boolean isCreated;
    public static native Burst newBurst(Compilation compilation);
    public native void delete();
}
