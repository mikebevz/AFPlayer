package org.fpl;

import org.fpl.ffmpeg.Manager;

import android.app.Activity;
import android.os.Bundle;

public class FPlayerActivity extends Activity {
    
	
	
    private String shoutcast;
	private String rtsp;

	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        shoutcast = "http://live-icy.gss.dr.dk:8000/Channel3_LQ.mp3";
        rtsp = "rtsp://live-rtsp.dr.dk/rtplive/_definst_/Channel3_LQ.stream";
        
        //Manager manager = new Manager();
        Manager.createEngine();
        
        
        
    }
}