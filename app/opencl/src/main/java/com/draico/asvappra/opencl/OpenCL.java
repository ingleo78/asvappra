package com.draico.asvappra.opencl;

import android.app.Activity;
import android.content.res.AssetManager;
import java.io.IOException;
import java.io.InputStream;

public class OpenCL {
    public static Activity activity;
    public static byte[] dataScript;
    public OpenCL(Activity activity) {
        this.activity = activity;
        AssetManager assetManager = activity.getAssets();
        try {
            InputStream contentScript = assetManager.open("LoadDataBlockMemory.c");
            dataScript = new byte[contentScript.available()];
            contentScript.read(dataScript, 0, contentScript.available());
            String data = new String(dataScript);
            contentScript.close();
        } catch(IOException e) { }
    }
    public native void LoadOpenCL();
    static {
        System.loadLibrary("OpenCL");
    }
}
