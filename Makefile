# For example:
# SOMERACHAN_CLANG_DIR="/User/somera/clang+llvm-3.7.0-x86_64-apple-darwin"

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
	@gyp somerachan/somerachan.gyp --depth=. -f xcode \
		-Dclang_dir_mac=$(SOMERACHAN_CLANG_DIR)
	@xcodebuild -project somerachan/somerachan.xcodeproj -configuration Release
	@mkdir -p bin
	@cp somerachan/build/Release/somerachan bin/somera
	@cp $(SOMERACHAN_CLANG_DIR)/lib/libclang.dylib bin/

test:
	@gyp somerachan/test.gyp --depth=. -f xcode
	@xcodebuild -project somerachan/test.xcodeproj -configuration Release
	./somerachan/build/Release/SomeraChanTest

slack:
	@gyp slackbot/slackbot.gyp --depth=. -f xcode
	@xcodebuild -project slackbot/slackbot.xcodeproj -configuration Release
	@cp slackbot/build/Release/slackbot bin/slackbot
