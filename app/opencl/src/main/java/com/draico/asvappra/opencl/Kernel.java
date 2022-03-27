package com.draico.asvappra.opencl;

import com.draico.asvappra.opencl.functioncustom.FunctionNativeKernel;
import com.draico.asvappra.opencl.listeners.Event;

public class Kernel {
    private Program program;
    private int currentKernel;
    private boolean isSetArguments;
    private boolean isKernelExecuted;
    public static int EXEC_INFO_SVM_PTRS = 0x11B6;
    public static int EXEC_INFO_SVM_FINE_GRAIN_SYSTEM = 0x11B7;
    public static native Kernel createKernel(Program program, String kernelName);
    public static native Kernel[] createKernelsInProgram(Program program);
    public static native Event nativeKernel(CommandQueue commandQueue, FunctionNativeKernel functionNativeKernel, Object ... argsFunctionNativeKernel);
    public native Event NDRangeKernel(CommandQueue commandQueue, int workNumberDimensions, int[] globalWorkOffset, int[] globalWorkSize, int[] localWorkSize);
    public native void releaseKernel();
    public native void setKernelArguments(CommandQueue commandQueue, Object ... arguments);
    public native void setKernelArgumentsSVMPointer(CommandQueue commandQueue, Object ... arguments);
    public native void setKernelExecInfoSVM(CommandQueue commandQueue, int execInfo, Object ... arguments);
    public native Kernel cloneKernel();
    public native String getFunctionName();
    public native int getNumberArguments();
    public native Context getContext();
    public native Program getProgram();
    public native String getAtributes();
    public native int[] getGlobalWorkSize(Device device);
    public native int getWorkGroupSize(Device device);
    public native int[] getCompileWorkGroupSize(Device device);
    public native long getLocalMemorySize(Device device);
    public native int getPreferredWorkSizeMultiple(Device device);
    public native long getPrivateMemorySize(Device device);
    public native int getMaxSubGroupSizeForNDRange(Device device, int ... sizeSubGroups);
    public native int getSubGroupCountForNDRange(Device device, int ... localWorkSize);
    public native int[] getLocalSizeForSubGroupCount(Device device, int numberSubGroups);
    public native int getMaxNumberSubGroups(Device device);
    public native int getCompileNumSubGroups(Device device);
    public native Object[] getArgumentsKernel(CommandQueue commandQueue);
    public native String getArgumentAddressQualifier();
    public native String getArgumentAccessQualifier();
    public native String getArgumentTypeName();
    public native String getArgumentTypeQualifier();
    public native String getName();
    public native String toString();
}
