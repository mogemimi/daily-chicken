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
      'target_name': 'slackbot',
      'product_name': 'slackbot',
      'type': 'executable',
      'include_dirs': [
        'rapidjson/include',
        '../somerachan/src',
      ],
      'sources': [
        'HttpUtility.cpp',
        'HttpUtility.h',
        'HttpService.cpp',
        'HttpService.h',
        'iTunesNowPlaying.cpp',
        'iTunesNowPlaying.h',
        'main.cpp',
        'SlackClient.cpp',
        'SlackClient.h',
        'TerminalHelper.cpp',
        'TerminalHelper.h',
        '../daily/SubprocessHelper.cpp',
        '../daily/SubprocessHelper.h',
        '../daily/Optional.h',
      ],
    },
  ],
}
