package com.draico.asvappra.neuralnetworks.hardware;

public class Plane {
    private int currentPlanes;
    private int currentPlane;
    private boolean isCreated;
    public native int getNumberBytesPerPixel();
    public native int getNumberBytesPerRow();
    public native byte[] getPixels();
}
