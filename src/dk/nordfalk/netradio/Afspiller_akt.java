/**
 * This file is part Mikes FFMpeg Mediaplayer for Android
 *
 * Mikes FFMpeg Mediaplayer for Android is free software: you can redistribute it and/or modify it under the terms of the GNU
 * General Public License version 2 as published by the Free Software Foundation.
 *
 * Mikes FFMpeg Mediaplayer for Android is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along with Sveriges Radio Play for Android. If not,
 * see <http://www.gnu.org/licenses/>.
 */
package dk.nordfalk.netradio;

import java.util.ArrayList;

import org.fpl.media.MediaPlayer;
import org.fpl.media.PcmAudioSink;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.media.AudioTrack.OnPlaybackPositionUpdateListener;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.Spinner;
import android.widget.TableLayout;
import android.widget.TextView;
import android.widget.Toast;

/**
 *
 * @author j
 */
public class Afspiller_akt extends Activity implements OnClickListener {

   private Button                    startStopKnap;
   private TextView                  tv;
   // private Button sendKnap;
   Log                               log                     = new Log();
   private CheckBox                  addBuzzTone;
   // private Spinner kanalSpinner;
   private String[][]                kanaler                 = {
         { "Dubstep Radio 1 mp3 80", "http://178.32.253.144:8026" },
         { "Dubstep Radio 2 mp3 196", "http://lemon.citrus3.com:8062" },
         { "EO Muzaiko", "http://listen.radionomy.com/muzaikoinfo" },
         { "EO krokoloko", "http://esperanto-radio.com/krokolokonun" },
         { "EO radiovatikana", "http://esperanto-radio.com/radiovatikananun" },
         { "Sverige P3 rtsp", "rtsp://mobil-live.sr.se/mobilradio/kanaler/p3-aac-96" },
         { "DR P1 HLS", "http://ahls.gss.dr.dk/A/A03L.stream/Playlist.m3u8" },
         { "DR P2 HLS", "http://ahls.gss.dr.dk/A/A04L.stream/Playlist.m3u8" },
         { "DR P3 Icy/mp3", "http://live-icy.gss.dr.dk:8000/Channel5_LQ.mp3", "mp3" },
         { "DR P3 RTSP", "rtsp://artsp.gss.dr.dk/A/A03L.stream" },
         { "DR P3 HLS", "http://ahls.gss.dr.dk/A/A05L.stream/Playlist.m3u8" },
/*
<option class="v2" value='http://esperanto-radio.com/cri'>&#264;ina Radio Internacia</option>
<option class="v2" value='http://esperanto-radio.com/kaliningrada'>Kaliningrada E-radio</option>
<option class="v2" value='http://esperanto-radio.com/3zzzradio'>3ZZZ en Esperanto</option>
<option class="v2" value='http://esperanto-radio.com/krokoloko'>Krokoloko</option>
<option class="v2" value='http://esperanto-radio.com/muzaiko'>Muzaiko</option>
<option class="v2" value='http://esperanto-radio.com/parolumondo'>Parolu Mondo</option>
<option class="v2" value='http://esperanto-radio.com/peranto'>Peranto</option>
<option class="v2" value='http://esperanto-radio.com/polaretradio'>Pola Retradio</option>
<option class="v2" value='http://esperanto-radio.com/radioaktiva'>Radio Aktiva Urugvajo</option>
<option class="v2" value='http://esperanto-radio.com/radiokubo'>Radio Havano Kubo</option>
<option class="v2" value='http://esperanto-radio.com/radiovatikana'>Radio Vatikana</option>
<option class="v2" value='http://esperanto-radio.com/radioverda'>Radio Verda</option>
<option class="v2" value='http://esperanto-radio.com/varsoviavento'>Varsovia Vento</option>
<option class="v2" value='http://esperanto-radio.com/verdastacio'>Verda Stacio</option>
<option class="v2" value='http://esperanto-radio.com/vinilkosmo'>Vinilkosmo</option>
<option class="v2" value='http://esperanto-radio.com/vej'>Voĉoj el Japanio</option>
          */
                                                             };
   // private int[] afspilningskvalitet = new int[kanaler.length];
   String[]                          afspilningskvalitetNavn = { "-", "Godt", "Afbrydelser", "Virker ikke!" };

