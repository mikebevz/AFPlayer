#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <jni.h>
#include <android/log.h>

//Fix for 	'UINT64_C' was not declared in this scope error
//@see http://code.google.com/p/ffmpegsource/issues/detail?id=11
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

#define TAG "CCC"

extern "C" {

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>

}

#include "media_player.h"
#include "fplayer.h"


//jmethodID errorCallbackMethodId = NULL;
//jobject callbackObject = NULL;
/*
 * JNI Interface functions
 */
JavaVM *cachedVM;
//static const char * filename = NULL;
//static const char * stream_format = NULL;

jint JNI_OnLoad(JavaVM* jvm, void* reserved) {

	JNIEnv *env;
	cachedVM = jvm;
	__android_log_print(ANDROID_LOG_INFO, TAG, "JNI_OnLoad Called");

	if (jvm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
		__android_log_print(ANDROID_LOG_ERROR, TAG, "Failed to get the environment using GetEnv()");
		return -1;
	}

	return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL
JNI_OnUnload(JavaVM *jvm, void *reserved) {
	JNIEnv *env;
	if (jvm->GetEnv((void**) &env, JNI_VERSION_1_6)) {
		return;
	}
	//(*env)->DeleteWeakGlobalRef(env, );
	return;
}

/**
 * Get cached environment
 */
JNIEnv *JNU_Get_Env() {
	JNIEnv *env;
	cachedVM->GetEnv((void **) &env, JNI_VERSION_1_6);

	return env;
}

JavaVM *getJavaVM() {
	return cachedVM;
}


void debug_log(const char *msg, int code) {

	//TODO when code=0 it's not an error
	char errstr[200];
	int r = av_strerror(code, errstr, 200);

	__android_log_print(ANDROID_LOG_DEBUG, TAG, "%s (%d): %s", msg, code, errstr);

	/*
	if (errorCallbackMethodId != NULL) {
		JNIEnv *env;
		env = JNU_Get_Env();
		jstring jmesg = env->NewStringUTF(errstr);
		env->CallVoidMethod(callbackObject, errorCallbackMethodId, jmesg, code);
		env->DeleteLocalRef(jmesg);
	}*/

}

// Start fplayer implementation

fplayer::fplayer() :
        m_stoprequested(false) {
	engine_started = false;
	file_iformat = NULL;
	stream_format = NULL;

}

fplayer::~fplayer() {
	//pthread_mutex_destroy(&m_mutex);
}

void fplayer::play(const char* filename, JNIEnv *env, jobject obj, jmethodID callback) {

	stream_url = filename;
	stream_callback = callback;
	stream_object = obj;
	stream_env = env;

	m_stoprequested = false;
	do_play();
}

void fplayer::play(const char* filename, const char* format, JNIEnv *env, jobject obj, jmethodID callback) {

	stream_url = filename;
	stream_callback = callback;
	stream_object = obj;
	stream_env = env;
	stream_format = format;
	m_stoprequested = false;
	do_play();
}

int fplayer::setSetupMethod(jmethodID method) {
   this->stream_setup_callback = method;
   return 0;
}

int fplayer::start_engine() {
	if (engine_started == true) {
		return ERROR_ALREADY_STARTED;
	}
	AVInputFormat *p = NULL;
	//__android_log_print(ANDROID_LOG_DEBUG, TAG, "Setup FFMPEG Engine");

	avcodec_init();
	av_register_all();
	avcodec_register_all();
	avdevice_register_all();

	//Show all supported input formats in log
	while (p = av_iformat_next(p)) {
		//TODO return list of formats to mediaplayer
		__android_log_print(ANDROID_LOG_DEBUG, TAG, "Input format: %s", p->name);
	}

	URLProtocol *prot = NULL;

	while (prot = av_protocol_next(prot)) {
		//TODO return list of protocols supported
		__android_log_print(ANDROID_LOG_DEBUG, TAG, "Protocol: %s", prot->name);
	}
	//TODO Redirect log out to android log
	av_log_set_level(AV_LOG_VERBOSE);

	return 0;
}

int fplayer::shutdown_engine() {
	if (engine_started == false) {
		return ERROR_ALREADY_STOPPED;
	}

	return 0;
}

int fplayer::do_play() {
	/*
	 while (!m_stoprequested) {
	 pthread_mutex_lock(&m_mutex);
	 __android_log_print(ANDROID_LOG_DEBUG, TAG, "PLAY!!! File: %s", stream_url);
	 pthread_mutex_unlock(&m_mutex);
	 }*/
	//__android_log_print(ANDROID_LOG_DEBUG, TAG, "do_play()");

	int OUT_BUFFER_SIZE = AVCODEC_MAX_AUDIO_FRAME_SIZE * 8;
	char* samples = (char *) av_malloc(OUT_BUFFER_SIZE);

	if (stream_url == NULL) {
		debug_log("Stream URL is not defined", NULL);
		//__android_log_print(ANDROID_LOG_DEBUG, TAG, "do_play()");
		//pthread_exit(&m_thread);
		return ERROR_NO_STREAM_ADDRESS;
	}

	int status;

	 //const char format[] = "applehttp";
	if (stream_format != NULL) {
		 __android_log_print(ANDROID_LOG_DEBUG, TAG, "Start Stream");
		 __android_log_print(ANDROID_LOG_DEBUG, TAG, stream_url);

		 if (!(file_iformat = av_find_input_format(stream_format))) {
		 __android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot find format: %s", stream_format);
		 debug_log("Cannot find format", (int)file_iformat);
		 return ERROR_CANNOT_FIND_FORMAT;
		 }
	}

	__android_log_print(ANDROID_LOG_DEBUG, TAG, "avformat_open_input: %s", stream_url);

	pFormatCtx = NULL;

	status = avformat_open_input(&pFormatCtx, stream_url, file_iformat, options);
	if (status != 0) {

		debug_log("Cannot open stream.", status);
		//__android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot open stream. Status: %d", status);
		return ERROR_CANNOT_OPEN_STREAM;
	}

	//populates AVFormatContex structure
	status = av_find_stream_info(pFormatCtx);
	if (status != NULL && status < 0) {
		debug_log("Cannot read stream info.", status);
		return ERROR_CANNOT_READ_STREAM_INFO;
	}

	__android_log_print(ANDROID_LOG_DEBUG, TAG, "stream info");

	if (pFormatCtx->nb_streams != 1
			&& pFormatCtx->streams[0]->codec->codec_type != AVMEDIA_TYPE_AUDIO) {
		debug_log("Sanity check failed", status);
		return ERROR_STREAM_SANITY_CHECK_FAILED;
	}

	/*
	 for (int i = 0; i < pFormatCtx->nb_streams; ++i) {
	 __android_log_print(ANDROID_LOG_DEBUG, TAG, "Found stream ID: %d", pFormatCtx->streams[i]->id);
	 //av_dict_get(pFormatCtx->streams[i]->metadata, "", )
	 }*/

	// Find audio stream
	//TODO selects only first stream
	int audioStream;
	for (int i = 0; i < pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			audioStream = i;
			__android_log_print(ANDROID_LOG_DEBUG, TAG, "Stream ID: %d selected", pFormatCtx->streams[i]->id);

			break;
		}
	}

	codecCtx = pFormatCtx->streams[audioStream]->codec;

	if (codecCtx == NULL) {
		//__android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot get codec");
		debug_log("Cannot get codec", NULL);
		return ERROR_CANNOT_OBTAIN_CODEC;
	}

	if (pFormatCtx->streams[audioStream]->codec->codec_id == NULL) {
		//__android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot get Codec ID from stream");
		debug_log("Cannot get CodecID from stream", NULL);
		return ERROR_NO_CODEC_INFO;
	}

	codec = avcodec_find_decoder(
			pFormatCtx->streams[audioStream]->codec->codec_id);

	if (codec == NULL) {
		//__android_log_print(ANDROID_LOG_ERROR, TAG, "Unsupported codec: %s", codec->name);
		debug_log("Unsupported codec", NULL);
		return ERROR_CODEC_NOT_SUPPORTED;
	}

	status = avcodec_open(codecCtx, codec);
	if (status < 0) {
		//__android_log_print(ANDROID_LOG_ERROR, TAG, "Unsupported codec - avcodec_open: %s", codec->name);
		debug_log("Unsupported codec", status);
		return ERROR_CODEC_NOT_SUPPORTED;
	}

	av_init_packet(&avpkt);
	int flushPerSecond = 2;
	int outputBufferSize = -1;
	jbyteArray array;
	jbyte* outputBuffer;
	int outputBufferPos = 0;
	m_stoprequested = false;

	//__android_log_print(ANDROID_LOG_DEBUG, TAG, "before start read frame");
	int ret_status;

	__android_log_print(ANDROID_LOG_DEBUG, TAG, "Sample Rate: %d", codecCtx->sample_rate);

	int ret = stream_env->CallIntMethod(stream_object, stream_setup_callback, codecCtx->sample_rate);
	if (ret != 0) {
	   __android_log_print(ANDROID_LOG_DEBUG, TAG, "Cannot call setup method");
	}

	while ((ret_status = av_read_frame(pFormatCtx, &avpkt)) == 0) {

		if (codecCtx->codec_type == AVMEDIA_TYPE_AUDIO
				&& avpkt.stream_index == audioStream) {



			int frame_size = OUT_BUFFER_SIZE;
			int size = avpkt.size;
			if (size == 0) {
				__android_log_print(ANDROID_LOG_ERROR, TAG,
						"Packet size is 0: %d", size);
				debug_log("Packet size is 0", NULL);
				break;
			}

			if (m_stoprequested) {
				av_free_packet(&avpkt);
				break;
			}

			while (size > 0) {
				int len = avcodec_decode_audio3(codecCtx, (short *) samples,
						&frame_size, &avpkt);
				if (len < 0) {
					__android_log_print(ANDROID_LOG_ERROR, TAG,
							"Error while decoding. Status/len: %d. Size: %d",

							len, size);
					debug_log("Error while decoding.", NULL);
					break;
				}

				if (outputBufferSize == -1) {
					outputBufferSize = codecCtx->sample_rate * codecCtx->channels * 2 / flushPerSecond;
					__android_log_print(ANDROID_LOG_DEBUG, TAG, "Setting outputBufferSize: %d", outputBufferSize);
					array = stream_env->NewByteArray(outputBufferSize);
					outputBuffer = stream_env->GetByteArrayElements(array, NULL);
				}

				// Flush the buffer to Java if it's full
				if (outputBufferPos + frame_size > outputBufferSize) {
					__android_log_print(ANDROID_LOG_DEBUG, TAG, "outputBufferPos=%d. frame_size_ptr=%d. outputBufferSize=%d", outputBufferPos, frame_size, outputBufferSize);

					stream_env->ExceptionClear();
					int ret = stream_env->CallIntMethod(stream_object, stream_callback, array, outputBufferPos);

					if (ret == 1) { // STOP-signal
						m_stoprequested = true;
					}

					__android_log_print(ANDROID_LOG_ERROR, TAG, "Flushed buffer to java");
					outputBufferPos = 0;

					if (m_stoprequested) {
						break;
					}
				}

				memcpy((outputBuffer + outputBufferPos), (int16_t *) samples, frame_size);
				outputBufferPos += frame_size;
				size -= len;
			}

			av_free_packet(&avpkt);

		}

	}
	char errstr[200];
	int r = av_strerror(ret_status, errstr, 200);
	//TODO Process return status from read frame
	__android_log_print(ANDROID_LOG_DEBUG, TAG, "Returned status from frame read: %d %s (%d)", ret_status, errstr, r);
	debug_log("Stream playback stopped", ret_status);

	if (outputBufferSize != -1) {
		stream_env->ReleaseByteArrayElements(array, outputBuffer, NULL);
		stream_env->DeleteLocalRef(array);
		outputBufferSize = -1;
	}

	av_free(samples);
	avcodec_close(codecCtx);
	avformat_free_context(pFormatCtx);

	return 0;

}


