#!/bin/bash

export NDK_BUILD="YES"

cd jni
$NDK/ndk-build clean

unset NDK_BUILD

