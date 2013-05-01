{
  'variables': {
    'target_arch%': 'ia32',
  },

  'target_defaults': {
    'cflags_cc': [ 
      '-stdlib=libstdc++' 
    ],    
    'cppflags': [ 
      '-stdlib=libc++'
    ],
    'xcode_settings': {
      'OTHER_CPLUSPLUSFLAGS': [
        '-stdlib=libc++'
      ]
    }
  }
}
