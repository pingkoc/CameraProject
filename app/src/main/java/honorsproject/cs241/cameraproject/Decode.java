package honorsproject.cs241.cameraproject;

/**
 * Created by Ping-Ko Chiu
 * CREDITS TO LAWRENCE ANGRAVE FOR HIS MODIFICATION ON DECODE
 *
 * NDK wrapper for decode
 */
public class Decode {

    static {
        System.loadLibrary("ndkDecode");
    }
    public static native void decodeYUV(final int[] out, final byte[] fg, final int width, final int height);
    public static native void decodeYUVGrayscale(final int[] out, final byte[] fg, final int width, final int height);
    public static native void decodeYUVGaussian(final int[] out, final byte[] fg, final int width, final int height);
    public static native void decodeYUVSobelED(final int[] out, final byte[] fg, final int width, final int height);
    public static native void decodeYUVFrameSubtract(final int[] out, final int[] previous, final byte[] fg, final int width, final int height);
    public static native void decodeYUVMotionTrack(final int[] out, final int[] previous, final byte[] fg, final int pos, final int width, final int height);

}
