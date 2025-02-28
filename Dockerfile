FROM ubuntu:latest
ENV DEBIAN_FRONTEND=noninteractive
RUN apt -y update
RUN apt -y upgrade
RUN apt -y install build-essential
RUN apt -y install cmake
RUN apt -y install git
RUN apt -y install libgtest-dev

# Dependencies
RUN apt -y install libtool
RUN apt -y install autoconf
RUN apt -y install libudev-dev
RUN apt -y install libopencv-dev
RUN apt -y install libeigen3-dev

# Build
COPY . /openev
WORKDIR /openev
RUN rm -rf build && mkdir build && cd build && cmake .. && make
