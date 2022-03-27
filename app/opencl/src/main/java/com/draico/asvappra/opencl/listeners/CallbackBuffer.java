package com.draico.asvappra.opencl.listeners;

import com.draico.asvappra.opencl.memory.buffer.Buffer;

public interface CallbackBuffer {
    void notify(Buffer buffer);
}
