/*
 * NCSA Copyright License Ping-Ko Chiu, please refer to readme.md for additional information
 * Attribution: Lawrence Angrave for original decode function in Java
 */

#include <stdio.h>
#include <jni.h>
#include <android/log.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pthread.h>


#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,"Decode",__VA_ARGS__)
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,"Decode",__VA_ARGS__)

static jint * globalOut;
static jbyte * globalIn;
static jint globalHeight;
static jint globalWidth;

    /**
     * Original Decode function description by Lawrence Angrave:
     * Decodes YUV frame to a buffer which can be use to create a bitmap. use
     * this for OS < FROYO which has a native YUV decoder decode Y, U, and V
     * values on the YUV 420 buffer described as YCbCr_422_SP by Android
     *
     * @param out
     *            the outgoing array of RGB bytes
     * @param fg
     *            the incoming frame bytes
     * @param width
     *            of source frame
     * @param height
     *            of source frame

	*/
	/*
    public static void decodeYUV(final int[] out, final byte[] fg, final int width, final int height)  {
        final int sz = width * height;
        int i, j;
        int Y, Cr = 0, Cb = 0;
        int pixPtr = 0;
        for (j = 0; j < height; j++) {

            int cOff = sz + (j >> 1) * width;
            for (i = 0; i < width; i++) {
                Y = fg[pixPtr];

                if ((i & 0x1) == 0) {

                    Cb = fg[cOff++];
                    Cr = fg[cOff++];
                    if (Cb < 0)
                        Cb += 127;
                    else
                        Cb -= 128;
                    if (Cr < 0)
                        Cr += 127;
                    else
                        Cr -= 128;
                }
                if (Y < 0)
                    Y += 255;
                int R = Cr + (Cr >> 2) + (Cr >> 3) + (Cr >> 5) + Y;
                int G = Y - (Cb >> 2) + (Cb >> 4) + (Cb >> 5) - (Cr >> 1) + (Cr >> 3) + (Cr >> 4) + (Cr >> 5);
                int B = Y + Cb + (Cb >> 1) + (Cb >> 2) + (Cb >> 6);
                // Todo: Reduce branching for 99.99% typical case that R,G,B are all in range

                // if (Rok && Gok && Bok) write else slow logic
                if (R < 0)
                    R = 0;
                else if (R > 255)
                    R = 255;
                if (G < 0)
                    G = 0;
                else if (G > 255)
                    G = 255;
                if (B < 0)
                    B = 0;
                else if (B > 255)
                    B = 255;
                out[pixPtr++] = 0xff000000 | ((255 - B) << 16) | ((255 - G) << 8) | (255-R);
            }
        }

    }
    */

    /*
     *Decodes YUV to RGB adapted from Lawrence Angrave's decode function.
     *Half of the screen has the original RGB colors while the other half
     *has the inverted colors. This is the first step to showcase the possibility
     *of pixel manipulation at NDK level.
     *Added reduced branching by Ping-Ko Chiu
     *
     * @param out
     *            the outgoing array of RGB bytes
     * @param fg
     *            the incoming frame bytes
     * @param width
     *            of source frame
     * @param height
     *            of source frame
     */
