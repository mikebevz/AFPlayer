package org.fpl;

import org.fpl.ffmpeg.Manager;
import org.fpl.media.MediaPlayer;

import android.app.Activity;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class FPlayerActivity extends Activity implements OnClickListener {

	private static final String TAG = "FPlayer Activity";
	private String shoutcast;
	//private String rtsp;
	//private Manager manager;
	private String applehttp;

	//private String LOCK = "LOCK";
	private MediaPlayer mp;
	private Button playBtn;
	private Button stopBtn;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		shoutcast = "http://live-icy.gss.dr.dk:8000/Channel3_LQ.mp3";
		//rtsp = "rtsp://live-rtsp.dr.dk/rtplive/_definst_/Channel3_LQ.stream";
		applehttp = "http://live-http.gss.dr.dk/streaming/audio/Channel21/Channel21_LQ0.m3u8"; // "http://javabog.dk/privat/Channel21_LQ-20111031-110328-0025490.ts";

		playBtn = (Button) findViewById(R.id.playBtn);
		playBtn.setOnClickListener(this);

		stopBtn = (Button) findViewById(R.id.stopBtn);
		stopBtn.setOnClickListener(this);

		mp = MediaPlayer.create(this, Uri.parse(applehttp));

	}

	@Override
	protected void onDestroy() {

		if (mp.isPlaying()) {
			mp.stop();
		}

		if (mp != null) {
			mp.release();
		}

		super.onDestroy();

	}

	@Override
	public void onClick(View v) {

		int id = v.getId();
		switch (id) {
		case R.id.playBtn:
			Log.d(TAG, "Start stream");
			mp.start();
			break;

		case R.id.stopBtn:
			Log.d(TAG, "Stop stream");
			mp.stop();
			break;

		default:
			break;
		}

	}
}