package com.draico.asvappra.neuralnetworks;

public class Execution {
    private int currentExecution;
    private boolean isCreated;
    public static int DURATION_ON_HARDWARE = 0;
    public static int DURATION_IN_DRIVER = 1;
    public static int FENCED_DURATION_ON_HARDWARE = 2;
    public static int FENCED_DURATION_IN_DRIVER = 3;
    public static native Execution newExecution(Compilation compilation);
    public native void burstCompute(Burst burst);
    public native void compute();
    public native void delete();
    public native long getDuration(int durationType);
    public native int[] getOutputOperandDimensions(int indexOutput);
    public native int getOutputOperandRank(int dexOutput);
    public native void setInput(OperandType operandType, Object data, int indexInput);
    public native void setInputFromMemory(OperandType operandType, Memory memory, int indexInput, int offset, int size);
    public native void setLoopTimeout(long duration);
    public native void setMeasureTiming(boolean isMeasure);
    public native void setOutput(OperandType operandType, Object data, int indexOutput);
    public native void setOutputFromMemory(OperandType operandType, Memory memory, int indexOutput, int offset, int size);
    public native void setTimeout(long duration);
    public native void startCompute(Event event);
    public native Event startComputeWithDependencies(Event[] events, long duration);
}
