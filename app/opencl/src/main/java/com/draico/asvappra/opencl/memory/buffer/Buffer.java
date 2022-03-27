package com.draico.asvappra.opencl.memory.buffer;

import com.draico.asvappra.opencl.CommandQueue;
import com.draico.asvappra.opencl.Context;
import com.draico.asvappra.opencl.listeners.CallbackBuffer;
import com.draico.asvappra.opencl.listeners.Event;
import com.draico.asvappra.opencl.memory.Memory;

public class Buffer {
    private int currentBuffer;
    private int currentBufferRegion;
    private long flagMapBuffer;
    private boolean isSubBuffer;
    private String typeData;
    private Context currentContext;
    public Memory bufferData;
    public int sizeBlockMemorySubBuffer;
    public static int BUFFER_READ_WRITE = 1 << 0;
    public static int BUFFER_WRITE_ONLY = 1 << 1;
    public static int BUFFER_READ_ONLY = 1 << 2;
    public static int BUFFER_USE_HOST_PTR = 1 << 3;
    public static int BUFFER_COPY_HOST_PTR = 1 << 5;
    public static int BUFFER_HOST_WRITE_ONLY = 1 << 7;
    public static int BUFFER_HOST_READ_ONLY = 1 << 8;
    public static int BUFFER_HOST_NO_ACCESS = 1 << 9;
    public static int MAP_BUFFER_READ = 1 << 0;
    public static int MAP_BUFFER_WRITE = 1 << 1;
    public static int MAP_WRITE_INVALIDATE_REGION = 1 << 2;
    public static int MIGRATE_BUFFER_OBJECT_HOST = 1 << 0;
    public static int MIGRATE_BUFFER_OBJECT_CONTENT_UNDEFINED = 1 << 1;
    public native static Buffer createBuffer(Context context, Memory memory, int flagsBuffer);
    public native static Buffer createBufferWithPrimitiveArray(Context context, Object data, int flagsBuffer);
    public native Buffer createSubBuffer(int flagsBuffer, int sizeBlockMemory);
    public native Event readBuffer(CommandQueue commandQueue, boolean isblockingRead, int offsetRead, int sizeBlock);
    public native int[] getIntArray(CommandQueue commandQueue, int sizeArray);
    public native short[] getShortArray(CommandQueue commandQueue, int sizeArray);
    public native long[] getLongArray(CommandQueue commandQueue, int sizeArray);
    public native float[] getFloatArray(CommandQueue commandQueue, int sizeArray);
    public native byte[] getByteArray(CommandQueue commandQueue, int sizeArray);
    public native Event writeBuffer(CommandQueue commandQueue, boolean isblockingWrite, int offsetWrite, int sizeBlock);
    public native void writeIntArray(CommandQueue commandQueue, int[] data);
    public native void writeShortArray(CommandQueue commandQueue, short[] data);
    public native void writeLongArray(CommandQueue commandQueue, long[] data);
    public native void writeFloatArray(CommandQueue commandQueue, float[] data);
    public native void writeByteArray(CommandQueue commandQueue, byte[] data);
    public native Event readBufferRect(CommandQueue commandQueue, Memory dataBlock, boolean isblockingRead, int[] offsetXYZDataBlock, int[] offsetXYZBuffer,
                                       int[] regionRead, int lengthColumnsPerRowInDataBlock, int sizeAreaReadDataBlock, int lengthColumnsPerRowInBuffer,
                                       int sizeAreaReadBuffer);
    public native Event writeBufferRect(CommandQueue commandQueue, Memory dataBlock, boolean isblockingRead, int[] offsetXYZSDataBlock, int[] offsetXYZBuffer,
                                        int[] regionWrite, int lengthColumnsPerRowInDataBlock, int sizeAreaWriteDataBlock, int lengthColumnsPerRowInBuffer,
                                        int sizeAreaWriteBuffer);
    public native Event copyBuffer(CommandQueue commandQueue, Buffer srcBuffer, int offsetSrcBuffer, int offsetDstBuffer, int sizeCopy);
    public native Event copyBufferRect(CommandQueue commandQueue, Buffer srcBuffer, int[] offsetXYZSrcBuffer, int[] offsetXYZDstBuffer,
                                       int[] regionCopy, int lengthColumnsPerRowInSrcBuffer, int sizeAreaCopySrcBuffer, int lengthColumnsPerRowInDstBuffer,
                                       int sizeAreaCopyDstBuffer);
    public native Event fillBuffer(CommandQueue commandQueue, Object patternFill, int sizeFillPattern);
    public native Event mapBuffer(CommandQueue commandQueue, boolean isBlockingMap, long flagsMapBuffer, int offsetMap, int sizeMap);
    public native Event unmapBuffer(CommandQueue commandQueue);
    public native Event migrateBuffer(CommandQueue commandQueue, Buffer[] buffers, int flagsMigrateBuffer);
    public native void setReleaseCallback(CallbackBuffer notify);
    public native void releaseBuffer();
    public native String toString();
}
