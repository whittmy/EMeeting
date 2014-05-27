#!/bin/bash

lame --decode $1 - | sox -q - -r 45100 -b 16 -e signed-integer -c 2 -t raw - | \
   ./klient -s localhost > /dev/null
