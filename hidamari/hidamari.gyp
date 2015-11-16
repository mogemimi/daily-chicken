{
  'includes': [
    '../somerachan/oh-my-gyp/c++14.gypi',
    '../somerachan/oh-my-gyp/config-debug-release.gypi',
    '../somerachan/oh-my-gyp/warn-as-error.gypi',
    '../somerachan/oh-my-gyp/warnings-level3.gypi',
    '../somerachan/oh-my-gyp/mac-basic.gypi',
    '../somerachan/oh-my-gyp/win32-basic.gypi',
  ],
  'targets': [
    {
      'target_name': 'hidamari',
      'product_name': 'hidamari',
      'type': 'executable',
      'include_dirs': [
        'rapidjson/include',
        '../somerachan/src',
      ],
      'sources': [
        'main.cpp',
      ],
    },
  ],
}
