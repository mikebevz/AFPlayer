LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := player
LOCAL_SRC_FILES := fplayer.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../includes \
					$(LOCAL_PATH)/../ffmpeg
					
LOCAL_LDLIBS += -L$(LOCAL_PATH)/../../libs/armeabi -llog  \
			    -L$(LOCAL_PATH)/../../obj/local/armeabi -lavdevice -lavformat -lavfilter -lavcodec \
			    	-lavutil -lpostproc -lswscale
LOCAL_STATIC_LIBRARIES := libavcodec libavdevice libavfilter libavformat libavutil libpostproc \
						  libswscale 

LOCAL_CFLAGS += -ggdb -fexceptions -Wall -Wextra -O0 -Wunreachable-code 			  
LOCAL_CPPFLAGS += -ggdb -Wall -Wextra
LOCAL_CXXFLAGS += -ggdb -Wall -Wextra -O0 -Wunreachable-code
LOCAL_LDFLAGS += -Wl -ggdb -lz 

include $(BUILD_SHARED_LIBRARY)
