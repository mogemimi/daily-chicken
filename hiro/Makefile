PRODUCTNAME = hiro
HEADERS = ../daily/*.h *.h
SOURCES = \
	../daily/*.cpp \
	main.cpp

xcode:
	./../bin/hidamari -generator=xcode \
		-o $(PRODUCTNAME) \
		-std=c++14 \
		-stdlib=libc++ \
		-I.. \
		$(HEADERS) \
		$(SOURCES)
	@xcodebuild -project $(PRODUCTNAME).xcodeproj -configuration Release
	@mkdir -p ../bin
	@cp build/Release/$(PRODUCTNAME) ../bin/$(PRODUCTNAME)
