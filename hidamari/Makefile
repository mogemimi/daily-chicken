CXX = clang++
SOURCES = \
	Any.h \
	CommandLineParser.cpp \
	CommandLineParser.h \
	FileSystem.cpp \
	FileSystem.h \
	XcodeProject.cpp \
	XcodeProject.h \
	StringHelper.cpp \
	StringHelper.h \
	SubprocessHelper.cpp \
	SubprocessHelper.h

bootstrap:
	$(CXX) -ohidamari \
		-std=c++14 \
		-stdlib=libc++ \
		-I../somerachan/src \
		*.cpp
	@mv hidamari ../bin/

xcode:
	./../bin/hidamari -generator=xcode \
		-ohidamari \
		-std=c++14 \
		-stdlib=libc++ \
		-I../somerachan/src \
		$(SOURCES) \
		main.cpp \
		*.h
	@xcodebuild -project hidamari.xcodeproj -configuration Release
	@cp build/Release/hidamari ../bin/hidamari

test:
	./../bin/hidamari -generator=xcode \
		-oHidamariTest \
		-std=c++14 \
		-stdlib=libc++ \
		-I../somerachan/src \
		-I../somerachan/iutest/include \
		tests/*.cpp \
		$(SOURCES) \
		*.h
	@xcodebuild -project HidamariTest.xcodeproj -configuration Release
	./build/Release/HidamariTest

hidamari:
	@open http://ch.nicovideo.jp/hidamari_01
