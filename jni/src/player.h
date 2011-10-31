#ifndef PLAYER
#define PLAYER

jint JNI_OnLoad(JavaVM* jvm, void* reserved);
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void *reserved);
JNIEnv *JNU_Get_Env();

#endif /* PLAYER */
