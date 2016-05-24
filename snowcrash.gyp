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
      'direct_dependent_settings' : {
          'include_dirs': [
            'ext/markdown-parser/ext/sundown/src',
          ],
      },
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
      'direct_dependent_settings' : {
          'include_dirs': [
            'ext/markdown-parser/src',
          ],
      },
      'export_dependent_settings': [
          'libsundown'
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
      'direct_dependent_settings' : {
          'include_dirs': [
            'src',
          ],
      },
      'export_dependent_settings': [
          'libmarkdownparser'
      ],
      'sources': [
        'src/HTTP.cc',
        'src/HTTP.h',
        'src/MSON.cc',
        'src/MSONOneOfParser.cc',
        'src/MSONSourcemap.cc',
        'src/MSONTypeSectionParser.cc',
        'src/MSONValueMemberParser.cc',
        'src/Blueprint.cc',
        'src/BlueprintSourcemap.cc',
        'src/Section.cc',
        'src/Section.h',
        'src/Signature.cc',
        'src/Signature.h',
        'src/snowcrash.cc',
        'src/snowcrash.h',
        'src/UriTemplateParser.cc',
        'src/UriTemplateParser.h',
        'src/PayloadParser.h',
        'src/SectionParserData.h',
        'src/ActionParser.h',
        'src/AssetParser.h',
        'src/AttributesParser.h',
        'src/Blueprint.h',
        'src/BlueprintParser.h',
        'src/BlueprintSourcemap.h',
        'src/BlueprintUtility.h',
        'src/CodeBlockUtility.h',
        'src/DataStructureGroupParser.h',
        'src/HeadersParser.h',
        'src/HeadersParser.cc',
        'src/ModelTable.h',
        'src/MSON.h',
        'src/MSONSourcemap.h',
        'src/MSONMixinParser.h',
        'src/MSONNamedTypeParser.h',
        'src/MSONOneOfParser.h',
        'src/MSONParameterParser.h',
        'src/MSONPropertyMemberParser.h',
        'src/MSONTypeSectionParser.h',
        'src/MSONUtility.h',
        'src/MSONValueMemberParser.h',
        'src/ParameterParser.h',
        'src/ParametersParser.h',
        'src/Platform.h',
        'src/RegexMatch.h',
        'src/RelationParser.h',
        'src/ResourceGroupParser.h',
        'src/ResourceParser.h',
        'src/SectionParser.h',
        'src/SectionProcessor.h',
        'src/SignatureSectionProcessor.h',
        'src/SourceAnnotation.h',
        'src/StringUtility.h',
        'src/ValuesParser.h',
      ],
      'conditions': [
        [ 'OS=="win"',
          { 'sources': [ 'src/win/RegexMatch.cc' ] },
          { 'sources': [ 'src/posix/RegexMatch.cc' ] } # OS != Windows
        ]
      ],
      'dependencies': [
        'libmarkdownparser',
      ]
    },
    {
      'target_name': 'test-libsnowcrash',
      'type': 'executable',
      'include_dirs': [
        'test/vendor/Catch/include',
      ],
      'sources': [
        'test/test-ActionParser.cc',
        'test/test-AssetParser.cc',
        'test/test-AttributesParser.cc',
        'test/test-Blueprint.cc',
        'test/test-BlueprintParser.cc',
        'test/test-BlueprintUtility.cc',
        'test/test-DataStructureGroupParser.cc',
        'test/test-HeadersParser.cc',
        'test/test-Indentation.cc',
        'test/test-ModelTable.cc',
        'test/test-MSONMixinParser.cc',
        'test/test-MSONNamedTypeParser.cc',
        'test/test-MSONOneOfParser.cc',
        'test/test-MSONParameterParser.cc',
        'test/test-MSONPropertyMemberParser.cc',
        'test/test-MSONTypeSectionParser.cc',
        'test/test-MSONUtility.cc',
        'test/test-MSONValueMemberParser.cc',
        'test/test-ParameterParser.cc',
        'test/test-ParametersParser.cc',
        'test/test-PayloadParser.cc',
        'test/test-RegexMatch.cc',
        'test/test-RelationParser.cc',
        'test/test-ResourceParser.cc',
        'test/test-ResourceGroupParser.cc',
        'test/test-SectionParser.cc',
        'test/test-Signature.cc',
        'test/test-StringUtility.cc',
        'test/test-SymbolIdentifier.cc',
        'test/test-UriTemplateParser.cc',
        'test/test-ValuesParser.cc',
        'test/test-Warnings.cc',
        'test/test-snowcrash.cc'
      ],
      'dependencies': [
        'libsnowcrash',
      ]
    },
    {
      'target_name': 'perf-libsnowcrash',
      'type': 'executable',
      'sources': [
        'test/performance/perf-snowcrash.cc'
      ],
      'dependencies': [
        'libsnowcrash',
      ]
    }
  ]
}
