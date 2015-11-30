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

test:
	./bin/hidamari -generator=xcode \
		-o DailyTest \
		-std=c++14 \
		-stdlib=libc++ \
		-I. \
		-Isomerachan/iutest/include \
		daily/*.h \
		daily/*.cpp \
		daily/tests/*.cpp
	@xcodebuild -project DailyTest.xcodeproj -configuration Release
	./build/Release/DailyTest
