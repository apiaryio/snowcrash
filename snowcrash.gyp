{
  'includes': [
    'common.gypi',
  ],

  'targets' : [
    {
      'target_name': 'sundown',
      'type': 'static_library',
      'include_dirs': [
        'sundown/src',
        'sundown/html',
      ],
      'sources': [
        'sundown/src/autolink.c',
        'sundown/src/buffer.c',
        'sundown/html/houdini_href_e.c',
        'sundown/html/houdini_html_e.c',
        'sundown/html/html.c',
        'sundown/html/html_smartypants.c',
        'sundown/src/markdown.c',
        'sundown/src/stack.c',
        'sundown/src/src_map.c'
      ]
    },

    {
      'target_name': 'libsnowcrash',
      'type': 'static_library',
      'include_dirs': [
        'src',
        'sundown/src',
        'sundown/src/html'
      ],
      'sources': [
        'src/Blueprint.cc',
        'src/BlueprintParser.cc',
        'src/MarkdownBlock.cc',
        'src/MarkdownParser.cc',
        'src/Parser.cc',
        'src/snowcrash.cc'
      ],
    },

    {
      'target_name': 'test-snowcrash',
      'type': 'executable',
      'include_dirs': [
        'src',
        'test',
        'test/vendor/Catch/include',
        'sundown/src',
        'sundown/src/html'
      ],
      'sources': [
        'test/test-Blueprint.cc',
        'test/test-BlueprintParser.cc',
        'test/test-MarkdownBlock.cc',
        'test/test-MarkdownParser.cc',
        'test/test-Parser.cc',
        'test/test-snowcrash.cc'
      ],
      'dependencies': [
        'libsnowcrash',
        'sundown'
      ],
      'ldflags': [
        '-stdlib=libstdc++'
      ],
      'xcode_settings': {
        'OTHER_LDFLAGS': [
          '-stdlib=libstdc++'
        ]
      }
    },

    {
      'target_name': 'snowcrash',
      'type': 'executable',
      'include_dirs': [
        'src',
        'src/snowcrash'
      ],
      'sources': [
        'src/snowcrash/snowcrash.cc',
        'src/snowcrash/Serialize.cc'
      ],
      'dependencies': [
        'libsnowcrash',
        'sundown'
      ],
      'ldflags': [
        '-stdlib=libstdc++'
      ],
      'xcode_settings': {
        'OTHER_LDFLAGS': [
          '-stdlib=libstdc++'
        ]
      }
    }    
  ]
}