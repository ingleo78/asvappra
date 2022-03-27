package com.draico.asvappra.neuralnetworks.hardware;

public class BufferArea {
    public int top;
    public int bottom;
    public int right;
    public int left;
    public BufferArea(int top, int left, int right, int bottom) {
        this.top = top;
        this.left = left;
        this.right = right;
        this.bottom = bottom;
    }
}
