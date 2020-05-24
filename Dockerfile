FROM ubuntu:bionic

COPY . /app
WORKDIR /app

RUN apt-get update
RUN apt-get install -y build-essential
RUN make

CMD ["./main"]
