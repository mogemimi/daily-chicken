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
      'target_name': 'SomeraChan',
      'product_name': 'somerachan',
      'type': 'executable',
      'sources': [
        'src/main.cpp',
        'src/consolecolor.cpp',
        'src/consolecolor.h',
        'src/editdistance.cpp',
        'src/editdistance.h',
        'src/optional.h',
        'src/spellcheck.h',
        'src/spellcheck-mac.mm',
        'src/worddiff.cpp',
        'src/worddiff.h',
        'src/wordsegmenter.cpp',
        'src/wordsegmenter.h',
      ],
      'xcode_settings': {
        'MACOSX_DEPLOYMENT_TARGET': '10.11',
        'CLANG_ENABLE_OBJC_ARC': 'YES',
        'LD_RUNPATH_SEARCH_PATHS': [
            '@executable_path/',
        ],
      },
      'link_settings': {
        'libraries': [
          '$(SDKROOT)/System/Library/Frameworks/AppKit.framework',
        ],
      },
    },
  ],
}
