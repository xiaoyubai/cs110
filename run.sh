#!/bin/sh
# privileged mode is required for ptrace
docker run --privileged -w /cs110/assign/code/assign8 -p 51630:51630 -v $(pwd):/cs110 -it cs110 /bin/bash 
