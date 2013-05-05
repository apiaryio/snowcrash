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
        'src/BlockClassifier.cc',
        'src/BlockClassifier.h',
        'src/Blueprint.cc',
        'src/Blueprint.h',
        'src/BlueprintParser.cc',
        'src/BlueprintParser.h',
        'src/BlueprintParserCore.cc',
        'src/BlueprintParserCore.h',
        'src/MarkdownBlock.cc',
        'src/MarkdownBlock.h',
        'src/MarkdownParser.cc',
        'src/MarkdownParser.h',
        'src/MethodParser.cc',
        'src/MethodParser.h',
        'src/OverviewParser.cc',
        'src/OverviewParser.h',
        'src/Parser.cc',
        'src/Parser.h',
        'src/ParserCore.cc',
        'src/ParserCore.h',
        'src/RegexMatch.cc',
        'src/RegexMatch.h',
        'src/ResourceGroupParser.cc',
        'src/ResourceGroupParser.h',
        'src/ResourceParser.cc',
        'src/ResourceParser.h',
        'src/snowcrash',
        'src/snowcrash.cc',
        'src/snowcrash.h'
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
        'test/test-OverviewParser.cc',
        'test/test-Parser.cc',
        'test/test-RegexMatch.cc',
        'test/test-ResouceGroupParser.cc',
        'test/test-ResourceParser.cc',
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
        'src/snowcrash/SerializeJSON.cc',
        'src/snowcrash/SerializeYAML.cc',
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