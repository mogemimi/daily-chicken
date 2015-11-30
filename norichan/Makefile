HEADERS = ../daily/*.h
SOURCES = ../daily/*.cpp

xcode:
	./../bin/hidamari -generator=xcode \
		-o norichan \
		-std=c++14 \
		-stdlib=libc++ \
		-I.. \
		$(HEADERS) \
		$(SOURCES) \
		main.cpp
	@xcodebuild -project norichan.xcodeproj -configuration Release
	@mkdir -p ../bin
	@cp build/Release/norichan ../bin/norichan
