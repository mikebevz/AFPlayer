#!/bin/bash

export NDK_BUILD="YES"

$NDK/ndk-build

unset NDK_BUILD

