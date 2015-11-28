SOURCES = \
	HttpUtility.cpp \
	HttpUtility.h \
	HttpService.cpp \
	HttpService.h \
	iTunesNowPlaying.cpp \
	iTunesNowPlaying.h \
	main.cpp \
	SlackClient.cpp \
	SlackClient.h \
	TerminalHelper.cpp \
	TerminalHelper.h \
	../daily/SubprocessHelper.cpp \
	../daily/SubprocessHelper.h \
	../daily/Optional.h

xcode:
	./../bin/hidamari -generator=xcode \
		-o slackbot \
		-std=c++14 \
		-stdlib=libc++ \
		-lcurl \
		-I.. \
		-Irapidjson/include \
		$(SOURCES) \
		main.cpp
	@xcodebuild -project slackbot.xcodeproj -configuration Release
	@mkdir -p ../bin
	@cp build/Release/slackbot ../bin/slackbot
