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
        'tests/commandlineparser.cpp',
        'tests/main.cpp',
      ],
    },
  ],
}
