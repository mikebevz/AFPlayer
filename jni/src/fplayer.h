#ifndef FPLAYER
#define FPLAYER

#ifdef __cplusplus

#else

#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__cplusplus)

#include <android/log.h>

extern int start_engine();
extern int shutdown_engine();
extern int start_audio_stream(JNIEnv *env, jobject obj, jstring filename);
extern int stop_audio_stream();

class fplayer {

public:
	fplayer();
	~fplayer();
	void play(char* filename, JNIEnv *env, jobject obj, jmethodID callback);
	void stop();
	int start_engine();
	int shutdown_engine();



private:
	volatile bool m_stoprequested;
	volatile bool m_running;
	pthread_mutex_t m_mutex;
	pthread_t m_thread;
	bool engine_started;
	char *stream_url;
	_jmethodID *stream_callback;
	_jobject *stream_object;

	AVFormatContext *pFormatCtx;
	AVInputFormat *file_iformat;
	AVDictionary **options;
	AVCodecContext *codecCtx;
	AVCodec *codec;
	AVPacket avpkt;
	int OUT_BUFFER_SIZE;
	char* samples;
    JNIEnv *stream_env;

	//int rc;
	static void* start_thread(void *obj);
	void do_play();
};

#else

extern int start_engine();
extern int shutdown_engine();
extern int start_audio_stream(JNIEnv *env,
		jobject obj, jstring filename);
extern int stop_audio_stream();

#endif

#ifdef __cplusplus
}
#endif

#endif /*FPLAYER*/
