all: build run

poi: build run

build:
	@csc poi.scm

run:
	@./poi

dependencies:
	chicken-install http-client
	chicken-install json
	chicken-install vector-lib
	chicken-install openssl

gochiusa2:
	open http://ch.nicovideo.jp/gochiusa2
