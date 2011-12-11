#ifndef FPLAYER
#define FPLAYER


#ifdef __cplusplus
extern "C" {

#include <android/log.h>

extern int start_engine();
extern int shutdown_engine();
extern int start_audio_stream(JNIEnv *env, jobject obj, jstring filename);
extern int stop_audio_stream();

class MikesFfmpegPlayer {

public:
	MikesFfmpegPlayer();
	~MikesFfmpegPlayer();
	int stop();
	int start_engine();
	int shutdown_engine();
	int setSetupMethod(jmethodID method);

	/**
	 * Audio stream URI
	 */
	const char *stream_url;

	/**
	 * Audio stream format
	 */
	const char *stream_format;


	/**
	 * TODO Consider if these are needed
	 */
	_jmethodID *stream_callback;
	_jmethodID *stream_setup_callback;
	_jobject *stream_object;
	JNIEnv *stream_env;

	/**
	 * Perform stream playback
	 */
	int do_play();

private:

	/**
	 * Is stream stop was requested
	 */
	volatile bool m_stoprequested;

	/**
	 * Is stream thread running
	 */
	// only for async thread: volatile bool m_running;


	//pthread_mutex_t m_mutex;

	/**
	 * Stream playback thread id
	 */
	//pthread_t m_thread;

	/**
	 * Is the audio engine was already initialized
	 */
	bool engine_started;





    AVFormatContext *pFormatCtx;
    AVInputFormat *file_iformat;
    AVDictionary **options;
    AVCodecContext *codecCtx;
    AVCodec *codec;
    AVPacket avpkt;

    static const int ERROR_NO_STREAM_ADDRESS = -1020;
    /**
     * Playback has been already started
     */
    static const int ERROR_ALREADY_STARTED = -1021;
    /**
     * Playback has been already stopped
     */
    static const int ERROR_ALREADY_STOPPED = -1022;
    /**
     * Cannot open stream
     */
    static const int ERROR_CANNOT_OPEN_STREAM = -1023;
    /**
     * Cannot read stream info
     */
    static const int ERROR_CANNOT_READ_STREAM_INFO = -1024;
    /**
     * Stream sanity check failed
     */
    static const int ERROR_STREAM_SANITY_CHECK_FAILED = -1025;
    /**
     * Cannot obtain codec
     */
    static const int ERROR_CANNOT_OBTAIN_CODEC = -1026;
    /**
     * Stream doesn't include codec info
     */
    static const int ERROR_NO_CODEC_INFO = -1027;

    /**
     * Current codec is not supported
     */
    static const int ERROR_CODEC_NOT_SUPPORTED = -1028;
    /**
     * Cannot find stream format
     */
    static const int ERROR_CANNOT_FIND_FORMAT = -1029;


};

}
#endif

#endif /*FPLAYER*/
