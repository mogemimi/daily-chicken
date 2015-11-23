{
  'includes': [
    '../somerachan/oh-my-gyp/c++14.gypi',
    '../somerachan/oh-my-gyp/config-debug-release.gypi',
    '../somerachan/oh-my-gyp/warnings-level3.gypi',
    '../somerachan/oh-my-gyp/mac-basic.gypi',
    '../somerachan/oh-my-gyp/win32-basic.gypi',
  ],
  'targets': [
    {
      'target_name': 'HidamariTest',
      'product_name': 'HidamariTest',
      'type': 'executable',
      'include_dirs': [
        'src',
        '../somerachan/iutest/include',
      ],
      'sources': [
        'Any.h',
        'CommandLineParser.cpp',
        'CommandLineParser.h',
        'StringHelper.cpp',
        'StringHelper.h',
        'SubprocessHelper.cpp',
        'SubprocessHelper.h',
        'tests/Any.cpp',
        'tests/CommandLineParser.cpp',
        'tests/main.cpp',
      ],
    },
  ],
}
