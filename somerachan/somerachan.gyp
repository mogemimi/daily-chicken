{
  'includes': [
    'oh-my-gyp/c++14.gypi',
    'oh-my-gyp/config-debug-release.gypi',
    'oh-my-gyp/warn-as-error.gypi',
    'oh-my-gyp/warnings-level3.gypi',
    'oh-my-gyp/mac-basic.gypi',
    'oh-my-gyp/win32-basic.gypi',
    'oh-my-gyp/mac/libgit2.gypi',
  ],
  'targets': [
    {
      'target_name': 'SomeraChan',
      'product_name': 'SomeraChan',
      'type': 'executable',
      'sources': [
        'main.cpp',
        'editdistance.cpp',
        'editdistance.h',
        'spellcheck.h',
        'spellcheck-mac.mm',
      ],
      'link_settings': {
        'libraries': [
          '$(SDKROOT)/System/Library/Frameworks/AppKit.framework',
        ],
      },
    },
  ],
}
