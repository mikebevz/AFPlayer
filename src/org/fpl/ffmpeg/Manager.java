package org.fpl.ffmpeg;


public class Manager {
	
	static {
		System.loadLibrary("player");
	}
	
	public native void createEngine();
	public native void playStream(String stream);
	public native void shutdownEngine();
	
}
