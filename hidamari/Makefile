xcode:
	@gyp hidamari.gyp --depth=. -f xcode
	@xcodebuild -project hidamari.xcodeproj -configuration Release
	@cp build/Release/hidamari ../bin/hidamari

test:
	@gyp test.gyp --depth=. -f xcode
	@xcodebuild -project test.xcodeproj -configuration Release
	./build/Release/HidamariTest

hidamari:
	@open http://ch.nicovideo.jp/hidamari_01

dogfood:
	./../bin/hidamari -generator=xcode -oHidamari2 \
		-std=c++14 \
		-stdlib=libc++ \
		-I../somerachan/src \
		*.cpp \
		*.h
