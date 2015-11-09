{
  'includes': [
    '../somerachan/oh-my-gyp/c++14.gypi',
    '../somerachan/oh-my-gyp/config-debug-release.gypi',
    '../somerachan/oh-my-gyp/warn-as-error.gypi',
    '../somerachan/oh-my-gyp/warnings-level3.gypi',
    '../somerachan/oh-my-gyp/mac-basic.gypi',
    '../somerachan/oh-my-gyp/win32-basic.gypi',
    '../somerachan/oh-my-gyp/mac/curl.gypi',
  ],
  'targets': [
    {
      'target_name': 'SlackBot',
      'product_name': 'SlackBot',
      'type': 'executable',
      'include_dirs': [
        'rapidjson/include',
      ],
      'sources': [
        'HttpService.cpp',
        'HttpService.h',
        'iTunesNowplaying.cpp',
        'iTunesNowplaying.h',
        'main.cpp',
        'SlackClient.cpp',
        'SlackClient.h',
      ],
    },
  ],
}
