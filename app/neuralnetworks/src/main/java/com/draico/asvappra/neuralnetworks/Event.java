package com.draico.asvappra.neuralnetworks;

public class Event {
    private int currentEvent;
    private boolean isCreated;
    public static native Event newEvent(int sync_fence_fd);
    public native int getSyncFenceFD();
    public native void waitExecutionComplete();
    public native void delete();
}
