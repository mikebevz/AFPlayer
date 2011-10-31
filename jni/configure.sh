#!/bin/bash

if [ "$NDK" = "" ]; then
	echo NDK variable not set, assuming ${HOME}/android-ndk
	export NDK=${HOME}/android-ndk
fi

ARCH=arm
SYSROOT=$NDK/platforms/android-3/arch-$ARCH
INCLUDES=$SYSROOT/usr/include 
TARGET_OS=linux
TARGET_ARCHS="armv5te" # Possible armv7a, separate with a space

# Expand the prebuilt/* path into the correct one
TOOLCHAIN=`echo $NDK/toolchains/arm-linux-androideabi-4.4.3/prebuilt/*-x86`
export PATH="$TOOLCHAIN/bin:$PATH"

export NDK_BUILD="yes"


#rm -rf build/ffmpeg
#mkdir -p build/ffmpeg
cd ffmpeg

# Don't build any neon version for now
for version in $TARGET_ARCHS; do

	DEST=../build/ffmpeg
	FLAGS="--target-os=$TARGET_OS --cross-prefix=$ARCH-$TARGET_OS-androideabi- --arch=$ARCH  --enable-cross-compile"
	FLAGS="$FLAGS --sysroot=$SYSROOT "
	#git FLAGS="$FLAGS --soname-prefix=/data/data/org.fpl/lib/"
	FLAGS="$FLAGS --disable-everything --disable-doc --disable-asm --disable-yasm --disable-symver --disable-stripping " #
	FLAGS="$FLAGS --enable-shared --enable-gpl --enable-version3 --enable-debug=3 "
	FLAGS="$FLAGS --nm=$TOOLCHAIN/bin/$ARCH-$TARGET_OS-androideabi-nm "
	#FLAGS="$FLAGS -Wl,-T"
	FLAGS="$FLAGS --enable-ffmpeg --enable-ffserver --enable-ffplay --enable-ffprobe --enable-avdevice --enable-swscale "
	FLAGS="$FLAGS --enable-small" #--optimization-flags=-O2
	FLAGS="$FLAGS --enable-zlib "
	#FLAGS="$FLAGS --enable-encoder=msmpeg4v3 --enable-encoder=nellymoser --enable-encoder=pcm_alaw \
	#              --enable-encoder=pcm_f32be --enable-encoder=pcm_s16be --enable-encoder=pcm_s16le --disable-encoder=h264"
	
    FLAGS="$FLAGS --enable-decoder=mp3 --enable-decoder=mp3adu --enable-decoder=mp3adufloat \
                  --enable-decoder=mp3float --enable-decoder=mp3on4 --enable-decoder=mp3on4floats \
                  --enable-decoder=aac --enable-decoder=aac_latm"
	FLAGS="$FLAGS --enable-protocol=http --enable-protocol=rtmp --enable-protocol=tcp --enable-network --enable-protocol=applehttp \
	             # --enable-protocol=rtmpe --enable-protocol=rtmps --enable-protocol=rtmtp --enable-protocol=rtmtpe --enable-protocol=rtp \
	              --enable-protocol=udp"
	#FLAGS="$FLAGS --enable-outdev=alsa --enable-outdev=sndio --enable-outdev=sdl --enable-outdev=oss"
	FLAGS="$FLAGS --enable-muxer=mp3 --enable-muxer=wav --enable-muxer=pcm_alaw --enable-muxer=pcm_mulaw \
	              --enable-muxer=pcm_s16be --enable-muxer=pcm_s16le --enable-muxer=pcm_u16be --enable-muxer=pcm_u16le \
	              --enable-demuxer=mp3 --enable-demuxer=wav --enable-demuxer=aac --enable-demuxer=applehttp --enable-demuxer=mpegts --enable-demuxer=aac"
    #FLAGS="$FLAGS --enable-indev=alsa --enable-indev=bktr --enable-indev=dv1394 --enable-indev=jack \
    #              --enable-indev=v412 --enable-indev=libdc1394 --enable-indev=vfcap --enable-indev=vfwcap \
    #              --enable-indev=dshow --enable-indev=oss --enable-indev=x11_grab_device \
    #              --enable-indev=dv1394 --enable-indev=sndio --enable-indev=fbdev --enable-indev=v4l"
    FLAGS="$FLAGS --enable-parser=mpegaudio --enable-parser=aac --enable-parser=aac_latm "
    FLAGS="$FLAGS --enable-bsf=mp3_header_decompress --enable-bsf=aac_adtstoasc"
    
    

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

