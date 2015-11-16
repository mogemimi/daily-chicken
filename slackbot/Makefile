xcode:
	@gyp slackbot.gyp --depth=. -f xcode
	@xcodebuild -project slackbot.xcodeproj -configuration Release
	@cp build/Release/slackbot ../bin/slackbot
