#!/bin/sh
# clean existing build related objects
for dir in `find ./assign/code -type d -d 1`
do
    make clean -C ${dir}
done

# build docker image
docker build -t cs110 .