void toRGBnInvert(jint* out, jbyte* fg, const int width, const int height)
{
        const int sz = width * height;
        int i, j;
        int Y, Cr = 0, Cb = 0;
        int pixPtr = 0;
        for (j = 0; j < height; j++) {

            int cOff = sz + (j >> 1) * width;
            for (i = 0; i < width/2; i++) {
                Y = fg[pixPtr];

                if ((i & 0x1) == 0) {

                    Cb = fg[cOff++];
                    Cr = fg[cOff++];
                    if (Cb < 0)
                        Cb += 127;
                    else
                        Cb -= 128;
                    if (Cr < 0)
                        Cr += 127;
                    else
                        Cr -= 128;
                }
                if (Y < 0)
                    Y += 255;
                int R = Cr + (Cr >> 2) + (Cr >> 3) + (Cr >> 5) + Y;
                int G = Y - (Cb >> 2) + (Cb >> 4) + (Cb >> 5) - (Cr >> 1) + (Cr >> 3) + (Cr >> 4) + (Cr >> 5);
                int B = Y + Cb + (Cb >> 1) + (Cb >> 2) + (Cb >> 6);

                // if (!(Rok && Gok && Bok)) reduce branching (Prof. Angrave's optimization idea implemented here)
                if (!(R < 255 && R > 0 && G < 255 && G > 0 && B < 255 && B > 0))
                {
                    if (R < 0)
                        R = 0;
                    else if (R > 255)
                        R = 255;
                    if (G < 0)
                        G = 0;
                    else if (G > 255)
                        G = 255;
                    if (B < 0)
                        B = 0;
                    else if (B > 255)
                        B = 255;
                }
                out[pixPtr++] = 0xff000000 | (B << 16) | (G << 8) | R;
            }
            for (i = width/2; i < width; i++) {
                Y = fg[pixPtr];

                if ((i & 0x1) == 0) {

                    Cb = fg[cOff++];
                    Cr = fg[cOff++];
                    if (Cb < 0)
                        Cb += 127;
                    else
                        Cb -= 128;
                    if (Cr < 0)
                        Cr += 127;
                    else
                        Cr -= 128;
                }
                if (Y < 0)
                    Y += 255;
                int R = Cr + (Cr >> 2) + (Cr >> 3) + (Cr >> 5) + Y;
                int G = Y - (Cb >> 2) + (Cb >> 4) + (Cb >> 5) - (Cr >> 1) + (Cr >> 3) + (Cr >> 4) + (Cr >> 5);
                int B = Y + Cb + (Cb >> 1) + (Cb >> 2) + (Cb >> 6);

                // if (!(Rok && Gok && Bok)) reduce branching (Prof. Angrave's optimization idea implemented here)
                if (!(R < 255 && R > 0 && G < 255 && G > 0 && B < 255 && B > 0))
                {
                    if (R < 0)
                        R = 0;
                    else if (R > 255)
                        R = 255;
                    if (G < 0)
                        G = 0;
                    else if (G > 255)
                        G = 255;
                    if (B < 0)
                        B = 0;
                    else if (B > 255)
                        B = 255;
                }
                out[pixPtr++] = 0xff000000 | ((255 - B) << 16) | ((255 - G) << 8) | (255-R);
            }
        }

}


    /*
     *Decodes YUV to Grayscale.
     *
     * @param out
     *            the outgoing array of RGB bytes
     * @param fg
     *            the incoming frame bytes
     * @param width
     *            of source frame
     * @param height
     *            of source frame
     */
void toGrayscale(jint* out, jbyte* fg, const int width, const int height)
{

	int p, i;
	int size = width*height;

	for (i = 0; i < size; i++)
	{
		p = fg[i] & 0xff;
		out[i] = 0xff000000 | p << 16 | p << 8 | p;
	}
}


    /*
     * NDK function for decodeYUV.
     * Calls toRGBnInvert(...)
     *
     * @param out
     *            the outgoing array of RGB bytes
     * @param fg
     *            the incoming frame bytes
     * @param width
     *            of source frame
     * @param height
     *            of source frame
     */

JNIEXPORT void JNICALL Java_honorsproject_cs241_cameraproject_Decode_decodeYUV
  (JNIEnv * env, jclass jc, jintArray out , jbyteArray fg, jint width , jint height) {

  jint* outarray= (*env)->GetIntArrayElements(env, out, 0);
  jbyte* fgarray= (*env)->GetByteArrayElements(env, fg, 0);

	toRGBnInvert(outarray, fgarray, width, height);

  (*env)->ReleaseIntArrayElements(env,out,outarray,0);
  (*env)->ReleaseByteArrayElements(env,fg, fgarray,0);
  }



    /*
     * NDK function for decodeYUVGrayscale.
     * Calls toGrayscale(...)
     *
     * @param out
     *            the outgoing array of RGB bytes
     * @param fg
     *            the incoming frame bytes
     * @param width
     *            of source frame
     * @param height
     *            of source frame
     */
