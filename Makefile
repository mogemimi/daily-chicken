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

somera:
	open http://ch.nicovideo.jp/somera

gochiusa2:
	open http://ch.nicovideo.jp/gochiusa2

xcode:
	@gyp somerachan/somerachan.gyp --depth=. -f xcode
