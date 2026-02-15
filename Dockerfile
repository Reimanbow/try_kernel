FROM ubuntu:24.04

RUN apt update && apt install -y \
    cmake \
    build-essential \
    gcc-arm-none-eabi \
    git \
    make

WORKDIR /workspace