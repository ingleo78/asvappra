package com.draico.asvappra.opencl.listeners;

import com.draico.asvappra.opencl.CommandQueue;
import com.draico.asvappra.opencl.memory.Memory;

public interface CallbackMemory {
    void notify(CommandQueue commandQueue, Memory[] memory);
}
