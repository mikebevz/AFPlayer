LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := player
LOCAL_SRC_FILES := player.c fplayer.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../includes \
					$(LOCAL_PATH)/../ffmpeg
					
LOCAL_LDLIBS += -L$(LOCAL_PATH)/../../libs/armeabi -llog  \
			    -L$(LOCAL_PATH)/../../obj/local/armeabi -lavdevice -lavformat -lavfilter -lavcodec \
			    	-lavutil -lpostproc -lswscale
LOCAL_STATIC_LIBRARIES := libavcodec libavdevice libavfilter libavformat libavutil libpostproc \
						  libswscale 

LOCAL_CFLAGS += -g					  
LOCAL_CPPFLAGS += -g
LOCAL_LDFLAGS += -Wl,-Map,xxx.map

include $(BUILD_SHARED_LIBRARY)