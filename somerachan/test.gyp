{
  'includes': [
    'oh-my-gyp/c++14.gypi',
    'oh-my-gyp/config-debug-release.gypi',
    'oh-my-gyp/warnings-level3.gypi',
    'oh-my-gyp/mac-basic.gypi',
    'oh-my-gyp/win32-basic.gypi',
  ],
  'targets': [
    {
      'target_name': 'SomeraChanTest',
      'product_name': 'SomeraChanTest',
      'type': 'executable',
      'include_dirs': [
        'src',
        'iutest/include',
      ],
      'sources': [
        'src/editdistance.cpp',
        'src/editdistance.h',
        'src/spellcheck.h',
        'src/spellcheck-mac.mm',
        'src/wordsegmenter.cpp',
        'src/wordsegmenter.h',
        'tests/editdistance.cpp',
        'tests/main.cpp',
        'tests/spellchecker.cpp',
        'tests/wordsegmenter.cpp',
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
