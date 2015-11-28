CXX = clang++
HEADERS = *.h ../daily/*.h
SOURCES = \
	../daily/*.cpp \
	MSBuild.cpp \
	XcodeProject.cpp

bootstrap:
	$(CXX) -o hidamari \
		-std=c++14 \
		-stdlib=libc++ \
		-I.. \
		$(SOURCES) \
		main.cpp
	@mkdir -p ../bin
	@mv hidamari ../bin/

xcode:
	./../bin/hidamari -generator=xcode \
		-o hidamari \
		-std=c++14 \
		-stdlib=libc++ \
		-I.. \
		$(HEADERS) \
		$(SOURCES) \
		main.cpp
	@xcodebuild -project hidamari.xcodeproj -configuration Release
	@mkdir -p ../bin
	@cp build/Release/hidamari ../bin/hidamari

test:
	./../bin/hidamari -generator=xcode \
		-o HidamariTest \
		-std=c++14 \
		-stdlib=libc++ \
		-I.. \
		-I../somerachan/iutest/include \
		$(HEADERS) \
		$(SOURCES) \
		tests/*.cpp
	@xcodebuild -project HidamariTest.xcodeproj -configuration Release
	./build/Release/HidamariTest

sketch:
	@open http://ch.nicovideo.jp/hidamari_01
