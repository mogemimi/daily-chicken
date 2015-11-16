xcode:
	@gyp hidamari.gyp --depth=. -f xcode
	@xcodebuild -project hidamari.xcodeproj -configuration Release
	@cp build/Release/hidamari ../bin/hidamari
