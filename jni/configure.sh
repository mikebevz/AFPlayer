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
	FLAGS="$FLAGS --disable-everything --disable-doc --disable-asm  --disable-symver --disable-stripping" #  
	FLAGS="$FLAGS --enable-shared  --enable-debug=3 --enable-gpl --enable-memalign-hack" # --enable-version3
	FLAGS="$FLAGS --nm=$TOOLCHAIN/bin/$ARCH-$TARGET_OS-androideabi-nm "
	#FLAGS="$FLAGS -Wl,-T"
	FLAGS="$FLAGS --enable-postproc --enable-swscale --enable-avfilter" # --disable-yasm
	FLAGS="$FLAGS --enable-small" #--optimization-flags=-O2
	FLAGS="$FLAGS --enable-zlib "	
    FLAGS="$FLAGS --enable-decoder=mp3 --enable-decoder=mp3adu --enable-decoder=mp3adufloat \
                  --enable-decoder=mp3float --enable-decoder=mp3on4 --enable-decoder=mp3on4floats \
                  --enable-decoder=aac --enable-decoder=aac_latm --enable-decoder=ac3 --enable-decoder=alac\
                  --enable-decoder=wmav1 --enable-decoder=wmav2 --enable-decoder=wmavoice"
	FLAGS="$FLAGS --enable-protocol=http --enable-protocol=tcp --enable-network --enable-protocol=applehttp \
	              --enable-protocol=rtp  --enable-protocol=udp --enable-protocol=md5 --enable-protocol=concat \
	              --enable-protocol=file --enable-protocol=crypto --enable-protocol=pipe --enable-protocol=rtsp \
	              --enable-protocol=mmst --enable-protocol=mmsh"
	FLAGS="$FLAGS --enable-muxer=mp3 --enable-muxer=wav --enable-muxer=pcm_alaw --enable-muxer=pcm_mulaw \
	              --enable-muxer=pcm_s16be --enable-muxer=pcm_s16le --enable-muxer=pcm_u16be --enable-muxer=pcm_u16le \
	              --enable-muxer=rtsp --enable-muxer=asf --enable-muxer=asf_stream\
	              --enable-demuxer=mp3 --enable-demuxer=wav --enable-demuxer=aac --enable-demuxer=applehttp \
	              --enable-demuxer=mpegts --enable-demuxer=aac --enable-demuxer=ogg --enable-demuxer=rtsp \
	              --enable-demuxer=asf "
    FLAGS="$FLAGS --enable-parser=mpegaudio --enable-parser=aac --enable-parser=aac_latm "
    #FLAGS="$FLAGS --enable-bsf=mp3_header_decompress --enable-bsf=aac_adtstoasc --enable-bsf=noise "
    
    

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
	echo $DEST
	echo ./configure $FLAGS --extra-cflags="$EXTRA_CFLAGS" --extra-ldflags="$EXTRA_LDFLAGS"
	./configure $FLAGS --extra-cflags="$EXTRA_CFLAGS" --extra-ldflags="$EXTRA_LDFLAGS" | tee $DEST/configuration.txt
	[ $PIPESTATUS == 0 ] || exit 1
	# make clean
	# make -j4 || exit 1
	# make install || exit 1

done


unset NDK_BUILD

