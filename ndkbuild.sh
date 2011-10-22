#!/bin/bash

export NDK_BUILD="YES"

cd jni
$NDK/ndk-build

unset NDK_BUILD

