package honorsproject.cs241.cameraproject;

/**
 * Created by Ping-Ko Chiu on 3/23/15.
 * Inspiration taken from http://examples.javacodegeeks.com/android/core/hardware/camera-hardware/android-camera-example/
 * For Layout, buttons and camera views.
 */
import java.io.IOException;
import java.util.List;
import java.util.LinkedList;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.os.SystemClock;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

@SuppressWarnings("deprecation")
public class CameraPreview extends SurfaceView implements SurfaceHolder.Callback, Camera.PreviewCallback {
    private static final String TAG = "CameraPreview";
    private SurfaceHolder mHolder;
    private Camera mCamera;
    private SurfaceTexture mCameraTexture;

    // For frametime calculations
    private double previousTime;
    private int frameCount = 0;

    // Position for Motion Tracking
    private int trackPos = 0;

    // Mode value
    private int mMode;

    // Camera Width and Height
    private int mCameraWidth, mCameraHeight;

    private boolean mPreviewRunning;

    // Decoded RGB values and previous frame's decoded RGB values
    private int[] mCameraDecodedRGB;
    private int[] mCameraDecodedRGBPrev;

    public CameraPreview(Context context, Camera camera) {
        super(context);
        mCamera = camera;
        mHolder = getHolder();
        // Add Callback to surface holder
        mHolder.addCallback(this);
        // For Android versions prior to 3.0
        mHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
    }

    public synchronized void surfaceCreated(SurfaceHolder holder) {
        Log.d(TAG,"surfaceCreated...");
        try {
            // create the surface and start camera preview
            if (mPreviewRunning)
                return;
            // Initialize preview
            initialize();
        } catch (Throwable e) {
            Log.d(VIEW_LOG_TAG, "Error setting camera preview: " + e.getMessage());
        }
    }

    // Get picture in Bitmap format
    public synchronized Bitmap getPicture() {
        Bitmap bitmap = Bitmap.createBitmap(mCameraWidth, mCameraHeight, Bitmap.Config.ARGB_8888);
        bitmap.setPixels(mCameraDecodedRGB, 0, mCameraWidth, 0, 0, mCameraWidth, mCameraHeight);
        return bitmap;
    }

    // Initialize camera preview, sets Surface Texture
    private void initialize() {
        Log.d(TAG, "INITIALIZING...");
        if (mCamera == null)
            mCamera = Camera.open();
        Camera.Parameters p = mCamera.getParameters();
        try {
            mCameraTexture = new SurfaceTexture(10);
            mCamera.setPreviewTexture(mCameraTexture);
        } catch (Throwable ignored) {

        }

        // Get supported sizes and min the minimum resolution
        // Code from Lawrence Angrave
        List<Camera.Size> sizes = p.getSupportedPictureSizes();
        int bestIndex = -1;
        int pixelCount = Integer.MAX_VALUE;

        for (int i = 0; i < sizes.size(); i++) {
            int w = sizes.get(i).width, h = sizes.get(i).height;
            int count = w*h;
            if (count < pixelCount) {
                bestIndex = i;
                pixelCount = count;
            }
        }
        Camera.Size size = sizes.get(bestIndex);

        mCameraWidth = size.width;
        mCameraHeight = size.height;
        Log.d(TAG, "Found Camera sizes...");

        // Initialize int[] for stored the decoded values
        mCameraDecodedRGB = new int[pixelCount];
        mCameraDecodedRGBPrev = new int[pixelCount];

        //Set preview size and format
        p.setPreviewSize(mCameraWidth, mCameraHeight);
        p.setPreviewFormat(ImageFormat.NV21);
        mCamera.setParameters(p);

        int bitsPerPixel = ImageFormat.getBitsPerPixel(p.getPreviewFormat());
        int bytes = (pixelCount*bitsPerPixel)/8;

        Log.d(TAG, "adding callback buffer");
        // Double buffer to improve frame rate
        mCamera.addCallbackBuffer(new byte[bytes]);
        mCamera.addCallbackBuffer(new byte[bytes]);

        // Setting call back with buffer allows the camera to use the buffers added
        mCamera.setPreviewCallbackWithBuffer(this);
        mPreviewRunning = true;

        //  Start Preview
        mCamera.startPreview();
        Log.d(TAG, "startPreview()");

    }

