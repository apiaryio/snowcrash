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
        'src/ActionParser.h',      
        'src/Blueprint.h',
        'src/BlueprintParser.h',
        'src/BlueprintParserCore.h',
        'src/BlueprintUtility.h',
        'src/HeaderParser.h',
        'src/HTTP.h',
        'src/HTTP.cc',
        'src/ListUtility.h',
        'src/MarkdownBlock.cc',
        'src/MarkdownBlock.h',
        'src/MarkdownParser.cc',
        'src/MarkdownParser.h',
        'src/ParameterDefinitonParser.h',
        'src/ParametersParser.h',
        'src/Parser.cc',
        'src/Parser.h',
        'src/ParserCore.cc',
        'src/ParserCore.h',
        'src/PayloadParser.h',
        'src/RegexMatch.h',
        'src/ResourceGroupParser.h',
        'src/ResourceParser.h',
        'src/Serialize.cc',
        'src/Serialize.h',
        'src/SerializeJSON.cc',
        'src/SerializeJSON.h',
        'src/SerializeYAML.cc',
        'src/SerializeYAML.h',
        'src/StringUtility.h',
        'src/snowcrash.cc',
        'src/snowcrash.h',
        'src/SourceAnnotation.h',
        'src/SymbolTable.h'
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
        'test/test-ActionParser.cc',
        'test/test-AssetParser.cc',
        'test/test-Blueprint.cc',
        'test/test-BlueprintParser.cc',
        'test/test-HeaderParser.cc',
        'test/test-ListUtility.cc ',
        'test/test-MarkdownBlock.cc',
        'test/test-MarkdownParser.cc',
        'test/test-Parser.cc',
        'test/test-PayloadParser.cc',
        'test/test-ParameterDefinitonParser.cc',
        'test/test-ParametersParser.cc',
        'test/test-RegexMatch.cc',
        'test/test-ResouceGroupParser.cc',
        'test/test-ResourceParser.cc',
        'test/test-SymbolTable.cc',
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
  ]
}
