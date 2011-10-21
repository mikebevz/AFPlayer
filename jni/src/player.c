#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include "manager.h"

AVFormatContext *pFormatCtx;

void Java_org_fpl_ffmpeg_Manager_createEngine(JNIEnv *env, jclass clazz) {
	//av_register_all();
}

void Java_org_fpl_ffmpeg_Manager_playStream(JNIEnv *env, jclass clazz,
		jobject obj) {
	if (av_find_stream_info(pFormatCtx) < 0) {

	}

}

void Java_org_fpl_ffmpeg_Manager_shutdownEngine(JNIEnv *env, jclass clazz) {

}
