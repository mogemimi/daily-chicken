# typo-poi

Typo/misspell checking library for C++ and command-line tool using LLVM Clang.

## Build and run

Build:

```sh
cd daily-chicken
export SOMERACHAN_CLANG_DIR="/User/somera/Desktop/clang+llvm-3.7.0-x86_64-apple-darwin"
make -C somerachan xcode
```

Run:

```sh
./bin/somera YourSourceCode.cpp --
```

## License

MIT License.

## Thanks

The following libraries and/or open source projects were used in typo-poi:

* [LLVM Clang](http://clang.llvm.org/)
* [iutest - iris unit test framework](https://github.com/srz-zumix/iutest)
