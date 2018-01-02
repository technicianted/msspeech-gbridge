FROM ubuntu:16.04

RUN apt-get update && \
	apt-get install -y libuuid1 libjson-c2 zlib1g libssl1.0.0 ca-certificates

ADD bin/* run.sh msspeech-gbridge /
ADD libs/* /usr/local/lib/

EXPOSE 8080/tcp
