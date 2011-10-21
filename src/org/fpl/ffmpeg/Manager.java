package org.fpl.ffmpeg;

import android.media.AudioTrack;

public class Manager {
	static {
		System.loadLibrary("player");
	}
	
	public static native void createEngine();
	public static native void playStream(AudioTrack track);
	public static native void shutdownEngine();
	
}
