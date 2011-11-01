#include <android/log.h>
#include <jni.h>
#include "media_player.h"
#include "fplayer.h"
#include "player.h"

#define TAG "Player.c"

JavaVM *cachedVM;
jstring filename;

jint JNI_OnLoad(JavaVM* jvm, void* reserved) {

	JNIEnv *env;
	cachedVM = jvm;
	__android_log_print(ANDROID_LOG_INFO, TAG, "JNI_OnLoad Called");
	if ((*jvm)->GetEnv(jvm, (void **) &env, JNI_VERSION_1_6) != JNI_OK) {
		__android_log_print(ANDROID_LOG_ERROR, TAG,
				"Failed to get the environment using GetEnv()");
		return -1;
	}

	return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL
JNI_OnUnload(JavaVM *jvm, void *reserved) {
	JNIEnv *env;
	if ((*jvm)->GetEnv(jvm, (void**) &env, JNI_VERSION_1_6)) {
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
	(*cachedVM)->GetEnv(cachedVM, (void **) &env, JNI_VERSION_1_6);

	return env;
}

JavaVM *getJavaVM() {
	return cachedVM;
}

/**
 * Set up audio engine
 */
JNIEXPORT void JNICALL Java_org_fpl_media_MediaPlayer_n_1createEngine(
		JNIEnv *env, jobject obj, jobject mplayer) {

	__android_log_print(ANDROID_LOG_DEBUG, TAG, "Create Engine");
	start_engine();
}

/**
 * Set data source
 */
JNIEXPORT void JNICALL Java_org_fpl_media_MediaPlayer_n_1setDataSource(
		JNIEnv *env, jobject obj, jstring path) {

	filename = path;
	__android_log_print(ANDROID_LOG_DEBUG, TAG, "Set DataSource: %s", (*env)->GetStringUTFChars(env, path, 0));

}

/**
 * Start playing stream back
 */
JNIEXPORT void JNICALL Java_org_fpl_media_MediaPlayer_n_1playStream(JNIEnv *env,
		jobject obj) {
	if (filename != 0) {
		start_audio_stream(env, obj, filename);
	} else {
		jclass excCls = (*env)->FindClass(env, "java/lang/IllegalArgumentException");
		if (excCls != 0) {
			(*env)->ThrowNew(env, excCls, "Data Source is not specified");
		}
	}

}

/**
 * Stop playing the stream
 */
JNIEXPORT void JNICALL Java_org_fpl_media_MediaPlayer_n_1stopStream
  (JNIEnv *env, jobject obj) {
	__android_log_write(ANDROID_LOG_DEBUG, TAG, "Stop Playing Stream");
	stop_audio_stream();
}

/**
 * Shutdown audio engine
 */
JNIEXPORT void JNICALL Java_org_fpl_ffmpeg_Manager_shutdownEngine(JNIEnv *env,
		jobject obj) {
	__android_log_write(ANDROID_LOG_DEBUG, TAG, "Shutdown engine");
	shutdown_engine();
}
