#!/bin/bash

lame --decode "sample.mp3" - | sox -q - -r 44100 -b 16 -e signed-integer -c 2 -t raw - | \
   ./klient -s localhost "$@" | \
   aplay -t raw -f cd -B 5000 -v -D sysdefault -
