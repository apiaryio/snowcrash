{
  'variables': {
    'target_arch%': 'ia32',
  },

  'target_defaults': {
    'cppflags': [ 
      '-std=gnu++11',
      '-stdlib=libc++' 
    ],
    'xcode_settings': {
      'OTHER_CPLUSPLUSFLAGS': [
        '-std=gnu++11',
        '-stdlib=libc++'
      ]
    }
  }
}
