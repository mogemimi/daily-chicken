CXX = clang++
HEADERS = *.h
SOURCES = \
	CommandLineParser.cpp \
	FileSystem.cpp \
	XcodeProject.cpp \
	StringHelper.cpp \
	SubprocessHelper.cpp

bootstrap:
	$(CXX) -o hidamari \
		-std=c++14 \
		-stdlib=libc++ \
		-I../somerachan/src \
		$(SOURCES) \
		main.cpp
	@mv hidamari ../bin/

xcode:
	./../bin/hidamari -generator=xcode \
		-o hidamari \
		-std=c++14 \
		-stdlib=libc++ \
		-I../somerachan/src \
		$(HEADERS) \
		$(SOURCES) \
		main.cpp
	@xcodebuild -project hidamari.xcodeproj -configuration Release
	@cp build/Release/hidamari ../bin/hidamari

test:
	./../bin/hidamari -generator=xcode \
		-o HidamariTest \
		-std=c++14 \
		-stdlib=libc++ \
		-I../somerachan/src \
		-I../somerachan/iutest/include \
		$(HEADERS) \
		$(SOURCES) \
		tests/*.cpp
	@xcodebuild -project HidamariTest.xcodeproj -configuration Release
	./build/Release/HidamariTest

hidamari:
	@open http://ch.nicovideo.jp/hidamari_01
