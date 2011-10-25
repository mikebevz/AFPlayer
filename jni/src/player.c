#include <android/log.h>
#include <jni.h>
#include "manager.h"
#include "fplayer.h"

#define TAG "Player.c"

JavaVM *cachedVM;

jint JNI_OnLoad(JavaVM* jvm, void* reserved) {

	JNIEnv *env;
	cachedVM = jvm;
	__android_log_print(ANDROID_LOG_INFO, TAG, "JNI_OnLoad Called");
	if((*jvm)->GetEnv(jvm, (void **) &env, JNI_VERSION_1_6) != JNI_OK) {
		__android_log_print(ANDROID_LOG_ERROR, TAG, "Failed to get the environment using GetEnv()");
		return -1;
	}

	return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL Java_org_fpl_ffmpeg_Manager_createEngine(JNIEnv *env,
		jobject obj) {

	__android_log_write(ANDROID_LOG_DEBUG, TAG, "Create Engine");
	start_engine();
}

JNIEXPORT void JNICALL Java_org_fpl_ffmpeg_Manager_playStream(JNIEnv *env,
		jobject obj, jstring filename) {
	start_audio_stream(env, obj, filename);
}

JNIEXPORT void JNICALL Java_org_fpl_ffmpeg_Manager_shutdownEngine(JNIEnv *env,
		jobject obj) {
	shutdown_engine();
}
