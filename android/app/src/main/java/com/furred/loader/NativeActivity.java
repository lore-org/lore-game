package com.furred.loader;

import android.util.Log;
import androidx.appcompat.app.AppCompatActivity;
import com.furred.lore_game.R;
import android.os.Bundle;

public class NativeActivity extends AppCompatActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        System.loadLibrary(getString(R.string.sanitised_app_name));
        androidMain();
    }

    public native int androidMain();
}
