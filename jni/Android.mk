LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := player
LOCAL_SRC_FILES := src/player.c
LOCAL_C_INCLUDES := includes
LOCAL_LDLIBS += -L$(LOCAL_PATH)/../libs/armeabi -lffmpeg -llog

include $(BUILD_SHARED_LIBRARY)

include $(LOCAL_PATH)/ffmpeg/Android.mk












