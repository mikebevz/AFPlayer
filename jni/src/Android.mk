LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := player
LOCAL_SRC_FILES := player.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../includes \
					$(LOCAL_PATH)/../ffmpeg
					
LOCAL_LDLIBS += -L$(LOCAL_PATH)/../../libs/armeabi  -llog \
			    -L$(LOCAL_PATH)/../../obj/local/armeabi -lavformat -lavcodec  \
			    	-lavutil -lpostproc -lswscale
#LOCAL_SHARED_LIBRARIES := ffmpeg
LOCAL_STATIC_LIBRARIES := libavformat libavcodec libavutil libpostproc libswscale 

include $(BUILD_SHARED_LIBRARY)