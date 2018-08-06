#!/bin/sh
# [d]ocker prevents grep itself from showing up in grep results
kill $(ps -a | grep [d]ocker | head -n 1 | awk '/ / { print $1 }')
sh run.sh
