#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

//Fix for 	'UINT64_C' was not declared in this scope error
//@see http://code.google.com/p/ffmpegsource/issues/detail?id=11
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

#define TAG "FPlayer.cpp"

extern "C" {
#include <android/log.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <jni.h>
#include <player.h>
#include <pthread.h>
}

#include "fplayer.h"

//#define INBUF_SIZE 4096






// Start fplayer implementation

fplayer::fplayer() :
		m_stoprequested(false), m_running(false) {
	pthread_mutex_init(&m_mutex, NULL);
	engine_started = false;
	OUT_BUFFER_SIZE = AVCODEC_MAX_AUDIO_FRAME_SIZE * 4;
	samples = (char *) av_malloc(OUT_BUFFER_SIZE);
}

fplayer::~fplayer() {
	pthread_mutex_destroy(&m_mutex);
}

void fplayer::play(char* filename, JNIEnv *env, jobject obj, jmethodID callback) {
	if (m_running != false) {
		return;
	}

	stream_url = filename;
	stream_callback = callback;
	stream_object = obj;
	stream_env = env;

	__android_log_print(ANDROID_LOG_DEBUG, TAG, "START THREAD");
	assert(m_running == false);
	m_running = true;
	m_stoprequested = false;
	pthread_create(&m_thread, NULL, &fplayer::start_thread, ((void**) this));
}

void fplayer::stop() {
	if (m_running != true) {
		return;
	}
	__android_log_print(ANDROID_LOG_DEBUG, TAG, "STOP THREAD");
	assert(m_running == true);
	m_running = false;
	m_stoprequested = true;
	pthread_join(m_thread, NULL);
}

int fplayer::start_engine() {
	if (engine_started == true) {
		return -1;
	}
	AVInputFormat *p = NULL;
	__android_log_print(ANDROID_LOG_DEBUG, TAG, "Setup FFMPEG Engine");

	av_register_all();
	avcodec_register_all();
	avdevice_register_all();

	//Show all supported input formats in log
	while (p = av_iformat_next(p)) {
		__android_log_print(ANDROID_LOG_DEBUG, TAG, p->name);
	}
	//TODO Redirect log out to android log
	av_log_set_level(AV_LOG_VERBOSE);

	return 0;
}

int fplayer::shutdown_engine() {
	if (engine_started == false) {
		return -1;
	}

	return 0;
}

// Private methods
void* fplayer::start_thread(void *obj) {
	reinterpret_cast<fplayer *>(obj)->do_play();
	return 0;
}

