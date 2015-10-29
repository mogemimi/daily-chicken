{
  'conditions': [['OS == "mac"', {'target_defaults': {
    'variables': {
      'clang_dir_mac%': '/usr/local/opt/clang+llvm',
    },
    'include_dirs': [
      '<(clang_dir_mac)/include',
    ],
    'xcode_settings': {
      'LIBRARY_SEARCH_PATHS': [
        '<(clang_dir_mac)/lib',
      ],
      'OTHER_CPLUSPLUSFLAGS': [
          '-fno-exceptions',
          '-fno-rtti',
      ],
    },
    'link_settings': {
      'libraries': [
        'libclang.dylib',
        'libclang.a',
        'libclangAnalysis.a',
        'libclangAST.a',
        'libclangASTMatchers.a',
        'libclangBasic.a',
        'libclangDriver.a',
        'libclangEdit.a',
        'libclangFrontend.a',
        'libclangFrontendTool.a',
        'libclangParse.a',
        'libclangLex.a',
        'libclangRewrite.a',
        'libclangRewriteFrontend.a',
        'libclangSema.a',
        'libclangSerialization.a',
        'libclangTooling.a',

        'libclangARCMigrate.a',
        'libclangCodeGen.a',
        'libclangIndex.a',
        'libclangStaticAnalyzerCheckers.a',
        'libclangStaticAnalyzerCore.a',
        'libclangStaticAnalyzerFrontend.a',
        'libclangToolingCore.a',

        # '<(clang_dir_mac)/lib/clang/3.7.0/lib/darwin/libclang_rt.10.4.a',
        # '<(clang_dir_mac)/lib/clang/3.7.0/lib/darwin/libclang_rt.cc_kext.a',
        # '<(clang_dir_mac)/lib/clang/3.7.0/lib/darwin/libclang_rt.osx.a',
        # '<(clang_dir_mac)/lib/clang/3.7.0/lib/darwin/libclang_rt.profile_osx.a',
      ],
    },
  }}]],
}
