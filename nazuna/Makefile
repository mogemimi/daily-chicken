HEADERS = ../daily/*.h *.h
SOURCES = \
	main.cpp

xcode:
	./../bin/hidamari -generator=xcode \
		-o nazuna \
		-std=c++14 \
		-stdlib=libc++ \
		-I.. \
		$(HEADERS) \
		$(SOURCES)
	@xcodebuild -project nazuna.xcodeproj -configuration Release
	@mkdir -p ../bin
	@cp build/Release/nazuna ../bin/nazuna
