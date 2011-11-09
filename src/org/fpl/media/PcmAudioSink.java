/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.fpl.media;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import dk.nordfalk.netradio.Log;
import java.util.ArrayList;
import java.util.LinkedList;

/**
 *
 * @author j
 */
public class PcmAudioSink {

	public static AudioTrack track;
  final int bytesPerSecond;
  final int preferredBufferInSeconds = 10;

  public LinkedList<byte[]> buffersInUse = new LinkedList<byte[]>();
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
    buffersInUse.add(buf);
    bytesInBuffer += length;
  }

  void startPlay() {
    track.play();
  }

  void write() {
    long start = System.currentTimeMillis();

    byte[] buff = buffersInUse.removeFirst();
    buffersNotInUse.add(buff);
    bytesInBuffer -= buff.length;

    result = track.write(buff, 0, buff.length);

    long slut = System.currentTimeMillis();
    Log.d("Wrote " + buff.length + " byte to AudioTrack in "+ (slut-start)+ " ms");
  }
}
