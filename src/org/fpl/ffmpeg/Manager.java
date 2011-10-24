package org.fpl.ffmpeg;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;

public class Manager {

	static {
		System.loadLibrary("player");
	}

	private AudioTrack track;

	public Manager() {

		int bufSize = AudioTrack.getMinBufferSize(44100,
				AudioFormat.CHANNEL_CONFIGURATION_STEREO,
				AudioFormat.ENCODING_PCM_16BIT);
		
		Log.d("Manager", "Buffer size: "+String.valueOf(bufSize));
		
		track = new AudioTrack(AudioManager.STREAM_MUSIC, 44100,
				AudioFormat.CHANNEL_CONFIGURATION_STEREO,
				AudioFormat.ENCODING_PCM_16BIT, bufSize, AudioTrack.MODE_STREAM);
		track.play();

	}

	public native void createEngine();

	public native void playStream(String stream);

	public native void shutdownEngine();

	public void streamCallback(byte[] data) {
		// TODO write to AudioTrack
		Log.d("Manager", "Fik " + data.length + " byte");
		track.write(data, 0, data.length);
	}

}
