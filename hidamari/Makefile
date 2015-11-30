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

sketch:
	@open http://ch.nicovideo.jp/hidamari_01
