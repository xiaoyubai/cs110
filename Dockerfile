FROM ubuntu:xenial
MAINTAINER Edmund Mok <edmundmok@outlook.com>

# move custom headers within lib dir to /usr/include (like string-utils.h)
COPY ./assign/code/lib/ /usr/include/

# install essential system progs
RUN apt-get update && apt-get install -y \
  bison \
  flex \
  gcc \
  g++-5 \
  git \
  libreadline-dev \
  make \
  man \
  python \
  vim

# create cs110 folder
CMD mkdir /cs110

# set cs110 as working dir
WORKDIR /cs110

