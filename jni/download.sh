#!/bin/bash

FFMPEG_URL = "http://ffmpeg.org/releases/ffmpeg-0.8.5.tar.gz"
FFMPEG_DIR = "ffmpeg"

if [ !-d $FFMPEG_URL ]; then
    # Download archive
    # 
    wget -i- $FFMPEG_DIR
    
    
fi