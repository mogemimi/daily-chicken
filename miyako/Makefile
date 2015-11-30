HEADERS = ../daily/*.h *.h
SOURCES = \
	../daily/*.cpp \
	Generator.cpp \
	main.cpp

xcode:
	./../bin/hidamari -generator=xcode \
		-o miyako \
		-std=c++14 \
		-stdlib=libc++ \
		-I.. \
		-I../slackbot/rapidjson/include \
		$(HEADERS) \
		$(SOURCES)
	@xcodebuild -project miyako.xcodeproj -configuration Release
	@mkdir -p ../bin
	@cp build/Release/miyako ../bin/miyako
