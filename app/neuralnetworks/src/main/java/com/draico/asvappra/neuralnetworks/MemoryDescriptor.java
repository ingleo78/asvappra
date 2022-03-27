package com.draico.asvappra.neuralnetworks;

public class MemoryDescriptor {
    private int currentMemoryDescriptor;
    private boolean isCreated;
    public static native MemoryDescriptor newMemoryDescriptor();
    public native void finish();
    public native void delete();
    public native void addInputRole(Compilation compilation, int indexInput, float frequency);
    public native void addOutputRole(Compilation compilation, int indexOutput, float frequency);
    public native void copy(MemoryDescriptor memoryDescriptorSrc);
    public native void setDimensions(int[] dimensions);
}