   private TextView                  statusTv;
   private TextView                  bufferstr;
   private Spinner                   kanalSpinner;
   // private boolean spiller;
   private String                    url;
   private MediaPlayer               mp;
   private Handler                   handler = new Handler();
   private CheckBox                  scrollCb;
   private WakeLock                  holdTelefonVågen;
   private ConnectivityManager       cm;
   private android.media.MediaPlayer androidMp;

   /** Called when the activity is first created. */
   @Override
   public void onCreate(Bundle savedInstanceState) {
      super.onCreate(savedInstanceState);

      // Volumen op/ned skal styre lydstyrken af medieafspilleren, uanset som noget spilles lige nu eller ej
      setVolumeControlStream(AudioManager.STREAM_MUSIC);

      if (savedInstanceState == null) {
         // skru op til 1/4 styrke hvis volumen er lavere end det
         AudioManager audioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
         int max = audioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
         int nu = audioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
         if (nu < 1 * max / 4) {
            audioManager.setStreamVolume(AudioManager.STREAM_MUSIC, 1 * max / 4, AudioManager.FLAG_SHOW_UI);
         }
      }

      PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
      holdTelefonVågen = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "afspiller");
      // holdTelefonVågen = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK, "afspiller");

      cm = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);

      tv = new TextView(this);
      tv.setId(100701);
      Log.systemOutToTextView(tv);

      TableLayout tl = new TableLayout(this);

      ArrayList<String> elem = new ArrayList<String>();
      for (String[] e : kanaler) {
         elem.add(e[0]);
      }

      kanalSpinner = new Spinner(this);
      kanalSpinner.setId(1008);
      kanalSpinner.setAdapter(new ArrayAdapter<String>(this, android.R.layout.simple_spinner_dropdown_item, android.R.id.text1, elem
            .toArray(new String[0])));
      kanalSpinner.setSelection(elem.size() - 1);
      tl.addView(kanalSpinner);
      // række.addView(kanalSpinner);

      LinearLayout række = new LinearLayout(this);
      scrollCb = new CheckBox(this);
      scrollCb.setText("Scroll");
      scrollCb.setChecked(Log.scroll_tv_til_bund);
      scrollCb.setOnClickListener(new OnClickListener() {
         public void onClick(View arg0) {
            Log.scroll_tv_til_bund = scrollCb.isChecked();
         }
      });
      scrollCb.setId(10012);
      række.addView(scrollCb);

      addBuzzTone = new CheckBox(this);
      addBuzzTone.setText("addBuzzTone");
      addBuzzTone.setOnClickListener(new OnClickListener() {
         public void onClick(View arg0) {
            mp.addBuzzTone = addBuzzTone.isChecked();
         }
      });
      addBuzzTone.setId(10012);
      række.addView(addBuzzTone);

      bufferstr = new TextView(this);
      bufferstr.setText("bufferstr");
      række.addView(bufferstr);

      tl.addView(række);

      række = new LinearLayout(this);

      /*
       * sendKnap = new Button(this); sendKnap.setText("Send\nrapport"); sendKnap.setOnClickListener(this);
       * række.addView(sendKnap);
       */

      statusTv = new TextView(this);
      række.addView(statusTv);
      ((LinearLayout.LayoutParams) statusTv.getLayoutParams()).weight = 1;

      tl.addView(række);

      startStopKnap = new Button(this);
      startStopKnap.setId(1010);
      tl.addView(startStopKnap);
      startStopKnap.setText("Spil");
      startStopKnap.setOnClickListener(this);

      ScrollView sv = new ScrollView(this);
      sv.setId(10011);
      sv.addView(tv);
      tl.addView(sv);
      setContentView(tl);

      onClick(null);
   }

   void åbnSendEpost(String emne, String txt) {
      Intent postIntent = new Intent(android.content.Intent.ACTION_SEND);
      postIntent.setType("plain/text");
      postIntent.putExtra(Intent.EXTRA_EMAIL, new String[] { "jacob.nordfalk@gmail.com" });
      postIntent.putExtra(Intent.EXTRA_CC, new String[] { "MIKP@dr.dk", "pappons@gmail.com" });
      postIntent.putExtra(Intent.EXTRA_SUBJECT, emne);
      postIntent.putExtra(Intent.EXTRA_TEXT, txt);
      startActivity(Intent.createChooser(postIntent, "Send mail..."));
   }

  long tid_brugerTrykketStart, tid_førsteData, tid_spiller;

   public void onClick(View v) {
      try {
         if (mp == null) {
            int kanalNr = kanalSpinner.getSelectedItemPosition();
            String navn = kanaler[kanalNr][0];
            String format = null;
            if (kanaler[kanalNr].length > 2) {
               format = kanaler[kanalNr][2];
            }
            url = kanaler[kanalNr][1];
            Log.d("Afspiller " + navn + " med URL:\n" + url);
            // Toast.makeText(Afspiller_akt.this, "Spiller " + navn, Toast.LENGTH_LONG).show();
            // Toast.makeText(Afspiller_akt.this, "Lad den køre 2 minutter før du bedømmer den",
            // Toast.LENGTH_LONG).show();

            visStatus(url);
            if (format != null) {
               mp = MediaPlayer.create(Uri.parse(url), format);
            } else {
               mp = MediaPlayer.create(Uri.parse(url));
            }

            tid_førsteData = tid_spiller = 0;
            tid_brugerTrykketStart = System.currentTimeMillis();

            mp.sink.setHandler(handler);
            mp.setRunWhenstreamCallback(new Runnable() {
               public void run() {
                  statusTv.setBackgroundColor(Color.RED);
                  if (mp == null)
                     return;
                  bufferstr.setText("Buffer:\n" + mp.sink.bufferInSecs() + " sek");
                  if (tid_førsteData == 0) tid_førsteData = System.currentTimeMillis();
               }
            });
            mp.sink.setRunWhenPcmAudioSinkWrite(new Runnable() {
               public void run() {
                  statusTv.setBackgroundColor(Color.BLACK);
                  if (mp == null)
                     return;
                  bufferstr.setText("Buffer:\n" + mp.sink.bufferInSecs() + " sek");
                  if (tid_spiller == 0) {
                    tid_spiller = System.currentTimeMillis();
                    String txt = "hvor lang tid der gik fra start til der faktisk kom lyd:"+
                            "\nTid start->data: "+(tid_førsteData - tid_brugerTrykketStart)/1000f+
                            "\nTid start->lyd:  "+(tid_spiller - tid_brugerTrykketStart)/1000f;

                    AlertDialog.Builder dialog=new AlertDialog.Builder(Afspiller_akt.this);
                    //dialog.setTitle("En AlertDialog");
                    dialog.setMessage(txt);
                    dialog.setPositiveButton("OK", null);
                    dialog.show();

                    //Toast.makeText(Afspiller_akt.this, txt, Toast.LENGTH_LONG).show();
                     }
                   }
            });
            mp.sink.getTrack().setPlaybackPositionUpdateListener(new OnPlaybackPositionUpdateListener() {
               public void onMarkerReached(AudioTrack arg0) {
                  Log.d("XXXXXXXX onMarkerReached " + arg0.getPlaybackHeadPosition());
               }

               public void onPeriodicNotification(AudioTrack arg0) {
                  Log.d("XXXXXX onPeriodicNotification ");
               }
            }, handler);
            Log.d("XXXXXX .setPlaybackPositionUpdateListener ");
            mp.start();

            if (holdTelefonVågen != null)
               holdTelefonVågen.acquire();
            // cm.startUsingNetworkFeature(ConnectivityManager.TYPE_MOBILE, null);
            cm.startUsingNetworkFeature(ConnectivityManager.TYPE_WIFI, null);
            startStopKnap.setText("Stop");
         } else {
            mp.stop();
            if (holdTelefonVågen != null)
               holdTelefonVågen.release();
            // cm.stopUsingNetworkFeature(ConnectivityManager.TYPE_MOBILE, null);
            cm.stopUsingNetworkFeature(ConnectivityManager.TYPE_WIFI, null);
            startStopKnap.setText("Play");
            mp.setRunWhenstreamCallback(null);
            mp.sink.setRunWhenPcmAudioSinkWrite(null);
            mp = null;
            if (androidMp != null)
               androidMp.release();
         }
      } catch (Exception e) {
         Log.e(e);
      }
   }

   @Override
   protected void onDestroy() {
      Log.d("onDestroy");
      super.onDestroy();
   }

   private void visStatus(String txt) {
      statusTv.setText(txt);
      Log.d(txt);
   }
}
