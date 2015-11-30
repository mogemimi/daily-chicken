# For example:
# SOMERACHAN_CLANG_DIR="/User/somera/clang+llvm-3.7.0-x86_64-apple-darwin"

xcode:
	@gyp somerachan.gyp --depth=. -f xcode \
		-Dclang_dir_mac=$(SOMERACHAN_CLANG_DIR)
	@xcodebuild -project somerachan.xcodeproj -configuration Release
	@mkdir -p bin
	@cp build/Release/somerachan ../bin/somera
	@cp $(SOMERACHAN_CLANG_DIR)/lib/libclang.dylib ../bin/

test:
	@gyp test.gyp --depth=. -f xcode
	@xcodebuild -project test.xcodeproj -configuration Release
	./build/Release/SomeraChanTest
