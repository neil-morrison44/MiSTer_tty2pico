FROM debian:stable-slim

RUN apt-get update
RUN apt-get install build-essential -y
RUN apt-get install python3 cmake git -y
RUN apt-get install gcc-arm-none-eabi -y

RUN git clone https://github.com/micropython/micropython.git /home/micropython

WORKDIR /home/micropython

RUN make -C mpy-cross

WORKDIR /home/micropython/ports/rp2

RUN cd boards/PICO && sed "s/\/\/ #define/#define/g" -i mpconfigboard.h
RUN make submodules

ENTRYPOINT make
