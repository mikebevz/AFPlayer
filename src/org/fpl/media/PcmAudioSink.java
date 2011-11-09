/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.fpl.media;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Handler;
import dk.nordfalk.netradio.Log;
import java.util.ArrayList;
import java.util.concurrent.LinkedBlockingQueue;

/**
 *
 * @author j
 */
public class PcmAudioSink {

  public Handler handler;
  public Runnable runWhenPcmAudioSinkWrite;

	public static AudioTrack track;
  final int bytesPerSecond;
  final int preferredBufferInSeconds = 5;

  public LinkedBlockingQueue<byte[]> buffersInUse = new LinkedBlockingQueue<byte[]>();
  public int bytesInBuffer = 0;
  public ArrayList<byte[]> buffersNotInUse = new ArrayList<byte[]>();
  int result;

  public PcmAudioSink() {
		Log.d("Create new PcmAudioSink");
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

  }

  byte[] getFreeBuffer(int length) {
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

  void putData(byte[] data, int length) {
    byte[] buf = getFreeBuffer(length);
    System.arraycopy(data, 0, buf, 0, length);
    try {
      //Virker ikke: boolean taken = buffersInUse.offer(buf, 1000, TimeUnit.MILLISECONDS);
      boolean taken = buffersInUse.offer(buf);
      if (!taken) throw new IllegalStateException(" not taken ??!?");
    } catch (Exception ex) {
      Log.e(ex);
      result = -1;
    }
    bytesInBuffer += length;
  }

  void startPlay() {
    track.play();
    Runnable r = new Runnable() {
      public void run() {
        while (!stop) try {
          write();
        } catch (InterruptedException ex) {
          Log.e(ex);
          result = -1;
        }
      }
    };
    new Thread(r).start();
  }

  private void write() throws InterruptedException {
    long start = System.currentTimeMillis();

    byte[] buff = buffersInUse.take();
    result = track.write(buff, 0, buff.length);
    long slut = System.currentTimeMillis();
    Log.d("Wrote " + buff.length + " byte to AudioTrack in "+ (slut-start)+ " ms");

    buffersNotInUse.add(buff);
    bytesInBuffer -= buff.length;

    if (handler != null && runWhenPcmAudioSinkWrite != null) {
      handler.post(runWhenPcmAudioSinkWrite);
    }
  }

  boolean stop = false;
  void stopPlay() {
    stop = true;
    track.release();
  }

  public String bufferInSecs() {
    return Float.toString((10*bytesInBuffer/bytesPerSecond)/10f);

  }
}
