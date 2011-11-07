#!/bin/bash

export NDK_BUILD="YES"

cd jni
$NDK/ndk-gdb

unset NDK_BUILD

