FROM ubuntu:latest
ENV DEBIAN_FRONTEND=noninteractive
RUN apt update
RUN apt upgrade
RUN apt install -y build-essential
RUN apt install -y cmake
RUN apt install -y git
RUN apt install -y libgtest-dev

# Dependencies
RUN apt install -y libtool
RUN apt install -y autoconf
RUN apt install -y libudev-dev
RUN apt install -y libopencv-dev
RUN apt install -y libeigen3-dev

# Build
COPY . /openev
WORKDIR /openev
RUN rm -rf build && mkdir build && cd build && cmake .. && make
