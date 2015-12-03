{
  'includes': [
    'oh-my-gyp/c++14.gypi',
    'oh-my-gyp/config-debug-release.gypi',
    'oh-my-gyp/warn-as-error.gypi',
    'oh-my-gyp/warnings-level3.gypi',
    'oh-my-gyp/mac-basic.gypi',
    'oh-my-gyp/win32-basic.gypi',
    'oh-my-gyp/mac/clang-libtooling.gypi',
  ],
  'targets': [
    {
      'target_name': 'SomeraChan',
      'product_name': 'somerachan',
      'type': 'executable',
      'include_dirs': [
        'src',
        '..',
      ],
      'sources': [
        'src/main.cpp',
        'src/consolecolor.cpp',
        'src/consolecolor.h',
        'src/editdistance.cpp',
        'src/editdistance.h',
        'src/spellcheck.h',
        'src/typo.cpp',
        'src/typo.h',
        'src/utf8.cpp',
        'src/utf8.h',
        'src/worddiff.cpp',
        'src/worddiff.h',
        'src/wordsegmenter.cpp',
        'src/wordsegmenter.h',
        'src/thirdparty/ConvertUTF.c',
        'src/thirdparty/ConvertUTF.h',
        '../daily/Optional.h',
        '../daily/StringHelper.cpp',
        '../daily/StringHelper.h',
      ],
      'conditions': [
        ['OS == "win"', {
          'sources': [
            'src/spellcheck-win.cpp',
          ],
        }],
        ['OS == "mac"', {
          'sources': [
            'src/spellcheck-mac.mm',
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/AppKit.framework',
            ],
          },
        }],
      ],
      'xcode_settings': {
        'MACOSX_DEPLOYMENT_TARGET': '10.11',
        'CLANG_ENABLE_OBJC_ARC': 'YES',
        'LD_RUNPATH_SEARCH_PATHS': [
            '@executable_path/',
        ],
      },
    },
  ],
}
