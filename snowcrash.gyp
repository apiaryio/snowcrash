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
        'ext/markdown-parser/ext/sundown/src/autolink.h',
        'ext/markdown-parser/ext/sundown/src/buffer.c',
        'ext/markdown-parser/ext/sundown/src/buffer.h',
        'ext/markdown-parser/ext/sundown/src/html_blocks.h',
        'ext/markdown-parser/ext/sundown/src/markdown.c',
        'ext/markdown-parser/ext/sundown/src/markdown.h',
        'ext/markdown-parser/ext/sundown/src/src_map.c',
        'ext/markdown-parser/ext/sundown/src/src_map.h',
        'ext/markdown-parser/ext/sundown/src/stack.c',
        'ext/markdown-parser/ext/sundown/src/stack.h',
        'ext/markdown-parser/ext/sundown/html/houdini.h',
        'ext/markdown-parser/ext/sundown/html/houdini_href_e.c',
        'ext/markdown-parser/ext/sundown/html/houdini_html_e.c',
        'ext/markdown-parser/ext/sundown/html/html.c',
        'ext/markdown-parser/ext/sundown/html/html.h',
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
        'ext/markdown-parser/src/ByteBuffer.h',
        'ext/markdown-parser/src/MarkdownNode.cc',
        'ext/markdown-parser/src/MarkdownNode.h',
        'ext/markdown-parser/src/MarkdownParser.cc',
        'ext/markdown-parser/src/MarkdownParser.h'
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
        'src/CBlueprint.cc',
        'src/CBlueprint.h',
        'src/CBlueprintSourcemap.cc',
        'src/CBlueprintSourcemap.h',
        'src/CSourceAnnotation.cc',
        'src/CSourceAnnotation.h',
        'src/HTTP.cc',
        'src/HTTP.h',
        'src/Section.cc',
        'src/Section.h',
        'src/Serialize.cc',
        'src/Serialize.h',
        'src/SerializeJSON.cc',
        'src/SerializeJSON.h',
        'src/SerializeYAML.cc',
        'src/SerializeYAML.h',
        'src/Signature.cc',
        'src/Signature.h',
        'src/snowcrash.cc',
        'src/snowcrash.h',
        'src/csnowcrash.cc',
        'src/csnowcrash.h',
        'src/UriTemplateParser.cc',
        'src/UriTemplateParser.h',
        'src/PayloadParser.h',
        'src/SectionParserData.h',
        'src/ActionParser.h',
        'src/AssetParser.h',
        'src/Blueprint.h',
        'src/BlueprintParser.h',
        'src/BlueprintSourcemap.h',
        'src/BlueprintUtility.h',
        'src/CodeBlockUtility.h',
        'src/HeadersParser.h',
        'src/MSON.h',
        'src/MSONSectionProcessor.h',
        'src/ParameterParser.h',
        'src/ParametersParser.h',
        'src/Platform.h',
        'src/RegexMatch.h',
        'src/ResourceGroupParser.h',
        'src/ResourceParser.h',
        'src/SectionParser.h',
        'src/SectionProcessor.h',
        'src/SourceAnnotation.h',
        'src/StringUtility.h',
        'src/SymbolTable.h',
        'src/ValuesParser.h',
        'src/Version.h'
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
        'test/test-Blueprint.cc',
        'test/test-BlueprintParser.cc',
        'test/test-HeadersParser.cc',
        'test/test-Indentation.cc',
        'test/test-ParameterParser.cc',
        'test/test-ParametersParser.cc',
        'test/test-PayloadParser.cc',
        'test/test-RegexMatch.cc',
        'test/test-ResourceParser.cc',
        'test/test-ResourceGroupParser.cc',
        'test/test-SectionParser.cc',
        'test/test-Signature.cc',
        'test/test-SymbolIdentifier.cc',
        'test/test-SymbolTable.cc',
        'test/test-UriTemplateParser.cc',
        'test/test-ValuesParser.cc',
        'test/test-Warnings.cc',
        'test/test-csnowcrash.cc',
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
        'ext/markdown-parser/src',
        'ext/markdown-parser/ext/sundown/src',
        'ext/cmdline'
      ],
      'sources': [
        'src/snowcrash/snowcrash.cc'
      ],
      'dependencies': [
        'libsnowcrash',
        'libmarkdownparser'
      ]
    },
    {
      'target_name': 'perf-libsnowcrash',
      'type': 'executable',
      'include_dirs': [
        'src',
        'ext/markdown-parser/src',
        'ext/markdown-parser/ext/sundown/src',
        'ext/cmdline',
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
}
