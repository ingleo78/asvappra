package com.draico.asvappra.opencl.listeners;

public interface CallbackContext {
    void notify(String errorInfo, byte[] data, Integer value);
}
