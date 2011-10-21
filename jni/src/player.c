#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include "manager.h"

AVFormatContext *pFormatCtx;

JNIEXPORT void JNICALL Java_org_fpl_ffmpeg_Manager_createEngine(JNIEnv *env, jclass clazz) {
	av_register_all();
}

JNIEXPORT void JNICALL Java_org_fpl_ffmpeg_Manager_playStream(JNIEnv *env, jclass clazz,
		jobject obj) {
	if (av_find_stream_info(pFormatCtx) < 0) {

	}

}

JNIEXPORT void JNICALL Java_org_fpl_ffmpeg_Manager_shutdownEngine(JNIEnv *env, jclass clazz) {

}
