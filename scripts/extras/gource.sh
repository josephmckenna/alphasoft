#!/bin/bash

#Script to make a gource animation of the git...

gource --hide filenames --seconds-per-day 0.1 --auto-skip-seconds 1 \
 --key \
 --hash-seed 7996 \
 --bloom-intensity 1. \
  --bloom-multiplier 1. \
 --multi-sampling -1920x1080 -o - | ffmpeg -y -r 60 -f image2pipe \
 -vcodec ppm -i - -vcodec libx264 -preset ultrafast -pix_fmt yuv420p \
 -crf 1 -threads 0 -bf 0 gource.mp4
