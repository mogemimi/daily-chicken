{
  'includes': [
    'oh-my-gyp/c++14.gypi',
    'oh-my-gyp/config-debug-release.gypi',
    'oh-my-gyp/warn-as-error.gypi',
    'oh-my-gyp/warnings-level3.gypi',
    'oh-my-gyp/mac-basic.gypi',
    'oh-my-gyp/win32-basic.gypi',
    'oh-my-gyp/mac/llvm.gypi',
    'oh-my-gyp/mac/clang.gypi',
  ],
  'targets': [
    {
      'target_name': 'AoiHana',
      'product_name': 'aoihana',
      'type': 'executable',
      'sources': [
        'aoihana.cpp',
      ],
      'xcode_settings': {
        'MACOSX_DEPLOYMENT_TARGET': '10.11',
        'CLANG_ENABLE_OBJC_ARC': 'YES',
      },
      'link_settings': {
        'libraries': [
          '$(SDKROOT)/System/Library/Frameworks/AppKit.framework',
        ],
      },
    },
  ],
}
