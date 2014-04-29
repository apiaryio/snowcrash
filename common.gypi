#
# Common build configuration
# ==========================
#
# GYP include file with common configuration settings.
#
# Attribution Notice:
# This work might use parts of Node.js `common.gypi` (https://github.com/joyent/node).
#
{
  'variables': {
    'target_arch%': 'ia32',
    'libsnowcrash_type%': 'static_library'
  },
  'target_defaults': {
    'defines': [ 
      'BUILDING_SNOWCRASH=1'
    ],
    'configurations': {
      'Debug': {
        'defines': [ 'DEBUG', '_DEBUG' ],
        'cflags': [ '-g', '-O0' ],
        'conditions': [
          ['target_arch=="x64"', {
            'msvs_configuration_platform': 'x64',
          }],
        ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'PreprocessorDefinitions': ['_SECURE_SCL=0', '_HAS_ITERATOR_DEBUGGING=0' ], # https://github.com/xbmc/xbmc/pull/1009
            'RuntimeLibrary': 3, # dll debug
            'Optimization': 0, # /Od, no optimization
            'MinimalRebuild': 'false',
            'OmitFramePointers': 'false',
            'BasicRuntimeChecks': 3, # /RTC1
            'RuntimeTypeInfo':  'true', # /GR, determine object type at runtime
            'DebugInformationFormat': 3, # /Zi, use program database as debugging information
            'ExceptionHandling': 1            
          },
          'VCLinkerTool': {
            'LinkIncremental': 2, # enable incremental linking
            'GenerateDebugInformation': 'true', # /DEBUG enable line by line debugging
            'SubSystem': 1 # /SUBSYSTEM:CONSOLE use console as native environment
          },
        },
        'xcode_settings': {
          'GCC_OPTIMIZATION_LEVEL': '0', # stop gyp from defaulting to -Os
        }
      },
      'Release': {
        'conditions': [
          ['target_arch=="x64"', {
            'msvs_configuration_platform': 'x64',
          }],
        ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 2, # dll release
            'Optimization': 0, # /Od, no optimizations
            'FavorSizeOrSpeed': 1, # /Ot, favour speed over size
            'InlineFunctionExpansion': 2, # /Ob2, inline anything eligible
            'WholeProgramOptimization': 'true', # /GL, whole program optimization, needed for LTCG
            'OmitFramePointers': 'true',
            'EnableFunctionLevelLinking': 'true',
            'EnableIntrinsicFunctions': 'true',
            'RuntimeTypeInfo': 'true', # /GR, determine object type at runtime
            'ExceptionHandling': 1,
            'AdditionalOptions': [
              '/MP', # compile across multiple CPUs
            ]
          },
          'VCLibrarianTool': {
            'AdditionalOptions': [
              '/LTCG', # link time code generation
            ],
          },
          'VCLinkerTool': {
            'LinkTimeCodeGeneration': 1, # link-time code generation
            'OptimizeReferences': 2, # /OPT:REF
            'EnableCOMDATFolding': 2, # /OPT:ICF
            'LinkIncremental': 1, # disable incremental linking
            'SubSystem': 1 # /SUBSYSTEM:CONSOLE use console as native environment
          }
        }
      }
    },
    'conditions': [
      ['OS == "win"', {
        'msvs_cygwin_shell': 0, # prevent actions from trying to use cygwin
        'defines': [
          'WIN32',
          # we don't really want VC++ warning us about
          # how dangerous C functions are...
          '_CRT_SECURE_NO_DEPRECATE',
          # ... or that C implementations shouldn't use
          # POSIX names
          '_CRT_NONSTDC_NO_DEPRECATE'
        ],
      }],
      [ 'OS in "linux freebsd openbsd solaris"', {
        'cflags': [ '-pthread', '-fPIC' ],
        'ldflags': [ '-pthread' ],
      }],
      [ 'OS in "linux freebsd openbsd solaris android"', {
        'cflags': [ '-Wall', '-Wextra', '-Wno-unused-parameter', ],
        'cflags_cc!': [ '-fno-rtti', '-fno-exceptions' ],
        'ldflags': [ '-rdynamic' ],
        'target_conditions': [
          ['_type=="static_library"', {
            'standalone_static_library': 1, # disable thin archive which needs binutils >= 2.19
          }],
        ],
        'conditions': [
          [ 'target_arch=="ia32"', {
            'cflags': [ '-m32' ],
            'ldflags': [ '-m32' ],
          }],
          [ 'target_arch=="x64"', {
            'cflags': [ '-m64' ],
            'ldflags': [ '-m64' ],
          }]
        ]
      }],
      ['OS=="mac"', {
        'defines': ['_DARWIN_USE_64_BIT_INODE=1'],        
        'xcode_settings': {
          'ALWAYS_SEARCH_USER_PATHS': 'NO',
          'GCC_CW_ASM_SYNTAX': 'NO',                # No -fasm-blocks
          'GCC_DYNAMIC_NO_PIC': 'NO',               # No -mdynamic-no-pic (Equivalent to -fPIC)
          'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',       # !-fno-exceptions
          'GCC_ENABLE_CPP_RTTI': 'YES',             # !-fno-rtti
          'GCC_ENABLE_PASCAL_STRINGS': 'NO',        # No -mpascal-strings
          'GCC_THREADSAFE_STATICS': 'NO',           # -fno-threadsafe-statics
          'PREBINDING': 'NO',                       # No -Wl,-prebind
          'MACOSX_DEPLOYMENT_TARGET': '10.5',       # -mmacosx-version-min=10.5
          'USE_HEADERMAP': 'NO',
          'OTHER_CFLAGS': [
            '-fno-strict-aliasing',
          ],
          'WARNING_CFLAGS': [
            '-Wall',
            '-Wendif-labels',
            '-W',
            '-Wno-unused-parameter',
          ],
        },
        'target_conditions': [
          ['_type!="static_library"', {
            'xcode_settings': {'OTHER_LDFLAGS': ['-Wl,-search_paths_first']},
          }],
        ],
        'conditions': [
          ['target_arch=="ia32"', {
            'xcode_settings': {'ARCHS': ['i386']},
          }],
          ['target_arch=="x64"', {
            'xcode_settings': {'ARCHS': ['x86_64']},
          }],
        ],
      }]
    ],
  }
}
