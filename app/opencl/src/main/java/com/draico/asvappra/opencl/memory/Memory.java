package com.draico.asvappra.opencl.memory;

import com.draico.asvappra.opencl.CommandQueue;
import com.draico.asvappra.opencl.Context;
import com.draico.asvappra.opencl.listeners.CallbackMemory;
import com.draico.asvappra.opencl.listeners.Event;
import java.util.HashMap;

public class Memory {
    private int currentMemory;
    private Context currentContext;
    public int sizeBlockMemory;
    private int flagsBlockMemory;
    private int flagsMapMemory;
    public HashMap<String, Object> dataMemory;
    public static String byteArrayMemory = "byteArrayMemory";
    public static String intArrayMemory = "intArrayMemory";
    public static String shortArrayMemory = "shortArrayMemory";
    public static String longArrayMemory = "longArrayMemory";
    public static String floatArrayMemory = "floatArrayMemory";
    public static int MEMORY_READ_WRITE = 1 << 0;
    public static int MEMORY_WRITE_ONLY = 1 << 1;
    public static int MEMORY_READ_ONLY = 1 << 2;
    public static int MEMORY_SVM_FINE_GRAIN_BUFFER = 1 << 10;
    public static int MEMORY_SVM_ATOMICS = 1 << 11;
    public static int MAP_READ = 1 << 0;
    public static int MAP_WRITE = 1 << 1;
    public static int MAP_WRITE_INVALIDATE_REGION = 1 << 2;
    public static int MIGRATE_MEMORY_OBJECT_HOST = 1 << 0;
    public static int MIGRATE_MEMORY_OBJECT_CONTENT_UNDEFINED = 1 << 1;
    public static native Memory mallocMemory(Context context, int flagsMemory, int sizeBlock);
    public native void freeMemory();
    public native void clear(CommandQueue commandQueue);
    public native Event enqueueFreeBlocksMemory(CommandQueue commandQueue, Memory[] memory, CallbackMemory callbackMemory);
    public native Event enqueueCopyBlockMemory(CommandQueue commandQueue, Memory srcMemory, boolean isBlockingMemory);
    public native Event enqueueFillBlockMemory(CommandQueue commandQueue, Object patternFill, int sizePattern, int sizeBlockToFill);
    public native Event enqueueMapMemory(CommandQueue commandQueue, boolean isBlockingMap, int sizeBlock, int flagsMap);
    public native Event enqueueUnmapMemory(CommandQueue commandQueue);
    public native Event enqueueMigrateMemory(CommandQueue commandQueue, Memory[] memoryList, int flagsMigrateMemory);
    public native void addByteArray(CommandQueue commandQueue, byte[] data, int offset);
    public native void addIntegerArray(CommandQueue coomandQueue, int[] data, int offset);
    public native void addFloatArray(CommandQueue commandQueue, float[] data, int offset);
    public native void addShortArray(CommandQueue commandQueue, short[] data, int offset);
    public native void addLongArray(CommandQueue commandQueue, long[] data, int offset);
    public native String toString();
}
