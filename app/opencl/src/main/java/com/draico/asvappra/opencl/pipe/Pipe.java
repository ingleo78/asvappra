package com.draico.asvappra.opencl.pipe;

import com.draico.asvappra.opencl.Context;

public class Pipe {
    private Context context;
    private int currentPipe;
    private int packetSize;
    private int numberPackets;
    public static int PIPE_READ_WRITE = 1 << 0;
    public static int PIPE_HOST_NO_ACCESS = 1 << 9;
    public static native Pipe createPipe(Context context, int typeAccess, int packetSize, int numberPackets);
    public native int getPacketSize();
    public native int getNumberPackets();
    public native void release();
    public native String toString();
}
