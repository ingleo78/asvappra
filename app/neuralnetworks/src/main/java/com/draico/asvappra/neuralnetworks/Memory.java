package com.draico.asvappra.neuralnetworks;

import com.draico.asvappra.neuralnetworks.hardware.Buffer;

public class Memory {
    private int currentMemory;
    private int size;
    private int offset;
    private int protectionType;
    private boolean isCreated;
    public static native Memory newMemory(MemoryDescriptor memoryDescriptor);
    public static native Memory newMemoryFronFileDescriptor(int fileDescriptor, int size, int offset, int mode);
    public static native Memory newMemoryFromBuffer(Buffer buffer);
    public native void copy(Memory neuralNetworkMemorySrc);
    public native void delete();
}
