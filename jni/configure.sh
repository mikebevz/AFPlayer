#!/bin/bash

if [ "$NDK" = "" ]; then
	echo NDK variable not set, assuming ${HOME}/android-ndk
	export NDK=${HOME}/android-ndk
fi

ARCH=arm
SYSROOT=$NDK/platforms/android-3/arch-$ARCH
TARGET_OS=linux
TARGET_ARCHS="armv5te" # Possible armv7a, separate with a space

# Expand the prebuilt/* path into the correct one
TOOLCHAIN=`echo $NDK/toolchains/arm-linux-androideabi-4.4.3/prebuilt/*-x86`
export PATH="$TOOLCHAIN/bin:$PATH"

export NDK_BUILD="yes"


rm -rf build/ffmpeg
mkdir -p build/ffmpeg
cd ffmpeg

# Don't build any neon version for now
for version in $TARGET_ARCHS; do

	DEST=../build/ffmpeg
	FLAGS="--target-os=$TARGET_OS --cross-prefix=$ARCH-$TARGET_OS-androideabi- --arch=$ARCH  --enable-cross-compile"
	FLAGS="$FLAGS --sysroot=$SYSROOT "
	#FLAGS="$FLAGS --soname-prefix=/data/data/com.bambuser.broadcaster/lib/"
	FLAGS="$FLAGS  --disable-everything --disable-doc --disable-asm --disable-yasm --disable-symver" #
	FLAGS="$FLAGS --enable-shared --disable-static"
	FLAGS="$FLAGS --enable-ffmpeg --enable-ffplay --enable-ffprobe --enable-avdevice"
	FLAGS="$FLAGS --enable-small" #--optimization-flags=-O2
	FLAGS="$FLAGS --enable-zlib"
	#FLAGS="$FLAGS --enable-encoder=msmpeg4v3 --enable-encoder=nellymoser "
    FLAGS="$FLAGS --enable-decoder=mp3 "
	FLAGS="$FLAGS --enable-protocol=http --enable-protocol=rtmp --enable-protocol=tcp --enable-network "
	FLAGS="$FLAGS --enable-outdev=alsa --enable-outdev=sndio --enable-outdev=sdl --enable-outdev=oss"
	FLAGS="$FLAGS --enable-muxer=mp3 --enable-demuxer=mp3 "
    FLAGS="$FLAGS --enable-indev=alsa "
    FLAGS="$FLAGS --enable-parser=mpegaudio "
    FLAGS="$FLAGS --enable-bsf=mp3_header_decompress "
    

	case "$version" in
		neon)
			EXTRA_CFLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=neon"
			EXTRA_LDFLAGS="-Wl,--fix-cortex-a8"
			# Runtime choosing neon vs non-neon requires
			# renamed files
			ABI="armeabi-v7a"
			;;
		armv7a)
			EXTRA_CFLAGS="-march=armv7-a -mfloat-abi=softfp"
			EXTRA_LDFLAGS=""
			ABI="armeabi-v7a"
			;;
		*)
			EXTRA_CFLAGS=""
			EXTRA_LDFLAGS=""
			ABI="armeabi"
			;;
	esac
	DEST="$DEST/$ABI"
	FLAGS="$FLAGS --prefix=$DEST"

	mkdir -p $DEST
	echo $FLAGS --extra-cflags="$EXTRA_CFLAGS" --extra-ldflags="$EXTRA_LDFLAGS" > $DEST/info.txt
	./configure $FLAGS --extra-cflags="$EXTRA_CFLAGS" --extra-ldflags="$EXTRA_LDFLAGS" | tee $DEST/configuration.txt
	[ $PIPESTATUS == 0 ] || exit 1
	# make clean
	# make -j4 || exit 1
	# make install || exit 1

done


unset NDK_BUILD

