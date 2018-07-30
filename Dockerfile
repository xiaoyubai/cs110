FROM ubuntu:xenial
MAINTAINER Edmund Mok <edmundmok@outlook.com>

# install essential system progs
RUN apt-get update && apt-get install -y \
  gcc \
  g++-5 \
  git \
  make \
  vim

# copy over git repo
COPY . / /cs110/
WORKDIR /cs110

