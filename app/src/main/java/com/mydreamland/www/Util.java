package com.mydreamland.www;

import android.app.AlertDialog;
import android.app.NativeActivity;
import android.content.Context;
import android.content.DialogInterface;

public class Util {
    public static void checkPermission(NativeActivity activity) {
        new AlertDialog.Builder(activity).setTitle("Info")
                .setMessage("first jni call")
                .setPositiveButton(android.R.string.ok, null)
                .setIcon(android.R.drawable.ic_dialog_alert).show();
    }
}
