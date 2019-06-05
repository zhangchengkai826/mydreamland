package com.mydreamland.www;

import android.Manifest;
import android.app.AlertDialog;
import android.app.NativeActivity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.pm.PackageManager;

public class Util {
    final static int REQUEST_WRITE_EXTERNAL_STORAGE = 0;

    public static boolean checkPermission(NativeActivity activity) {
        if(activity.checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)
                == PackageManager.PERMISSION_GRANTED) {
            return true;
        } else {
            activity.requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    REQUEST_WRITE_EXTERNAL_STORAGE);
            return false;
        }
    }
}