fplayer player;



/**
 * Set up audio engine
 */
JNIEXPORT void JNICALL Java_org_fpl_media_MediaPlayer_n_1createEngine(
		JNIEnv *env, jobject obj, jobject mplayer) {

	__android_log_print(ANDROID_LOG_DEBUG, TAG, "Create Engine");

	//start_engine();

	//"(Ljava/lang/String;I)V"

	//callbackObject = obj;
	//jclass cls = env->GetObjectClass(obj);
	//errorCallbackMethodId = env->GetMethodID(cls, "streamErrorCallback", "(Ljava/lang/String;I)V");

	player.start_engine();
}

/**
 * Set data source
 */
JNIEXPORT void JNICALL Java_org_fpl_media_MediaPlayer_n_1setDataSource__Ljava_lang_String_2(
		JNIEnv *env, jobject obj, jstring path) {
   jboolean isCopy = JNI_TRUE;
	player.stream_url = env->GetStringUTFChars(path, &isCopy);
	__android_log_print(ANDROID_LOG_DEBUG, TAG, "Filename: %s", player.stream_url );

}


/**
 * Set data soruce with format
 */
JNIEXPORT void JNICALL Java_org_fpl_media_MediaPlayer_n_1setDataSource__Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv *env, jobject obj, jstring path, jstring format) {

        jboolean isCopy = JNI_TRUE;
        player.stream_url = JNU_Get_Env()->GetStringUTFChars(path, &isCopy);
	//filename = path;
	player.stream_format = JNU_Get_Env()->GetStringUTFChars(format, &isCopy);
	__android_log_print(ANDROID_LOG_DEBUG, TAG, "Filename: %s. StreamFormat: %s", player.stream_url , player.stream_format);

}

