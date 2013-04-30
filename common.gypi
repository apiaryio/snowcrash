{
  'variables': {
    'target_arch%': 'ia32',
  },

  'target_defaults': {
    'cppflags': [ 
      '-std=c++11',
      '-stdlib=libc++' 
    ],
    'cflags_cc': [ 
      '-std=c++11',
      '-stdlib=libc++' 
    ],
    'xcode_settings': {
      'OTHER_CPLUSPLUSFLAGS': [
        '-std=c++11',
        '-stdlib=libc++'
      ]
    }
  }
}
