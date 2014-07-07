{
  "includes": [
    "common.gypi"
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
      'type': '<(libsnowcrash_type)',
      'include_dirs': [
        'src',
        'sundown/src',
        'sundown/src/html'
      ],
      'sources': [
        'src/HTTP.cc',
        'src/MarkdownBlock.cc',
        'src/MarkdownParser.cc',
        'src/Parser.cc',
        'src/ParserCore.cc',
        'src/RegexMatch.h',
        'src/Serialize.cc',
        'src/Serialize.h',
        'src/SerializeJSON.cc',
        'src/SerializeYAML.cc',
        'src/UriTemplateParser.cc',
        'src/snowcrash.cc',
        'src/csnowcrash.cc',
        'src/CBlueprint.cc',
        'src/CSourceAnnotation.cc'
      ],
      'conditions': [
        [ 'OS=="win"', 
          { 'sources': [ 'src/win/RegexMatch.cc' ] }, 
          { 'sources': [ 'src/posix/RegexMatch.cc' ] } # OS != Windows
        ]
      ],
      'dependencies': [
          'sundown'
      ]
    },
    {
      'target_name': 'test-libsnowcrash',
      'type': 'executable',
      'include_dirs': [
        'src',
        'test',
        'test/vendor/Catch/include',
        'sundown/src',
        'sundown/src/html'
      ],
      'sources': [
        'test/test-ActionParser.cc',
        'test/test-AssetParser.cc',
        'test/test-Blueprint.cc',
        'test/test-BlueprintParser.cc',
        'test/test-HeaderParser.cc',
        'test/test-Indentation.cc',
        'test/test-ListUtility.cc',
        'test/test-MarkdownBlock.cc',
        'test/test-MarkdownParser.cc',
        'test/test-ParameterDefinitonParser.cc',
        'test/test-ParametersParser.cc',
        'test/test-Parser.cc',
        'test/test-PayloadParser.cc',
        'test/test-RegexMatch.cc',
        'test/test-ResouceGroupParser.cc',
        'test/test-ResourceParser.cc',
        'test/test-SymbolIdentifier.cc',
        'test/test-SymbolTable.cc',
        'test/test-Warnings.cc',
        'test/test-csnowcrash.cc',
        'test/test-UriTemplateParser.cc',
        'test/test-snowcrash.cc'
      ],
      'dependencies': [
        'libsnowcrash',
        'sundown'
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
        'sundown'
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
            'sundown'
          ]
        }
      ]
    }]
  ]  
}
