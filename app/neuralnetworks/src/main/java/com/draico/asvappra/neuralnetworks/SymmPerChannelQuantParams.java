package com.draico.asvappra.neuralnetworks;

public class SymmPerChannelQuantParams {
    public int channelDim;
    public int scaleCount;
    public float[] scales;
    public SymmPerChannelQuantParams(int channelDim, int scaleCount, float[] scales) {
        this.channelDim = channelDim;
        this.scaleCount = scaleCount;
        this.scales = scales;
    }
}
