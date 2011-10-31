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

	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        shoutcast = "http://live-icy.gss.dr.dk:8000/Channel3_LQ.mp3";
        rtsp = "rtsp://live-rtsp.dr.dk/rtplive/_definst_/Channel3_LQ.stream";
        applehttp = "http://live-http.gss.dr.dk/streaming/audio/Channel21/Channel21_LQ0.m3u8"; //"http://javabog.dk/privat/Channel21_LQ-20111031-110328-0025490.ts";
        
        
        
        
        AsyncTask<Void, Void, Void> async = new AsyncTask<Void, Void, Void>() {

			@Override
			protected Void doInBackground(Void... params) {
				
				manager = new Manager();
		        manager.createEngine();
		        manager.playStream(applehttp);
				
				return null;
			}
		};
		
		async.execute();
        
    }
	
	
	@Override
	protected void onStop() {
		manager.shutdownEngine();
		super.onStop();
		
	}
}