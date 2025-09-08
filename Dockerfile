FROM ubuntu:latest
ENV DEBIAN_FRONTEND=noninteractive
RUN apt -y update
RUN apt -y upgrade
RUN apt -y install build-essential
RUN apt -y install cmake
RUN apt -y install git
RUN apt -y install libgtest-dev
RUN apt -y install software-properties-common

# Dependencies
RUN add-apt-repository ppa:inivation-ppa/inivation
RUN apt -y update
RUN apt -y install libboost-all-dev
RUN apt -y install libcaer-dev
RUN apt -y install libeigen3-dev
RUN apt -y install libopencv-dev

# Build
COPY . /openev
WORKDIR /openev
RUN git submodule init && git submodule update
RUN rm -rf build && mkdir build && cd build && cmake .. && make && make test
