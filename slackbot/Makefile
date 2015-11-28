HEADERS = *.h ../daily/*.h
SOURCES = \
	../daily/*.cpp \
	HttpUtility.cpp \
	HttpService.cpp \
	iTunesNowPlaying.cpp \
	main.cpp \
	SlackClient.cpp \
	TerminalHelper.cpp

xcode:
	./../bin/hidamari -generator=xcode \
		-o slackbot \
		-std=c++14 \
		-stdlib=libc++ \
		-llibcurl.tbd \
		-I.. \
		-Irapidjson/include \
		$(HEADERS) \
		$(SOURCES) \
		main.cpp
	@xcodebuild -project slackbot.xcodeproj -configuration Release
	@mkdir -p ../bin
	@cp build/Release/slackbot ../bin/slackbot
