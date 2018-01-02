.PHONY=default build push

default: build

build: Dockerfile Makefile 
	cp -a /usr/local/lib/libgrpc* libs/
	cp -a /usr/local/lib/libprotobuf* libs/
	cp -a /usr/local/lib/libwebsockets* libs/
	cp -a /usr/local/lib/libmsspeech* libs/
	cp -a ../build/msspeech-gbridge bin/

	docker build -t msspeech-gbridge .

push: build
	docker tag msspeech-gbridge technicianted/msspeech-gbridge:experimental
	docker push technicianted/msspeech-gbridge

clean:
	rm -rf libs/*
	rm -rf bin/*
