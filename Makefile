SOMERACHAN_LLVM_DIR="~/develop/clang+llvm-3.7.0-x86_64-apple-darwin"
SOMERACHAN_CLANG_DIR="~/develop/clang+llvm-3.7.0-x86_64-apple-darwin"

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
	@gyp somerachan/aoihana.gyp --depth=. -f xcode \
		-Dllvm_dir_mac=$(SOMERACHAN_LLVM_DIR) \
		-Dclang_dir_mac=$(SOMERACHAN_CLANG_DIR)
	@gyp somerachan/somerachan.gyp --depth=. -f xcode
	@xcodebuild -project somerachan/somerachan.xcodeproj -configuration Release
	@cp somerachan/build/Release/somerachan ./somera
