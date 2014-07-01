{
  "includes": [
    "common.gypi"
  ],
  
  'targets' : [
    {
      'target_name': 'libsundown',
      'type': 'static_library',
      'include_dirs': [
        'ext/markdown-parser/ext/sundown/src',
        'ext/markdown-parser/ext/sundown/html'
      ],
      'sources': [
        'ext/markdown-parser/ext/sundown/src/autolink.c',
        'ext/markdown-parser/ext/sundown/src/buffer.c',
        'ext/markdown-parser/ext/sundown/src/markdown.c',
        'ext/markdown-parser/ext/sundown/src/src_map.c',
        'ext/markdown-parser/ext/sundown/src/stack.c',
        'ext/markdown-parser/ext/sundown/html/houdini_href_e.c',
        'ext/markdown-parser/ext/sundown/html/houdini_html_e.c',
        'ext/markdown-parser/ext/sundown/html/html.c',
        'ext/markdown-parser/ext/sundown/html/html_smartypants.c'
      ]
    },
    {
      'target_name': 'libmarkdownparser',
      'type': 'static_library',
      'include_dirs': [
        'ext/markdown-parser/ext/sundown/src',
        'ext/markdown-parser/ext/sundown/html'
        'ext/markdown-parser/src',
      ],
      'sources': [
        'ext/markdown-parser/src/ByteBuffer.cc',
        'ext/markdown-parser/src/MarkdownNode.cc',
        'ext/markdown-parser/src/MarkdownParser.cc'
      ],
      'dependencies': [
          'libsundown'
      ]      
    },    
    {
      'target_name': 'libsnowcrash',
      'type': '<(libsnowcrash_type)',
      'include_dirs': [
        'src',
        'ext/markdown-parser/src',
        'ext/markdown-parser/ext/sundown/src',
        'ext/markdown-parser/ext/sundown/html'
      ],
      'sources': [
        'src/HTTP.cc',
        'src/Section.cc',
        'src/Serialize.cc',
        'src/SerializeJSON.cc',
        'src/SerializeYAML.cc',
        'src/Signature.cc',
        'src/UriParser.cc',
        'src/snowcrash.cc'
      ],
      'conditions': [
        [ 'OS=="win"', 
          { 'sources': [ 'src/win/RegexMatch.cc' ] }, 
          { 'sources': [ 'src/posix/RegexMatch.cc' ] } # OS != Windows
        ]
      ],
      'dependencies': [
          'libmarkdownparser'
      ]
    },
    {
      'target_name': 'test-libsnowcrash',
      'type': 'executable',
      'include_dirs': [
        'src',
        'test',
        'test/vendor/Catch/include',
        'ext/markdown-parser/src',
        'ext/markdown-parser/ext/sundown/src',
        'ext/markdown-parser/ext/sundown/html'
      ],
      'sources': [
        'test/test-ActionParser.cc',
        'test/test-AssetParser.cc',
        'test/test-HeadersParser.cc',
        'test/test-ParameterParser.cc',
        'test/test-ParametersParser.cc',
        'test/test-PayloadParser.cc',
        'test/test-ResourceParser.cc',
        'test/test-ValuesParser.cc',
        'test/test-Blueprint.cc',
        'test/test-SectionParser',
        'test/test-snowcrash.cc'
      ],
      'dependencies': [
        'libsnowcrash',
        'libmarkdownparser'
      ]
    },

    {
      'target_name': 'snowcrash',
      'type': 'executable',
      'include_dirs': [
        'src',
        'src/snowcrash',
        'cmdline'
      ],
      'sources': [
        'src/snowcrash/snowcrash.cc'
      ],
      'dependencies': [
        'libsnowcrash',
        'libmarkdownparser'
      ]
    }
  ],
  'conditions': [
    ['OS in "mac linux"', {
      'targets': [
        {
          'target_name': 'perf-libsnowcrash',
          'type': 'executable',
          'include_dirs': [
            'src',
            'cmdline',
            'test',
            'test/performance',
          ],
          'sources': [
            'test/performance/perf-snowcrash.cc'
          ],
          'dependencies': [
            'libsnowcrash',
            'libmarkdownparser'
          ]
        }
      ]
    }]
  ]  
}
