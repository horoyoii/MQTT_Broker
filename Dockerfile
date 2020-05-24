FROM ubuntu:bionic

COPY . /app
WORKDIR /app

RUN ./main

