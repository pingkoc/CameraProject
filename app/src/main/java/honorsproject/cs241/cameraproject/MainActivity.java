package honorsproject.cs241.cameraproject;

    /*
     * Inspiration taken from http://examples.javacodegeeks.com/android/core/hardware/camera-hardware/android-camera-example/
     * For Layout, buttons and camera views.
     * Adopted some implementation from Lawrence Angrave's Streamer project for CS241 UIUC
     * Written by Ping-Ko Chiu
     */
import android.graphics.Bitmap;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.util.Log;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;

import android.content.Context;
import android.content.pm.PackageManager;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.PictureCallback;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.Toast;

@SuppressWarnings("deprecation")
public class MainActivity extends ActionBarActivity {
    private static final String TAG = "MainActivity";
    private Camera mCamera;
    private CameraPreview mPreview;
    private PictureCallback mPicture;
    private Button capture, switchCamera, switchMode;
    private Context myContext;
    private LinearLayout cameraPreview;
    private boolean cameraFront = false;
    private int mMode = 0;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate");
        super.onCreate(savedInstanceState);
        // Set Content view
        setContentView(R.layout.activity_main);
        // Keep Screen on
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        myContext = this;
        // Initialize view
        initialize();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        // Set preview to NULL
        mPreview = null;
    }

    // Find front facing camera, code from Java Code Geek
    private int findFrontFacingCamera() {
        int cameraId = -1;
        // Search for the front facing camera
        int numberOfCameras = Camera.getNumberOfCameras();
        for (int i = 0; i < numberOfCameras; i++) {
            CameraInfo info = new CameraInfo();
            Camera.getCameraInfo(i, info);
            if (info.facing == CameraInfo.CAMERA_FACING_FRONT) {
                cameraId = i;
                cameraFront = true;
                break;
            }
        }
        // return front cameraID
        return cameraId;
    }

    // Find back facing camera, code from Java Code Geek
    private int findBackFacingCamera() {
        int cameraId = -1;
        //Search for the back facing camera
        int numberOfCameras = Camera.getNumberOfCameras();
        //for every camera check
        for (int i = 0; i < numberOfCameras; i++) {
            CameraInfo info = new CameraInfo();
            Camera.getCameraInfo(i, info);
            if (info.facing == CameraInfo.CAMERA_FACING_BACK) {
                cameraId = i;
                cameraFront = false;
                break;
            }
        }
        // return back cameraID
        return cameraId;
    }

    public void onResume() {
        super.onResume();
        if (!hasCamera(myContext)) {
            finish();
        }
        if (mCamera == null) {
            // On resume, check if front exists and notify user
            if (findFrontFacingCamera() < 0) {
                Toast.makeText(this, "No front Camera found", Toast.LENGTH_LONG).show();
                // Remove button visibility
                switchCamera.setVisibility(View.GONE);
            }
            // Open back camera as default
            mCamera = Camera.open(findBackFacingCamera());
            // Refresh camera
            mPreview.refreshCamera(mCamera, mMode);
        }
    }

    // Initialize view, set up cameraPreview, capture, switch camera and mode
    public void initialize() {
        cameraPreview = (LinearLayout) findViewById(R.id.camera_preview);
        mPreview = new CameraPreview(myContext, mCamera);
        cameraPreview.addView(mPreview);

        capture = (Button) findViewById(R.id.button_capture);
        capture.setOnClickListener(captrureListener);

        switchCamera = (Button) findViewById(R.id.button_ChangeCamera);
        switchCamera.setOnClickListener(switchCameraListener);

        switchMode = (Button) findViewById(R.id.button_switchmode);
        switchMode.setOnClickListener(switchModeListener);
    }

    // OnClickListener for SwitchCamera Button
    OnClickListener switchCameraListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            // Check if there are more than 1 camera
            int camerasNumber = Camera.getNumberOfCameras();
            if (camerasNumber > 1) {

                Log.d(TAG, "RELEASING CAMERA");
                // Release camera first!
                releaseCamera();
                Log.d(TAG, "CHOOSING OTHER CAMERA");
                // Switch to other camera (will refresh)
                switchCamera();
            }
        }
    };

    // OnClictListener for Mode Switch button
    OnClickListener switchModeListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            // Cycle through modes
            mMode = (mMode + 1) % 6;
            if (mMode == 0) {
                Toast toast = Toast.makeText(myContext, "Mode Switched to: RGB and Inverted RGB", Toast.LENGTH_LONG);
                toast.show();
            } else if (mMode == 1) {
                Toast toast = Toast.makeText(myContext, "Mode Switched to: Grayscale", Toast.LENGTH_LONG);
                toast.show();
            } else if (mMode == 2) {
                Toast toast = Toast.makeText(myContext, "Mode Switched to: Gaussian Blur", Toast.LENGTH_LONG);
                toast.show();
            } else if (mMode == 3) {
                Toast toast = Toast.makeText(myContext, "Mode Switched to: Sobel Edge Detection", Toast.LENGTH_LONG);
                toast.show();
            } else if (mMode == 4) {
                Toast toast = Toast.makeText(myContext, "Mode Switched to: Frame Subtraction", Toast.LENGTH_LONG);
                toast.show();
            } else if (mMode == 5) {
                Toast toast = Toast.makeText(myContext, "Mode Switched to: Motion Tracking", Toast.LENGTH_LONG);
                toast.show();
            }
            // Refresh Camera after changes
            mPreview.refreshCamera(mCamera, mMode);
        }
    };

    public void switchCamera() {
        // If Front Camera, switch to back
        if (cameraFront)
        {
            int cameraId = findBackFacingCamera();
            if (cameraId >= 0)
            {
                mCamera = Camera.open(cameraId);
                Log.d(TAG, "REFRESHING CAMERA");
                // Refresh Camera
                mPreview.refreshCamera(mCamera, mMode);
            }
        } else // If back camera, switch to front
        {
            int cameraId = findFrontFacingCamera();
            if (cameraId >= 0)
            {
                mCamera = Camera.open(cameraId);
                Log.d(TAG, "REFRESHING CAMERA");
                // Refresh Camera
                mPreview.refreshCamera(mCamera, mMode);
            }
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        //when on Pause, release camera in order to be used from other applications
        releaseCamera();
    }

    // hasCamera function
    private boolean hasCamera(Context context) {
        if (context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_CAMERA)) {
            return true;
        } else {
            return false;
        }
    }


    OnClickListener captrureListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            //mCamera.takePicture(null, null, mPicture);
            File pictureFile = getOutputMediaFile();
            FileOutputStream out = null;
            if (pictureFile == null) {
                return;
            }
            try {
                //write the file
                out = new FileOutputStream(pictureFile);
                Bitmap pic = mPreview.getPicture();
                pic.compress(Bitmap.CompressFormat.PNG, 100, out);
                out.close();
                Toast toast = Toast.makeText(myContext, "Picture saved: " + pictureFile.getName(), Toast.LENGTH_LONG);
                toast.show();

            } catch (FileNotFoundException e) {
            } catch (IOException e) {
            } finally {
                try {
                    if (out != null)
                        out.close();

                } catch (IOException e) {
                    e.printStackTrace();
                }
            }

            //refresh camera to continue preview
            mPreview.refreshCamera(mCamera, mMode);

        }
    };

    //make picture and save to a folder
    private static File getOutputMediaFile() {
        //make a new file directory inside the "sdcard" folder
        File mediaStorageDir = new File("/sdcard/", "CameraProject");

        //if this "JCGCamera folder does not exist
        if (!mediaStorageDir.exists()) {
            //if you cannot make this folder return
            if (!mediaStorageDir.mkdirs()) {
                return null;
            }
        }

        //take the current timeStamp
        String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
        File mediaFile;
        //and make a media file:
        mediaFile = new File(mediaStorageDir.getPath() + File.separator + "IMG_" + timeStamp + ".jpg");

        return mediaFile;
    }

    private void releaseCamera() {
        // stop and release camera
        if (mCamera != null) {
            mCamera.release();
            mCamera = null;
        }
    }
}
