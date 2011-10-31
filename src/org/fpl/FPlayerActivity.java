package org.fpl;

import org.fpl.ffmpeg.Manager;

import android.app.Activity;
import android.os.AsyncTask;
import android.os.Bundle;

public class FPlayerActivity extends Activity {
    
	
	
    private String shoutcast;
	private String rtsp;
	private Manager manager;
	private String applehttp;
	
	private String LOCK = "LOCK";

	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        shoutcast = "http://live-icy.gss.dr.dk:8000/Channel3_LQ.mp3";
        rtsp = "rtsp://live-rtsp.dr.dk/rtplive/_definst_/Channel3_LQ.stream";
        applehttp = "http://live-http.gss.dr.dk/streaming/audio/Channel21/Channel21_LQ0.m3u8"; //"http://javabog.dk/privat/Channel21_LQ-20111031-110328-0025490.ts";
        
        
        
        
        Thread t = new Thread() {

			@Override
			public void run() {				
				synchronized (LOCK) {

					manager = new Manager();
			        manager.createEngine();
			        LOCK.notifyAll();
				}
		        manager.playStream(applehttp);				
			}
		};
		
		synchronized (LOCK) {
			t.start();
			try {
				LOCK.wait();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}			
		}
        
    }
	
	
	@Override
	protected void onDestroy() {
		manager.shutdownEngine();
		super.onDestroy();
		
	}
}