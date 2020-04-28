FROM ubuntu:18.04
RUN mkdir /var/log/erss
RUN apt-get -y update && apt-get install -y autoconf automake libtool curl make g++ unzip git sudo
RUN apt-get -y update && apt-get install -y g++ libssl-dev libxerces-c-dev libpqxx-dev
RUN apt-get update && apt-get install -y libssl-dev libxerces-c-dev libpqxx-dev
RUN git clone https://github.com/protocolbuffers/protobuf.git \
    &&cd protobuf   \
    &&git submodule update --init --recursive \
    &&./autogen.sh  \
    &&./configure --prefix=/usr &&\
    make &&\
    make check &&\
    make install && \
    ldconfig && \
    cd ..

ADD ./backend /var/log/erss

WORKDIR /var/log/erss/backend
