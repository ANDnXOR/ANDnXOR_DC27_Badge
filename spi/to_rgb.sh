#!/bin/bash
ffmpeg -hide_banner -loglevel quiet -i $1 -r 20 -f rawvideo -s 17x9 -pix_fmt rgb24 $2
