#!/bin/bash

if [ "$NDK" = "" ]; then
	echo NDK variable not set, assuming ${HOME}/android-ndk
	export NDK=${HOME}/android-ndk
fi


FFMPEG_VERSION="0.8.5"
FFMPEG_FILE="ffmpeg-$FFMPEG_VERSION.tar.gz"
FFMPEG_URL="http://ffmpeg.org/releases/$FFMPEG_FILE"
FFMPEG_SIG="http://ffmpeg.org/releases/ffmpeg-$FFMPEG_VERSION.tar.bz2.asc"
FFMPEG_DIR="ffmpeg"
PATCH="diffs/ffmpeg_"$FFMPEG_VERSION"_android.patch"

if [ -d $FFMPEG_DIR ]; then
	echo "Delete"
	rm -Rf $FFMPEG_DIR	
fi

if [ -f $FFMPEG_FILE ]; then
	echo "Delete existing archive"
	rm $FFMPEG_FILE
fi


# Download archive
# 
wget $FFMPEG_URL
	
if [ -f $FFMPEG_FILE ]; then
	tar -xzvf $FFMPEG_FILE
    mv ffmpeg-$FFMPEG_VERSION $FFMPEG_DIR
    cd $FFMPEG_DIR
    patch -p1 < ../$PATCH
    cd ..
    ./configure.sh
    ./ndkbuild.sh
    rm $FFMPEG_FILE
fi


      
