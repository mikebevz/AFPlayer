
all: Android.mk

Android.mk: configure.sh
	echo Running configure.sh ...
	./configure.sh
	touch Android.mk

include $(call all-subdir-makefiles)