void fplayer::do_play() {
	/*
	 while (!m_stoprequested) {
	 pthread_mutex_lock(&m_mutex);
	 __android_log_print(ANDROID_LOG_DEBUG, TAG, "PLAY!!! File: %s", stream_url);
	 pthread_mutex_unlock(&m_mutex);
	 }*/

	if (stream_url == NULL) {
		__android_log_print(ANDROID_LOG_DEBUG, TAG,
				"Stream address is not defined");
		pthread_exit(&m_thread);
	}

	const char format[] = "applehttp";
	int status;

	__android_log_print(ANDROID_LOG_DEBUG, TAG, "Start Stream");
	__android_log_print(ANDROID_LOG_DEBUG, TAG, stream_url);

	if (!(file_iformat = av_find_input_format(format))) {
		__android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot find format: %s",
				format);
		pthread_exit(&m_thread);
	}

	status = avformat_open_input(&pFormatCtx, stream_url, file_iformat,
			options);
	//status = avio_open_dyn_buf()
	if (status != 0) {
		__android_log_print(ANDROID_LOG_ERROR, TAG,
				"Cannot open stream. Status: %d", status);
		pthread_exit(&m_thread);

	}

	//populates AVFormatContex structure
	status = av_find_stream_info(pFormatCtx);
	if (status < 0) {
		__android_log_print(ANDROID_LOG_ERROR, TAG,
				"Cannot read stream info. Status: %d", status);
		//return -1;
	}

	if (pFormatCtx->nb_streams != 1
			&& pFormatCtx->streams[0]->codec->codec_type != AVMEDIA_TYPE_AUDIO) {

		__android_log_print(ANDROID_LOG_ERROR, TAG, "Sanity check failed");
		//return -1;
	}

	// Find audio stream
	int audioStream;
	for (int i = 0; i < pFormatCtx->nb_streams; i++) {
		__android_log_print(ANDROID_LOG_DEBUG, TAG, "Codecs %d: %s", i,
				pFormatCtx->streams[i]->codec->codec_name);
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			audioStream = i;
			break;
		}
	}

	codecCtx = pFormatCtx->streams[audioStream]->codec;

	if (codecCtx == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot get codec");

		//return -1;
	}

	if (pFormatCtx->streams[audioStream]->codec->codec_id == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, TAG,
				"Cannot get Codec ID from stream");
		//return -1;
	}

	codec = avcodec_find_decoder(
			pFormatCtx->streams[audioStream]->codec->codec_id); //avcodec_find_decoder_by_name("mp3adufloat");

	if (codec == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, TAG, "Unsupported codec: %s",
				codec->name);
		//return -1;
	}

	status = avcodec_open(codecCtx, codec);
	if (status < 0) {
		__android_log_print(ANDROID_LOG_ERROR, TAG,
				"Unsupported codec - avcodec_open: %s", codec->name);
		//return -1;
	}

	av_init_packet(&avpkt);

	int flushPerSecond = 2;
	int outputBufferSize = -1;

	//__android_log_print(ANDROID_LOG_DEBUG, TAG, "OutBufferSize: %d    codecCtx->bits_per_raw_sample %d", outputBufferSize, codecCtx->bits_per_raw_sample);

	jbyteArray array;
	jbyte* outputBuffer;
	int outputBufferPos = 0;

	__android_log_print(ANDROID_LOG_DEBUG, TAG, "Start main loop");

	while (av_read_frame(pFormatCtx, &avpkt) >= 0) {
		//pthread_mutex_lock(&m_mutex);

		__android_log_print(ANDROID_LOG_DEBUG, TAG, "Entered loop");

		if (codecCtx->codec_type == AVMEDIA_TYPE_AUDIO) {
			int frame_size_ptr = OUT_BUFFER_SIZE;

			int size = avpkt.size;

			if (size == 0) {
				__android_log_print(ANDROID_LOG_ERROR, TAG,
						"Packet size is 0: %d", size);
			}

			__android_log_print(ANDROID_LOG_DEBUG, TAG, "before While");

			while (size > 0) {
				int len = avcodec_decode_audio3(codecCtx, (int16_t *) samples,
						&frame_size_ptr, &avpkt);
				if (len < 0) {
					__android_log_print(ANDROID_LOG_ERROR, TAG,
							"Error while decoding. Status/len: %d. Size: %d",
							len, size);
					break;
				}


				__android_log_print(ANDROID_LOG_DEBUG, TAG, "After decoding");

				if (outputBufferSize == -1) {
					outputBufferSize = codecCtx->sample_rate
							* codecCtx->channels * 2 / flushPerSecond;

					__android_log_print(ANDROID_LOG_DEBUG, TAG, "Setting outputBufferSize: %d", outputBufferSize);

					JavaVMAttachArgs args;

					getJavaVM()->AttachCurrentThread(&stream_env, &args);

					array = stream_env->NewByteArray(outputBufferSize);


					__android_log_print(ANDROID_LOG_DEBUG, TAG, "Getting new array");

					outputBuffer = stream_env->GetByteArrayElements(array,NULL);
					__android_log_print(
							ANDROID_LOG_DEBUG,
							TAG,
							"OutBufferSize: %d    codecCtx->bits_per_raw_sample %d",
							outputBufferSize, codecCtx->bits_per_raw_sample);
				}

				__android_log_print(ANDROID_LOG_DEBUG, TAG, "After setting outputBufferSize");

				// Flush the buffer to Java if it's full
				if (outputBufferPos + frame_size_ptr > outputBufferSize) {
					__android_log_print(ANDROID_LOG_ERROR, TAG,"outputBufferPos=%d. frame_size_ptr=%d. outputBufferSize=%d", outputBufferPos, frame_size_ptr, outputBufferSize);
					stream_env->ExceptionClear();
					stream_env->MonitorEnter(stream_object);
					stream_env->CallVoidMethod(stream_object,
							stream_callback, array, outputBufferPos);
					stream_env->MonitorExit(stream_object);

					__android_log_print(ANDROID_LOG_ERROR, TAG,
							"Flushed buffer to java");
					outputBufferPos = 0;

					if (m_stoprequested) {
						getJavaVM()->DetachCurrentThread();
						pthread_exit(&m_thread);
						//m_stoprequested = false;
					}
				}

				memcpy((outputBuffer + outputBufferPos), (int16_t *) samples,
						frame_size_ptr);
				outputBufferPos += frame_size_ptr;

				size -= len;

			}

			av_free_packet(&avpkt);
		}

		//stop_playback: __android_log_print(ANDROID_LOG_ERROR, TAG, "Playback stopped");
		//pthread_mutex_unlock(&m_mutex);
	}

	if (outputBufferSize != -1) {
		stream_env->ReleaseByteArrayElements(array, outputBuffer, NULL);
		stream_env->DeleteLocalRef(array);
		outputBufferSize = -1;
	}
	free(samples);
	avcodec_close(codecCtx);
	avformat_free_context(pFormatCtx);

}
// End fplayer implementation

