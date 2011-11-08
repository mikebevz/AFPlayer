package org.fpl.media;

import android.os.Handler;
import java.lang.ref.WeakReference;


import android.content.Context;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.net.Uri;
import dk.nordfalk.netradio.Log;
import java.util.ArrayList;
import java.util.LinkedList;
//import android.util.Log;

public class MediaPlayer {

	private final static String TAG = "MediaPlayer";
	public static AudioTrack track;

	static {
		System.loadLibrary("player");
	}

	private boolean isPlaying;
	private boolean stopRequested;
  public boolean addBuzzTone = false;
  public Handler handler;
  private final int bytesPerSecond;
  private final int preferredBufferInSeconds = 10;

  public LinkedList<byte[]> buffersInUse = new LinkedList<byte[]>();
  public int bytesInBuffer = 0;
  public ArrayList<byte[]> buffersNotInUse = new ArrayList<byte[]>();

	public MediaPlayer() {
		Log.d(TAG, "Create new MediaPlayer");
    int sampleRateInHz = 44100;
    int channelConfig = AudioFormat.CHANNEL_CONFIGURATION_STEREO;
    int audioFormat = AudioFormat.ENCODING_PCM_16BIT;

    int minBufSize = AudioTrack.getMinBufferSize(sampleRateInHz, channelConfig, audioFormat);
    if (minBufSize<=0) throw new InternalError("Buffer size error: "+minBufSize);

    int bufferSize = 176400; // minBufSize * 8
    //int bufferSize = 86016;
		track = new AudioTrack(AudioManager.STREAM_MUSIC,
            sampleRateInHz, channelConfig, audioFormat,
            bufferSize, AudioTrack.MODE_STREAM);

    bytesPerSecond = track.getChannelCount() * track.getSampleRate() *
            (track.getAudioFormat() == AudioFormat.ENCODING_PCM_16BIT ? 2 : 1);

		Log.d("X "+bytesPerSecond+ "  "+track.getChannelCount()+ "  "+track.getSampleRate()+ "  "+track.getAudioFormat());

		Log.d("Manager", "Buffer size - min: " + minBufSize+ "  - ("+ minBufSize*1000/bytesPerSecond+" msecs)");
		Log.d("Manager", "Buffer size - act: " + bufferSize+ "  - ("+ bufferSize*1000/bytesPerSecond+" msecs)");


		n_createEngine(new WeakReference<MediaPlayer>(this));

	}

	/**
	 * Create a new instance of MediaPlayer to play stream back
	 *
	 * @param context Activity context
	 * @param uri URI of the media resource, fx, a stream
	 *
	 * @return MediaPlayer
	 */
	public static MediaPlayer create(Context context, Uri uri) {
		Log.d(TAG, "Create Stream");

		MediaPlayer mp = new MediaPlayer();
		mp.setDataSource(context, uri);
		mp.prepare();// Not needed yet. Is here for compatibility

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
	public void setDataSource(Context context, Uri uri)
			throws IllegalStateException {

		String scheme = uri.getScheme();
		if (scheme == null || scheme.equals("file")) {
			// TODO Implement file playback
			throw new IllegalArgumentException("File given "+uri);
		}

		n_setDataSource(uri.toString()); // Play path of stream
		return;

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
		//n_stopStream();
    if (isPlaying) track.stop();
		stopRequested = true;

	}

  /** Not needed. Is here for compatibility */
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

  public Runnable runWhenstreamCallbackStart;
  public Runnable runWhenstreamCallbackEnd;

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
    try {
      if (stopRequested) {
        isPlaying = false;
        return 1;
      }

      if (addBuzzTone) {
        for (int i=0; i<length; i+=151) data[i] += i%5*15;
      }

      byte[] buf = getFreeBuffer(length);
      System.arraycopy(data, 0, buf, 0, length);
      buffersInUse.add(buf);
      bytesInBuffer += length;

      if (!isPlaying()) {

        if (bytesInBuffer < preferredBufferInSeconds*bytesPerSecond) {
          return 0; // Not enough data, still bufferring
        }

        // Enough data - start playing
        track.play();
        setPlaying(true);
      }


      if (handler != null && runWhenstreamCallbackStart != null) {
        handler.post(runWhenstreamCallbackStart);
      }


      long start = System.currentTimeMillis();

      byte[] buff = buffersInUse.removeFirst();
      buffersNotInUse.add(buff);
      bytesInBuffer -= buff.length;

      int result = track.write(buff, 0, buff.length);

      long slut = System.currentTimeMillis();
      Log.d(TAG, "Wrote " + length + " byte to AudioTrack in "+ (slut-start)+ " ms");
      if (result == AudioTrack.ERROR_INVALID_OPERATION || result != length) {
        Log.e(TAG, "Cannot write to AudioTrack. Ret Code: " + result);
        return 1;
      }

      return 0;

    } finally {
      if (handler != null && runWhenstreamCallbackEnd != null) {
        handler.post(runWhenstreamCallbackEnd);
      }
    }
	}

  private byte[] getFreeBuffer(int length) {
    int n = buffersNotInUse.size();
    while (--n>0) {
      byte[] b = buffersNotInUse.get(n);
      if (b.length == length) {
        buffersNotInUse.remove(n);
        Log.d("getFreeBuffer genbruger "+b);
        return b;
      }
    }
    Log.d("getFreeBuffer OPRETTER NY");
    return new byte[length];
  }



}