    // Refresh the Camera View
    public void refreshCamera(Camera camera, int mode) {
        // If surface does not exist
        if (mHolder.getSurface() == null) {
            return;
        }
        try {
            Log.d(TAG, "REFRESH CAMERA: STOPPING PREVIEW");
            // Stopping Preview
            mCamera.stopPreview();
        } catch (Exception e) {
        }
        // Update mode
        mMode = mode;
        // Update Camera
        setCamera(camera);
        try {
            Log.d(TAG, "REFRESH CAMERA: Set preview Display");
            // Initialize Preview again
            initialize();
        } catch (Exception e) {
            Log.d(VIEW_LOG_TAG, "Error starting camera preview: " + e.getMessage());
        }
    }

    public synchronized void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        // If surface changed, unlikely because we locked the surface to horizontal
        refreshCamera(mCamera, mMode);
    }

    // Set Camera
    public void setCamera(Camera camera) {
        mCamera = camera;
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        try {
            if (mCamera != null) {
                // Stop preview
                mCamera.stopPreview();
                mPreviewRunning = false;

                final Camera c = mCamera;
                post(new Runnable() {
                    public void run() {
                        c.release();
                    }
                });

            }
        } catch (Exception e) {
            Log.e("Camera", e.getMessage());
        }
    }

    // Get Frametime
    private double ft() {
        double currentTime = SystemClock.elapsedRealtime();
        double frameTime = currentTime - previousTime;
        previousTime = currentTime;
        frameCount++;
        return frameTime;
    }

    public synchronized void onPreviewFrame(byte[] data, Camera camera) {
        // Get frametime
        double ft = ft();
        // Only display every 10 frames
        if (frameCount == 10)
        {
            Log.d(TAG, "FrameTime: " + ft + "ms");
            frameCount = 0;
        }
        //Log.d(TAG, "ON PREVIEW FRAME...");
        Canvas canvas = null;
        try {

            // Lock canvas
            canvas = mHolder.lockCanvas(null);

            if (canvas == null || ! mPreviewRunning) {
                Log.d(TAG, "canvas is null or preview is not ready");
                return;
            }

            int canvasWidth = canvas.getWidth();
            int canvasHeight = canvas.getHeight();

            // Detect mode and perform the specific processing operations
            if (mMode == 0)
                Decode.decodeYUV(mCameraDecodedRGB, data, mCameraWidth, mCameraHeight);
            else if (mMode == 1)
                Decode.decodeYUVGrayscale(mCameraDecodedRGB, data, mCameraWidth, mCameraHeight);
            else if (mMode == 2)
                Decode.decodeYUVGaussian(mCameraDecodedRGB, data, mCameraWidth, mCameraHeight);
            else if (mMode == 3)
                Decode.decodeYUVSobelED(mCameraDecodedRGB, data, mCameraWidth, mCameraHeight);
            else if (mMode == 4)
                Decode.decodeYUVFrameSubtract(mCameraDecodedRGB, mCameraDecodedRGBPrev, data, mCameraWidth, mCameraHeight);
            else if (mMode == 5)
                Decode.decodeYUVMotionTrack(mCameraDecodedRGB, mCameraDecodedRGBPrev, data, trackPos, mCameraWidth, mCameraHeight);


            canvas.save();
            canvas.scale(canvasWidth / (float) mCameraWidth, canvasHeight / (float) mCameraHeight);
            // Draw the bitmap
            canvas.drawBitmap(mCameraDecodedRGB, 0 /*offset*/, mCameraWidth /*array stride*/, 0, 0, mCameraWidth, mCameraHeight, false, null);
            canvas.restore();

        } catch (Exception ex) {
            ex.printStackTrace();
        } finally {
            camera.addCallbackBuffer(data);

            if (canvas != null) {
                mHolder.unlockCanvasAndPost(canvas);
            }
        }
    }
}

