FROM ubuntu:xenial
MAINTAINER Edmund Mok <edmundmok@outlook.com>

# move custom headers within lib dir to /usr/include (like string-utils.h)
COPY ./assign/code/lib/ /usr/include/

# install essential system progs
RUN apt-get update && apt-get install -y \
  automake \
  bison \
  flex \
  gcc \
  g++ \
  g++-5 \
  git \
  libboost-all-dev \
  libcurl4-openssl-dev \
  libreadline-dev \
  libssl-dev \
  libxml2 \
  libxml2-dev \
  make \
  man \
  python \
  python-pip \
  vim

# create cs110 folder
CMD mkdir /cs110

# install socket++ lib
WORKDIR /usr/include/socketxx
RUN ./autogen && ./configure --prefix=/usr && (make || true) && su && ((make install) || true)