/**
 * Start playing stream back
 */
JNIEXPORT void JNICALL Java_org_fpl_media_MediaPlayer_n_1playStream(JNIEnv *env,
		jobject obj) {
	if (player.stream_url != 0) {
		//start_audio_stream(env, obj, player.stream_url );

		//const char *stream_path;
		const char *format;

__android_log_print(ANDROID_LOG_ERROR, TAG, "XXX1");
		//stream_path = filename;//(char*) env->GetStringUTFChars(player.stream_url , NULL);
__android_log_print(ANDROID_LOG_ERROR, TAG, "XXX2");
		format = player.stream_format;//(char*) env->GetStringUTFChars(stream_format, NULL);
__android_log_print(ANDROID_LOG_ERROR, TAG, "XXX3");

		jclass cls = env->GetObjectClass(obj);
		if (!cls) {
			__android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot get class");
		}

		jmethodID setupMethod = env->GetMethodID(cls, "streamSetupCallback", "(I)I");
		if (setupMethod != NULL) {
		   player.setSetupMethod(setupMethod);
		} else {
		   __android_log_print(ANDROID_LOG_ERROR, TAG,
		                  "Cannot get setup callback method");
		}

		jmethodID method = env->GetMethodID(cls, "streamCallback", "([BI)I");
		if (!method) {
			__android_log_print(ANDROID_LOG_ERROR, TAG,
					"Cannot get callback method");
		}
		if (player.stream_format != NULL) {
			player.play(player.stream_url, env, obj, method);
		} else {
			player.play(player.stream_url, format, env, obj, method);
		}

	} else {
		jclass excCls = env->FindClass("java/lang/IllegalArgumentException");
		if (excCls != 0) {
			env->ThrowNew(excCls, "Data Source is not specified");
		}
	}

}

/**
 * Stop playing the stream
 */
JNIEXPORT void JNICALL Java_org_fpl_media_MediaPlayer_n_1stopStream(JNIEnv *env,
		jobject obj) {
	__android_log_write(ANDROID_LOG_DEBUG, TAG, "Stop Playing Stream");
	//stop_audio_stream();
	//player.stop();
}

/**
 * Shutdown audio engine
 */
JNIEXPORT void JNICALL Java_org_fpl_ffmpeg_Manager_shutdownEngine(JNIEnv *env,
		jobject obj) {
	__android_log_write(ANDROID_LOG_DEBUG, TAG, "Shutdown engine");
	//shutdown_engine();
	player.shutdown_engine();
}

