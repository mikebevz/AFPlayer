#!/bin/bash

export NDK_BUILD="YES"

$NDK/ndk-build NDK_DEBUG=1 

unset NDK_BUILD

