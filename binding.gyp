{
  'variables': {
    'addon_cpp_std%': 'c++20',
    'module_name%': 'addon',
    'module_path%': './lib/binding/',
  },
  'targets': [
    {
      'target_name': '<(module_name)',
      'type': 'loadable_module',
      'dependencies': [
        # https://github.com/nodejs/node-addon-api/blob/main/doc/setup.md
        # using exceptions (instead of maybe, like we used to have on nan)
        "<!(node -p \"require('node-addon-api').targets\"):node_addon_api_except",
      ],
      'sources': [
        'src/addon.cc',
        'src/ReproduceIssue.cc',
      ],
      'include_dirs': [
        '<!@(node -p "require(\'node-addon-api\').include")',
      ],
      'defines': [
        'NAPI_VERSION=8',
      ],
      'conditions': [
        ['OS=="win"', {
          'msvs_settings': {
            'VCCLCompilerTool': {
              'AdditionalOptions': [
                '/std:<(addon_cpp_std)',
                '/MP', #compile across multiple CPUs
              ],
            },
            'VCLinkerTool': {
              'GenerateDebugInformation': 'true',
            },
          },
          'configurations': {
            'Release': {
              'msvs_settings': {
                'VCCLCompilerTool': {
                  'ExceptionHandling': '1',
                  'Optimization': 2,                  # /O2 safe optimization
                  'FavorSizeOrSpeed': 1,              # /Ot, favour speed over size
                  'InlineFunctionExpansion': 2,       # /Ob2, inline anything eligible
                  'WholeProgramOptimization': 'true', # /GL, whole program optimization, needed for LTCG
                  'OmitFramePointers': 'true',
                  'EnableFunctionLevelLinking': 'true',
                  'EnableIntrinsicFunctions': 'true',
                  'WarnAsError': 'true',
                }
              }
            },
            'Debug': {
              'msvs_settings': {
                'VCCLCompilerTool': {
                  'WarnAsError': 'false'
                }
              }
            }
          },
        }, { # OS != "win"
            # Use level 2 optimizations
          'cflags': [
            '-O2',
          ],
          'cflags_cc': [
            '-O2',
            '-std=<(addon_cpp_std)',
            '-Wno-narrowing',
          ],
            # Allow C++ exceptions
            # Disable level 3 optimizations
          'cflags!': [
            '-fno-exceptions',
            '-O3'
          ],
          'cflags_cc!': [
            '-fno-exceptions',
            '-O3'
          ],
        }],
        ['OS=="mac"', {
          'cflags+': ['-fvisibility=hidden'],
          'configurations': {
            'Debug': {
              'xcode_settings': {
                'GCC_GENERATE_DEBUGGING_SYMBOLS': 'YES',
                'GCC_OPTIMIZATION_LEVEL': '0',
                'DEAD_CODE_STRIPPING': 'NO',
                'GCC_INLINES_ARE_PRIVATE_EXTERN': 'NO',
                'GCC_SYMBOLS_PRIVATE_EXTERN': 'NO'
              },
              'cflags_cc': ['-g', '-O0', '-fno-omit-frame-pointer'],
              'cflags': ['-g', '-O0', '-fno-omit-frame-pointer']
            },
            'Release': {
              'xcode_settings': {
                'GCC_OPTIMIZATION_LEVEL': 's',
                'DEAD_CODE_STRIPPING': 'YES'
              }
            }
          },
          'xcode_settings': {
            'OTHER_CPLUSPLUSFLAGS': [
              '-std=<(addon_cpp_std)', '-stdlib=libc++',
            ],
            'OTHER_LDFLAGS': [
              '-Wl,-bind_at_load',
              '-stdlib=libc++'
            ],
            'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES', # -fvisibility=hidden
            'GCC_ENABLE_CPP_RTTI': 'YES',
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'MACOSX_DEPLOYMENT_TARGET': '11.6',
            'CLANG_CXX_LIBRARY': 'libc++',
            'CLANG_CXX_LANGUAGE_STANDARD': '<(addon_cpp_std)',
            'WARNING_CFLAGS': [
              '-Wno-c++11-narrowing',
              '-Wno-constant-conversion'
            ],
          },
        }],
      ]
    },
    {
      'target_name': 'action_after_build',
      'type': 'none',
      'dependencies': ['<(module_name)'],
      'copies': [
        {
          'files': ['<(PRODUCT_DIR)/<(module_name).node'],
          'destination': '<(module_path)'
        }
      ],
    }
  ]
}
