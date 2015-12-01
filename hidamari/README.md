# Hidamari

A Clang/GCC-compatible C/C++ project file generator for coding sketch.

## Build and run

```sh
cd daily-chicken

# Build
cd make -C hidamari bootstrap
cd make -C hidamari xcode

# Run
./bin/hidamari -help
```

## Usage

```sh
# Generate myapp.xcodeproj file
hidamari -generator=xcode -o myapp myapp.cpp

# Generate myapp.sln and myapp.vcxproj file
hidamari -generator=msbuild -o myapp myapp.cpp
```

```sh
hidamari -generator=xcode \
  -o myapp \
  -std=c++14 \
  -stdlib=libc++ \
  -Ipath/to/include \
  *.cpp
@xcodebuild -project myapp.xcodeproj -configuration Release
```

The command-line tool is Clang/GCC-compatible, so it is the same as running the following commands in terminal:

```sh
clang \
  -o myapp \
  -std=c++14 \
  -stdlib=libc++ \
  -Ipath/to/include \
  *.cpp
```

## License

MIT License.
