package org.fpl.ffmpeg;

import java.io.IOException;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;

public class Manager {

	private static final int BUFFER_SIZE = 50;

	private static final String TAG = "Manager";

	static {
		System.loadLibrary("player");
	}

	private AudioTrack track;
	// Queue<byte[]> buffer;
	// ArrayList<byte[]> buffer;

	private int minBufSize;
	byte[] buffer = new byte[minBufSize];
	boolean started = false;
	int overallBytes = 0;

	private int nBytesRead;

	public Manager() {

		// buffer = new ArrayList<byte[]>();//new
		// ArrayDeque<byte[]>(BUFFER_SIZE);

		minBufSize = AudioTrack.getMinBufferSize(44100,
				AudioFormat.CHANNEL_CONFIGURATION_STEREO,
				AudioFormat.ENCODING_PCM_16BIT);

		Log.d("Manager", "Buffer size: " + String.valueOf(minBufSize));

		track = new AudioTrack(AudioManager.STREAM_MUSIC, 44100,
				AudioFormat.CHANNEL_CONFIGURATION_STEREO,
				AudioFormat.ENCODING_PCM_16BIT, minBufSize * 8,
				AudioTrack.MODE_STREAM);
		// track.play();

	}

	public native void createEngine();

	public native void playStream(String stream);

	public native void shutdownEngine();

	public void streamCallback(byte[] data) {
		// TODO write to AudioTrack
		Log.d("Manager", "Received " + data.length + " byte");

		nBytesRead = data.length;

		if (nBytesRead >= 0) {
			int result = track.write(data, 0, data.length);
			if (result == AudioTrack.ERROR_INVALID_OPERATION || result != data.length) {
				Log.e(TAG, "Cannot write to AudioTrack");
				return;
			}
			overallBytes +=  result;

			Log.d("Manager", "bytesWritten " + overallBytes);

			if (!started && overallBytes > minBufSize*4) {
				
				track.play();
				started = true;
				Log.d("Manager", "Start playing!!! Yea");
			}
		}

	}

	private void playFromBuffer() {
		/*
		 * if (buffer.size() >= BUFFER_SIZE) {
		 * 
		 * for (int i=0; i<buffer.size();i++) { byte[] data = buffer.get(i);
		 * track.write(data, 0, data.length); buffer.remove(i); }
		 * 
		 * 
		 * }
		 */

	}

}
