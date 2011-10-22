#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fplayer.h"

#define TAG "FPlayer.cpp"

//Fix for 	'UINT64_C' was not declared in this scope error
//@see http://code.google.com/p/ffmpegsource/issues/detail?id=11
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

extern "C" {
#include <android/log.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
//#include <libavformat/avio.h>
#include <libavdevice/avdevice.h>
}

//FPlayer::FPlayer() {
//	av_register_all();
//};

static AVFormatContext *pFormatCtx;
static AVInputFormat *file_iformat;
AVFormatParameters params;
//AVIOContext *avioContext;

extern "C" int start_engine() {
	AVInputFormat *p = NULL;

	__android_log_print(ANDROID_LOG_DEBUG, TAG, "Start Engine");

	av_register_all();
	avcodec_register_all();
	avdevice_register_all();

	while (p = av_iformat_next(p)) {
		//printf("%s: %s:\n", p->name, p->long_name);
		__android_log_print(ANDROID_LOG_DEBUG, TAG, p->name);
	}

	av_log_set_level(AV_LOG_VERBOSE);

	return 0;
}

extern "C" int shutdown_engine() {
	__android_log_print(ANDROID_LOG_DEBUG, TAG, "Shutdown Engine");

	return 0;
}

extern "C" int start_audio_stream(char* filename) {
	const char url[] = "http://live-icy.gss.dr.dk:8000/Channel3_LQ.mp3";
	const char format[] = "mp3";

	__android_log_print(ANDROID_LOG_DEBUG, TAG, "Start Audio");
	__android_log_print(ANDROID_LOG_DEBUG, TAG, url);


	if (!(file_iformat = av_find_input_format(format))) {
		__android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot find mp3");
		return -1;
	}

	params.prealloced_context = 0;

	if (av_open_input_file(&pFormatCtx, url, file_iformat, 0, &params) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot open stream");
		return -1;
	}

	/* poulates AVFormatContex structure */
	if (av_find_stream_info(pFormatCtx) < 0) {
		__android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot read stream info");
		return -1;
	}

	if (pFormatCtx->nb_streams != 1
			&& pFormatCtx->streams[0]->codec->codec_type != AVMEDIA_TYPE_AUDIO) {
		__android_log_print(ANDROID_LOG_ERROR, TAG, "Sanity check failed");
		return -1;
	}

	//if (avio_open(&avioContext, filename, AVIO_FLAG_READ) != 0) {
	//	__android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot open stream");
	//	return -1;
	//}

	// Retrieve stream information

	//if (av_find_stream_info(pFormatCtx) < 0) {
	//	__android_log_print(ANDROID_LOG_ERROR, TAG,
	//			"Couldn't find stream information");
	//	return -1;
	//}

	// Dump information about file onto standard error
	//dump_format(pFormatCtx, 0, filename, 0);

	return 0;
}
