package com.draico.asvappra.opencl.tools;

import android.app.Dialog;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
import com.draico.asvappra.opencl.OpenCL;
import com.draico.asvappra.opencl.R;

public class MessageError {
    public static void showMessage(final String messageError) {
        OpenCL.activity.runOnUiThread(new Runnable() {
            @Override public void run() {
                View view = OpenCL.activity.getLayoutInflater().inflate(R.layout.message_error, null);
                final Dialog dialog = new Dialog(OpenCL.activity);
                dialog.setContentView(view);
                ImageView close = view.findViewById(R.id.CloseShowMessageError);
                TextView message = view.findViewById(R.id.Message);
                message.setText(messageError);
                close.setOnClickListener(new View.OnClickListener() {
                    @Override public void onClick(View v) {
                        dialog.dismiss();
                    }
                });
                dialog.show();
            }
        });
    }
}
