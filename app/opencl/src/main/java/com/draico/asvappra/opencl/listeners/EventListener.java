package com.draico.asvappra.opencl.listeners;

public interface EventListener {
    void setEventStatus(Event event, int status);
    void waitForEvents(Event[] listEvent);
}