JNIEXPORT void JNICALL Java_honorsproject_cs241_cameraproject_Decode_decodeYUVGrayscale
  (JNIEnv * env, jclass jc, jintArray out , jbyteArray fg, jint width , jint height) {

  jint* outarray= (*env)->GetIntArrayElements(env, out, 0);
  jbyte* fgarray= (*env)->GetByteArrayElements(env, fg, 0);

	toGrayscale(outarray, fgarray, width, height);

  (*env)->ReleaseIntArrayElements(env,out,outarray,0);
  (*env)->ReleaseByteArrayElements(env,fg, fgarray,0);
}



    /*
     * NDK function for decodeYUVGaussian.
     * Applies Gaussian Blur on Grayscale image
     * using convolution with 5x5 Gaussian Kernel
     * No Optimizations, no multithread
     *
     * @param out
     *            the outgoing array of RGB bytes
     * @param fg
     *            the incoming frame bytes
     * @param width
     *            of source frame
     * @param height
     *            of source frame
     */
JNIEXPORT void JNICALL Java_honorsproject_cs241_cameraproject_Decode_decodeYUVGaussian
  (JNIEnv * env, jclass jc, jintArray out , jbyteArray fg, jint width , jint height) {

  jint* outarray= (*env)->GetIntArrayElements(env, out, 0);
  jbyte* fgarray= (*env)->GetByteArrayElements(env, fg, 0);

	//5x5 Gaussian Kernel taken from http://homepages.inf.ed.ac.uk/rbf/HIPR2/gsmooth.htm
	int gaussianK[5][5] = {{1, 4, 7, 4, 1}, {4, 16, 26, 16, 4}, {7, 26, 41, 26, 7}, {4, 16, 26, 16, 4}, {1, 4, 7, 4, 1}};
	int i, q, t;
	// Holds convolution sum of the surrounding 25 pixels
	int SUM;
	int size = width*height;

	// traverse through all pixels, starting early and ending early to reduce operations
	for (i = 2*width + 2; i < size-2*width-2; i++)
	{
		SUM = 0;
		int x = i % width;
		int y = i / width;
		// Check bounds
		if (x != 0 && x != width - 1 && y != 0 && y != height - 1)
		{
			// Calculate Convolution sum
			for (q = -2; q <= 2; q ++)
			{
				for (t = -2; t <= 2; t++)
				{
					SUM += (int)(((fgarray[i + q + t*width]) & 0xff) * gaussianK[q+2][t+2]);
				}
			}
		}
		// Normalize by dividing sum of kernel values
		int avg = SUM / 273;
		if (avg > 255)
			avg = 255;
		outarray[i] = 0xff000000 | avg << 16 | avg << 8 | avg;
	}

  (*env)->ReleaseIntArrayElements(env,out,outarray,0);
  (*env)->ReleaseByteArrayElements(env,fg, fgarray,0);

  }


    /*
     * Thread function for Sobel Edge Detection.
     * Using convolution with 3x3 Sobel Kernel
     */
