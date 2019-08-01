FROM ubuntu:18.04

RUN sed -i.bak -e "s%http://archive.ubuntu.com/ubuntu/%http://ftp.tsukuba.wide.ad.jp/Linux/ubuntu/%g" /etc/apt/sources.list

RUN apt-get update && \
  apt-get install -y build-essential cmake ninja-build git curl && \
  apt-get clean && \
  rm -rf /var/lib/apt/lists/*
