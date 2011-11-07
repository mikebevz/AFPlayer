package org.fpl.media;

import java.lang.ref.WeakReference;

import org.fpl.ffmpeg.Manager;

import android.content.Context;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.net.Uri;
import dk.nordfalk.netradio.Log;

//import android.util.Log;

public class MediaPlayer {

	private final static String TAG = "MediaPlayer";
	private static Manager manager;
	private static String streamUrl;
	private static int minBufSize;
	private static AudioTrack track;

	static {
		System.loadLibrary("player");
	}

	private boolean isPlaying;
	private boolean stopRequested;
	public boolean addBuzzTone = true;

	public MediaPlayer() {
		Log.d(TAG, "Create new MediaPlayer");

		minBufSize = AudioTrack.getMinBufferSize(44100,
				AudioFormat.CHANNEL_CONFIGURATION_STEREO,
				AudioFormat.ENCODING_PCM_16BIT);

		Log.d("Manager", "Buffer size: " + String.valueOf(minBufSize));

		track = new AudioTrack(AudioManager.STREAM_MUSIC, 44100,
				AudioFormat.CHANNEL_CONFIGURATION_STEREO,
				AudioFormat.ENCODING_PCM_16BIT, 88016, // minBufSize*2,
				AudioTrack.MODE_STREAM);

		n_createEngine(new WeakReference<MediaPlayer>(this));

	}

	/**
	 * Create a new instance of MediaPlayer to play stream back
	 * 
	 * @param context
	 *            Activity context
	 * @param uri
	 *            URI of the media resource, fx, a stream
	 * 
	 * @return MediaPlayer
	 */
	public static MediaPlayer create(Context context, Uri uri) {
		Log.d(TAG, "Create Stream");

		MediaPlayer mp = new MediaPlayer();
		mp.setDataSource(context, uri);
		// mp.prepare(); Not needed yet. Is here for compatibility

		return mp;

	}

	/**
	 * Set data source to be played back
	 * 
	 * @param context
	 *            Activity context
	 * @param uri
	 *            URI of the media resource
	 * 
	 * @throws IllegalStateException
	 */
	private void setDataSource(Context context, Uri uri)
			throws IllegalStateException {

		String scheme = uri.getScheme();
		if (scheme == null || scheme.equals("file")) {
			// TODO Implement file playback
			Log.d(TAG, "File given");
			return;
		}

		n_setDataSource(uri.toString()); // Play path of stream
		return;

	}

	/**
	 * Create MediaPlayer instance to play local file
	 * 
	 * @param context
	 *            Activity context
	 * @param resource
	 *            Media file resource id
	 * 
	 * @return MediaPlayer
	 */
	public static MediaPlayer create(Context context, int resource) {

		return null;
	}

	/**
	 * Start playing stream back
	 * 
	 * @throws IllegalStateException
	 */
	public void start() throws IllegalStateException {
		stopRequested = false;

		Runnable r = new Runnable() {
			public void run() {
				Log.d(TAG, "PlayThread: invoking n_playStream... ");
				n_playStream();
				Log.d(TAG, "PlayThread: n_playStream finished.");
			}
		};

		Thread playThread = new Thread(r);
		playThread.start();
		track.play();

	}

	/**
	 * Shutdown engine and release all variables
	 * 
	 * @throws IllegalStateException
	 */
	public void release() throws IllegalStateException {
		n_shutdownEngine();
	}

	public boolean isPlaying() {
		return isPlaying;
	}

	private void setPlaying(boolean status) {
		isPlaying = status;
	}

	/**
	 * Stop playback
	 * 
	 * @throws IllegalStateException
	 */
	public void stop() throws IllegalStateException {
		// n_stopStream();
		track.stop();
		stopRequested = true;

	}

	public void prepare() throws IllegalStateException {

	}

	/**
	 * Set up what needs to be set up in JNI
	 * 
	 * @param mplayer
	 *            Reference to MediaPlayer
	 */
	public native void n_createEngine(WeakReference<MediaPlayer> mplayer);

	/**
	 * Set data source - stream url for now
	 * 
	 * @param path
	 *            Stream URL
	 */
	public native void n_setDataSource(String path);

	/**
	 * Start playing stream back
	 * 
	 */
	public native void n_playStream();

	/**
	 * Stop playing stream back
	 */
	public native void n_stopStream();

	/**
	 * Shutdown decoder engine
	 */
	public native void n_shutdownEngine();

	/**
	 * Method called from JNI
	 * 
	 * @param data
	 *            Byte Array with decompressed data
	 * @param length
	 *            Length of the data in the array
	 * 
	 */
	public int streamCallback(byte[] data, int length) {

		if (!isPlaying()) {
			setPlaying(true);
		}

		Log.d(TAG, "Received " + data.length + " byte wher we use " + length);

		if (addBuzzTone) {
			for (int i = 0; i < length; i += 151)
				data[i] += i % 5 * 15;
		}

		int result = track.write(data, 0, length);
		if (result == AudioTrack.ERROR_INVALID_OPERATION || result != length) {
			Log.e(TAG, "Cannot write to AudioTrack. Ret Code: " + result);
			return 1;
		}

		if (stopRequested)
			return 1;

		return 0;

	}

}