void* sobelWorker(void* args) {
	// Convert argument to number
	int num = *(int*)args;
	// Chunk size responsible
	int chunk = (globalHeight*globalWidth / 4);
	// Starting and ending position
	int start = chunk*num;
	int end = start + chunk;
	// If handling the last chunk, reduce operation
	if (num == 3)
		end -= globalWidth;
	// 3x3 X and Y gradient kernels
	int GX[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
	int GY[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
	int p, i, t;
	int SUMx, SUMy, SUM;
	// Traverse through pixels
	for (i = start + 1; i < end-1; i++)
	{
		SUMx = SUMy = 0;
		int x = i % globalWidth;
		int y = i / globalWidth;
		// Check for bounds
		if (x != 0 && x != globalWidth - 1 && y != 0 && y != globalHeight - 1)
		{
				// Apply Convolution, loop unrolled for optimization (hopefully compiler can optimize)
				for (t = -1; t <= 1; t++)
				{
					SUMx += (int)(((globalIn[i -1 + t*globalWidth]) & 0xff) * GX[0][t+1] + ((globalIn[i + t*globalWidth]) & 0xff) * GX[1][t+1] + ((globalIn[i +1 +t*globalWidth]) & 0xff) * GX[2][t+1]);
				}

				for (t = -1; t <= 1; t++)
				{
					SUMy += (int)(((globalIn[i -1 + t*globalWidth]) & 0xff) * GY[0][t+1] + ((globalIn[i + t*globalWidth]) & 0xff) * GY[1][t+1] + ((globalIn[i +1 +t*globalWidth]) & 0xff) * GY[2][t+1]);
				}
		}
		// Using an approximation instead of sqrt
		SUM = abs(SUMx) + abs(SUMy);
		if (SUM > 255)
			SUM = 255;
		if (SUM < 100)
			SUM = 0;
		globalOut[i] = 0xff000000 | SUM << 16 | SUM << 8 | SUM;
	}
	free(args);
	return NULL;
}

    /*
     * NDK function for decodeYUVSobelED.
     * Applies Sobel edge detection on Grayscale image
     * using convolution with 3x3 X, Y Gradient Kernel
     * Multi-threaded for optimized performance (slight improvements, might be memory bound)
     * Unrolled Loops for easier vectorization
     *
     * @param out
     *            the outgoing array of RGB bytes
     * @param fg
     *            the incoming frame bytes
     * @param width
     *            of source frame
     * @param height
     *            of source frame
     */
JNIEXPORT void JNICALL Java_honorsproject_cs241_cameraproject_Decode_decodeYUVSobelED
  (JNIEnv * env, jclass jc, jintArray out , jbyteArray fg, jint width , jint height) {

  jint* outarray= (*env)->GetIntArrayElements(env, out, 0);
  jbyte* fgarray= (*env)->GetByteArrayElements(env, fg, 0);

	// Store array in global vars to avoid having to pass them in thread function arguments
	globalOut = outarray;
	globalIn = fgarray;
	globalWidth = width;
	globalHeight = height;

	pthread_t tid[3];

	for (int j = 1; j < 4; j++) {
		int * num = malloc(sizeof(int));
		*num = j;
		pthread_create(&tid[j-1], NULL, sobelWorker, num);
	}
	// 3x3 X and Y gradient kernels
	int GX[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
	int GY[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
	int p, i, t;
	int SUMx, SUMy, SUM;
	int size = width*height;
	// Traverse through pixels
	for (i = width + 1; i < size-width-1; i++)
	{
		SUMx = SUMy = 0;
		int x = i % globalWidth;
		int y = i / globalWidth;
		// Check for bounds
		if (x != 0 && x != globalWidth - 1 && y != 0 && y != globalHeight - 1)
		{
				// Apply Convolution, loop unrolled for optimization (hopefully compiler can optimize)
				for (t = -1; t <= 1; t++)
				{
					SUMx += (int)(((globalIn[i -1 + t*globalWidth]) & 0xff) * GX[0][t+1] + ((globalIn[i + t*globalWidth]) & 0xff) * GX[1][t+1] + ((globalIn[i +1 +t*globalWidth]) & 0xff) * GX[2][t+1]);
				}

				for (t = -1; t <= 1; t++)
				{
					SUMy += (int)(((globalIn[i -1 + t*globalWidth]) & 0xff) * GY[0][t+1] + ((globalIn[i + t*globalWidth]) & 0xff) * GY[1][t+1] + ((globalIn[i +1 +t*globalWidth]) & 0xff) * GY[2][t+1]);
				}
		}
		// Using an approximation instead of sqrt
		SUM = abs(SUMx) + abs(SUMy);
		if (SUM > 255)
			SUM = 255;
		if (SUM < 100)
			SUM = 0;
		outarray[i] = 0xff000000 | SUM << 16 | SUM << 8 | SUM;
	}
	// Join all threads
	for (int j = 1; j < 4; j++) {
		pthread_join(tid[j-1], NULL);
	}
  (*env)->ReleaseIntArrayElements(env,out,outarray,0);
  (*env)->ReleaseByteArrayElements(env,fg, fgarray,0);

  }


    /*
     * ndk function for decodeyuvframesubtract.
     * extract current rgb values and compare to previous frame's rgba values
     * previous frame's rgb values are passed in, processed and updated at the end
     * using a tolerance value of 10%, colors all changes > 10% in red, otherwise in original color
     *
     * @param out
     *            the outgoing array of rgb bytes
     * @param prev
     *            the previous frame's array of rgb bytes
     * @param fg
     *            the incoming frame bytes
     * @param width
     *            of source frame
     * @param height
     *            of source frame
     */
JNIEXPORT void JNICALL Java_honorsproject_cs241_cameraproject_Decode_decodeYUVFrameSubtract
  (JNIEnv * env, jclass jc, jintArray out , jintArray prev ,jbyteArray fg, jint width , jint height) {

  jint* outarray= (*env)->GetIntArrayElements(env, out, 0);
  jint* prevarray= (*env)->GetIntArrayElements(env, prev, 0);
  jbyte* fgarray= (*env)->GetByteArrayElements(env, fg, 0);

        const int sz = width * height;
        int i, j;
        int Y, Cr = 0, Cb = 0;
        int pixPtr = 0;
        for (j = 0; j < height; j++) {

            int cOff = sz + (j >> 1) * width;
            for (i = 0; i < width; i++) {
                Y = fgarray[pixPtr];

                if ((i & 0x1) == 0) {

                    Cb = fgarray[cOff++];
                    Cr = fgarray[cOff++];
                    if (Cb < 0)
                        Cb += 127;
                    else
                        Cb -= 128;
                    if (Cr < 0)
                        Cr += 127;
                    else
                        Cr -= 128;
                }
                if (Y < 0)
                    Y += 255;
                int R = Cr + (Cr >> 2) + (Cr >> 3) + (Cr >> 5) + Y;
                int G = Y - (Cb >> 2) + (Cb >> 4) + (Cb >> 5) - (Cr >> 1) + (Cr >> 3) + (Cr >> 4) + (Cr >> 5);
                int B = Y + Cb + (Cb >> 1) + (Cb >> 2) + (Cb >> 6);
                // if (!(Rok && Gok && Bok)) reduce branching (Prof. Angrave's optimization idea implemented here by Ping-Ko Chiu)
                if (!(R < 255 && R > 0 && G < 255 && G > 0 && B < 255 && B > 0))
                {
                    if (R < 0)
                        R = 0;
                    else if (R > 255)
                        R = 255;
                    if (G < 0)
                        G = 0;
                    else if (G > 255)
                        G = 255;
                    if (B < 0)
                        B = 0;
                    else if (B > 255)
                        B = 255;
                }

				// Extract Previous RGB values
                int prevR = prevarray[pixPtr] & 0xff;
                int prevG = (prevarray[pixPtr] >> 8) & 0xff;
                int prevB = (prevarray[pixPtr] >> 16) & 0xff;
                // Take Difference of each value and their sum divided by 3
                int eval = (abs(R - prevR) + abs(G - prevG) + abs(B - prevB))/3;
                // If difference is > 10%, change pixel to red
                if (eval > 26)
                {
                    outarray[pixPtr] = 0xff000000 | (255 << 16);
                }
                // Else show as current pixel colors
                else
                {
                    outarray[pixPtr] = 0xff000000 | (B << 16) | (G << 8) | R;
                }
                // Store to prevarray;
                prevarray[pixPtr] = 0xff000000 | (B << 16) | (G << 8) | R;
                pixPtr++;
            }
        }

  (*env)->ReleaseIntArrayElements(env,out,outarray,0);
  (*env)->ReleaseIntArrayElements(env,prev,prevarray,0);
  (*env)->ReleaseByteArrayElements(env,fg, fgarray,0);
  }


    /*
     * NDK function for decodeYUVMotionTrack.
     * A feature on top of the FrameSubtract feature to track motino.
     * All the basic technical details remains from frame subtract,
     * e.g. difference in color, 10% tolerance...
     * Pixels that are considered "changed" will be accounted for
     * centerOfMotionX and centerOfMotionY calculations.
     * A cross will be drawn at the position of centerOfMotion.
     *
     * @param out
     *            the outgoing array of rgb bytes
     * @param prev
     *            the previous frame's array of rgb bytes
     * @param fg
     *            the incoming frame bytes
     * @param pos
     *            the position of tracking
     * @param width
     *            of source frame
     * @param height
     *            of source frame
     */
JNIEXPORT void JNICALL Java_honorsproject_cs241_cameraproject_Decode_decodeYUVMotionTrack
  (JNIEnv * env, jclass jc, jintArray out , jintArray prev ,jbyteArray fg, jint pos, jint width , jint height) {

  jint* outarray= (*env)->GetIntArrayElements(env, out, 0);
  jint* prevarray= (*env)->GetIntArrayElements(env, prev, 0);
  jbyte* fgarray= (*env)->GetByteArrayElements(env, fg, 0);

	// To find centerOfMotion positions
	int centerOfMotionX = 0;
	int centerOfMotionY = 0;
	// Counts pixels chaged so we can later determine if changes are significant
	int pixelsChanged = 0;
        const int sz = width * height;
        int i, j;
        int Y, Cr = 0, Cb = 0;
        int pixPtr = 0;
        for (j = 0; j < height; j++) {

            int cOff = sz + (j >> 1) * width;
            for (i = 0; i < width; i++) {
                Y = fgarray[pixPtr];

                if ((i & 0x1) == 0) {

                    Cb = fgarray[cOff++];
                    Cr = fgarray[cOff++];
                    if (Cb < 0)
                        Cb += 127;
                    else
                        Cb -= 128;
                    if (Cr < 0)
                        Cr += 127;
                    else
                        Cr -= 128;
                }
                if (Y < 0)
                    Y += 255;
                int R = Cr + (Cr >> 2) + (Cr >> 3) + (Cr >> 5) + Y;
                int G = Y - (Cb >> 2) + (Cb >> 4) + (Cb >> 5) - (Cr >> 1) + (Cr >> 3) + (Cr >> 4) + (Cr >> 5);
                int B = Y + Cb + (Cb >> 1) + (Cb >> 2) + (Cb >> 6);
                // if (!(Rok && Gok && Bok)) reduce branching (Prof. Angrave's optimization idea implemented here by Ping-Ko Chiu)
                if (!(R < 255 && R > 0 && G < 255 && G > 0 && B < 255 && B > 0))
                {
                    if (R < 0)
                        R = 0;
                    else if (R > 255)
                        R = 255;
                    if (G < 0)
                        G = 0;
                    else if (G > 255)
                        G = 255;
                    if (B < 0)
                        B = 0;
                    else if (B > 255)
                        B = 255;
                }

				// Extract Previous RGB values
                int prevR = prevarray[pixPtr] & 0xff;
                int prevG = (prevarray[pixPtr] >> 8) & 0xff;
                int prevB = (prevarray[pixPtr] >> 16) & 0xff;
                // Take Difference of each value and their sum divided by 3
                int eval = (abs(R - prevR) + abs(G - prevG) + abs(B - prevB))/3;
                // If difference is > 10%, consider pixel changed
                if (eval > 26)
                {
                    // Increment Pixels Changed
                    pixelsChanged++;
                    // Sum position
                    centerOfMotionX += i;
                    centerOfMotionY += j;
                }
                outarray[pixPtr] = prevarray[pixPtr] = 0xff000000 | (B << 16) | (G << 8) | R;
                pixPtr++;
            }
        }
        // If the number of Pixels changed is significant
        if (pixelsChanged > 1024)
            // Calculate center of motion and update to Pos
            pos = (centerOfMotionX /= pixelsChanged) + (centerOfMotionY/=pixelsChanged)* width;
        // Draw Green cross at centerOfMotion
        for (int i = -15; i < 16; i++)
        {
            for (int j = -3; j < 4; j++)
            {
                int index = pos + i + j*width;
                if (index >= 0 && index < sz)
                    outarray[index] = 0xff000000 | (255 << 8);
            }
        }
        for (int i = -3; i < 4; i++)
        {
            for (int j = -15; j < 16; j++)
            {
                int index = pos + i + j*width;
                if (index >= 0 && index < sz)
                    outarray[index] = 0xff000000 | (255 << 8);
            }
        }

  (*env)->ReleaseIntArrayElements(env,out,outarray,0);
  (*env)->ReleaseIntArrayElements(env,prev,prevarray,0);
  (*env)->ReleaseByteArrayElements(env,fg, fgarray,0);
  }
