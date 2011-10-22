package org.fpl;

import org.fpl.ffmpeg.Manager;

import android.app.Activity;
import android.os.Bundle;

public class FPlayerActivity extends Activity {
    
	
	
    private String shoutcast;
	private String rtsp;
	private Manager manager;

	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        shoutcast = "http://live-icy.gss.dr.dk:8000/Channel3_LQ.mp3";
        rtsp = "rtsp://live-rtsp.dr.dk/rtplive/_definst_/Channel3_LQ.stream";
        
        manager = new Manager();
        manager.createEngine();
        manager.playStream(shoutcast);
        
        
    }
	
	
	@Override
	protected void onStop() {
		manager.shutdownEngine();
		super.onStop();
		
	}
}