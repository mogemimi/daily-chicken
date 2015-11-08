This is my personal repo for learning CHICKEN Scheme as a hobby.

## Setup

```sh
brew install chicken
```

```sh
git submodule update --init
```

See also http://wiki.call-cc.org/platforms#mac-os-x

## Dependencies

```sh
make dependencies
```

## Build and run

```sh
make
```

## Test for Mac

```sh
make gochiusa2
```

## Build and run somerachan

Build:

```sh
export SOMERACHAN_CLANG_DIR="/User/somera/Desktop/clang+llvm-3.7.0-x86_64-apple-darwin"
make xcode
```

Run:

```sh
./bin/somera MySomerachanApp.cpp --
```

## Open Source Software used in Somerachan

* LLVM Clang
* iutest
* Generate Your Projects