fplayer player;

extern "C" int start_audio_stream(JNIEnv *env, jobject obj, jstring filename) {
	__android_log_print(ANDROID_LOG_DEBUG, TAG, "Start Stream");

	char *string;

	string = (char*) env->GetStringUTFChars(filename, NULL);

	jclass cls = env->GetObjectClass(obj);
	if (!cls) {
		__android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot get class");
		return -1;
	}

	jmethodID method = env->GetMethodID(cls, "streamCallback", "([BI)V");
	if (!method) {
		__android_log_print(ANDROID_LOG_ERROR, TAG,
				"Cannot get callback method");
		return -1;
	}

	player.play(string, env, obj, method);
	return 0;
}

extern "C" int stop_audio_stream() {
	__android_log_print(ANDROID_LOG_DEBUG, TAG, "Stop Stream");
	player.stop();
	return 0;
}

extern "C" int start_engine() {
	__android_log_print(ANDROID_LOG_DEBUG, TAG, "Start Engine");
	player.start_engine();
	return 0;
}

extern "C" int shutdown_engine() {
	__android_log_print(ANDROID_LOG_DEBUG, TAG, "Shutdown Engine");
	player.shutdown_engine();
	return 0;
}

/*
 extern "C" int start_engine() {
 AVInputFormat *p = NULL;
 __android_log_print(ANDROID_LOG_DEBUG, TAG, "Setup FFMPEG Engine");

 av_register_all();
 avcodec_register_all();
 avdevice_register_all();

 //Show all supported input formats in log
 while (p = av_iformat_next(p)) {
 __android_log_print(ANDROID_LOG_DEBUG, TAG, p->name);
 }
 //TODO Redirect log out to android log
 av_log_set_level(AV_LOG_VERBOSE);

 return 0;
 }

 extern "C" int shutdown_engine() {
 __android_log_print(ANDROID_LOG_DEBUG, TAG, "Shutdown Engine");

 return 0;
 }

 extern "C" int start_audio_stream(JNIEnv *env, jobject obj, jstring filename) {

 const char *url = env->GetStringUTFChars(filename, NULL);
 const char format[] = "applehttp";
 int status;

 __android_log_print(ANDROID_LOG_DEBUG, TAG, "Start Audio");
 __android_log_print(ANDROID_LOG_DEBUG, TAG, url);

 if (!(file_iformat = av_find_input_format(format))) {
 __android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot find format: %s",
 format);

 return -1;
 }

 status = avformat_open_input(&pFormatCtx, url, file_iformat, options);
 //status = avio_open_dyn_buf()
 if (status != 0) {
 __android_log_print(ANDROID_LOG_ERROR, TAG,
 "Cannot open stream. Status: %d", status);
 return -1;
 }

 //populates AVFormatContex structure
 status = av_find_stream_info(pFormatCtx);
 if (status < 0) {
 __android_log_print(ANDROID_LOG_ERROR, TAG,
 "Cannot read stream info. Status: %d", status);
 return -1;
 }

 if (pFormatCtx->nb_streams != 1
 && pFormatCtx->streams[0]->codec->codec_type != AVMEDIA_TYPE_AUDIO) {

 __android_log_print(ANDROID_LOG_ERROR, TAG, "Sanity check failed");
 return -1;
 }

 // Find audio stream
 int audioStream;
 for (int i = 0; i < pFormatCtx->nb_streams; i++) {
 __android_log_print(ANDROID_LOG_DEBUG, TAG, "Codecs %d: %s", i,
 pFormatCtx->streams[i]->codec->codec_name);
 if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
 audioStream = i;
 break;
 }
 }

 codecCtx = pFormatCtx->streams[audioStream]->codec;

 if (codecCtx == NULL) {
 __android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot get codec");

 return -1;
 }

 if (pFormatCtx->streams[audioStream]->codec->codec_id == NULL) {
 __android_log_print(ANDROID_LOG_ERROR, TAG,
 "Cannot get Codec ID from stream");
 return -1;
 }

 codec = avcodec_find_decoder(
 pFormatCtx->streams[audioStream]->codec->codec_id); //avcodec_find_decoder_by_name("mp3adufloat");

 if (codec == NULL) {
 __android_log_print(ANDROID_LOG_ERROR, TAG, "Unsupported codec: %s",
 codec->name);
 return -1;
 }

 status = avcodec_open(codecCtx, codec);
 if (status < 0) {
 __android_log_print(ANDROID_LOG_ERROR, TAG,
 "Unsupported codec - avcodec_open: %s", codec->name);
 return -1;
 }

 jclass cls = (JNU_Get_Env())->GetObjectClass(obj);
 if (!cls) {
 __android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot get class");
 return -1;
 }

 jmethodID method = (JNU_Get_Env())->GetMethodID(cls, "streamCallback",
 "([BI)V");
 if (!method) {
 __android_log_print(ANDROID_LOG_ERROR, TAG,
 "Cannot get callback method");
 return -1;
 }

 av_init_packet(&avpkt);

 int flushPerSecond = 2;
 int outputBufferSize = -1;

 __android_log_print(ANDROID_LOG_DEBUG, TAG,
 "OutBufferSize: %d    codecCtx->bits_per_raw_sample %d",
 outputBufferSize, codecCtx->bits_per_raw_sample);

 jbyteArray array;
 jbyte* outputBuffer;
 int outputBufferPos = 0;

 while (av_read_frame(pFormatCtx, &avpkt) >= 0) {

 if (codecCtx->codec_type == AVMEDIA_TYPE_AUDIO) {
 int frame_size_ptr = OUT_BUFFER_SIZE;

 int size = avpkt.size;

 if (size == 0) {
 __android_log_print(ANDROID_LOG_ERROR, TAG,
 "Packet size is 0: %d", size);
 }

 if (playing == false) {
 //goto stop_playback;
 __android_log_print(ANDROID_LOG_ERROR, TAG, "Playback stopped");
 playing = true;
 break;
 }

 while (size > 0) {

 int len = avcodec_decode_audio3(codecCtx, (int16_t *) samples,
 &frame_size_ptr, &avpkt);

 if (len < 0) {
 __android_log_print(ANDROID_LOG_ERROR, TAG,
 "Error while decoding. Status/len: %d. Size: %d",
 len, size);
 break;
 }

 if (outputBufferSize == -1) {
 outputBufferSize = codecCtx->sample_rate
 * codecCtx->channels * 2 / flushPerSecond;
 array = env->NewByteArray(outputBufferSize);
 outputBuffer = env->GetByteArrayElements(array, NULL);
 __android_log_print(
 ANDROID_LOG_DEBUG,
 TAG,
 "OutBufferSize: %d    codecCtx->bits_per_raw_sample %d",
 outputBufferSize, codecCtx->bits_per_raw_sample);
 }

 //__android_log_print(ANDROID_LOG_ERROR, TAG,"outputBufferPos=%d + frame_size_ptr=%d >? outputBufferSize=%d", outputBufferPos, frame_size_ptr, outputBufferSize);

 // Flush the buffer to Java if it's full
 if (outputBufferPos + frame_size_ptr > outputBufferSize) {
 __android_log_print(
 ANDROID_LOG_ERROR,
 TAG,
 "outputBufferPos=%d. frame_size_ptr=%d. outputBufferSize=%d",
 outputBufferPos, frame_size_ptr, outputBufferSize);
 env->CallVoidMethod(obj, method, array, outputBufferPos);
 __android_log_print(ANDROID_LOG_ERROR, TAG,
 "Flushed buffer to java");
 outputBufferPos = 0;
 }

 memcpy((outputBuffer + outputBufferPos), (int16_t *) samples,
 frame_size_ptr);
 outputBufferPos += frame_size_ptr;

 size -= len;

 }

 av_free_packet(&avpkt);
 }

 //stop_playback: __android_log_print(ANDROID_LOG_ERROR, TAG, "Playback stopped");

 }

 if (outputBufferSize != -1) {
 env->ReleaseByteArrayElements(array, outputBuffer, NULL);
 env->DeleteLocalRef(array);
 outputBufferSize = -1;
 }
 free(samples);
 avcodec_close(codecCtx);
 avformat_free_context(pFormatCtx);

 return 0;
 }

 int stop_audio_stream() {

 if (playing) {
 playing = false;
 }
 __android_log_print(ANDROID_LOG_DEBUG, TAG, "Stop playing the stream");

 }
 */
