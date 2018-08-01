FROM ubuntu:xenial
MAINTAINER Edmund Mok <edmundmok@outlook.com>

# install essential system progs
RUN apt-get update && apt-get install -y \
  gcc \
  g++-5 \
  git \
  make \
  python \
  vim

# create cs110 folder
CMD mkdir /cs110

# set cs110 as working dir
WORKDIR /cs110

