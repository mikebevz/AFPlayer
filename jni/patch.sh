#!/bin/bash

for i in `find diffs -type f`; do
    (cd ffmpeg && patch -p1 < ../$i)
done
