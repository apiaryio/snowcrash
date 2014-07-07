![logo](https://raw.github.com/apiaryio/api-blueprint/master/assets/logo_apiblueprint.png)

# Snow Crash [![Build Status](https://travis-ci.org/apiaryio/snowcrash.png?branch=master)](https://travis-ci.org/apiaryio/snowcrash)

### API Blueprint Parser
Snow Crash is the reference [API Blueprint](http://apiblueprint.org) parser built on top of the [Sundown](https://github.com/vmg/sundown) Markdown parser.

API Blueprint is Web API documentation language. You can find API Blueprint documentation on the [API Blueprint site](http://apiblueprint.org).

## Status
- [Format 1A6](https://github.com/apiaryio/api-blueprint/releases/tag/format-1A6) fully implemented 

## Install
OS X using Homebrew:

```sh
$ brew install --HEAD \
  https://raw.github.com/apiaryio/snowcrash/master/tools/homebrew/snowcrash.rb
```

Other systems refer to [build notes](#build).

## Use

### C++ library

```c++
#include "snowcrash.h"

snowcrash::SourceData blueprint = R"(
# My API
## GET /message
+ Response 200 (text/plain)

        Hello World!
)";
snowcrash::Result result;
snowcrash::Blueprint ast;
snowcrash::parse(blueprint, 0, result, ast);

std::cout << "API Name: " << ast.name << std::endl;
```

Refer to [`Blueprint.h`](src/Blueprint.h) for the details about the Snow Crash AST. See [Snow Crash bindings](#bindings) for using the library in **other languages**. 

### Command line tool

```bash
$ cat << 'EOF' > blueprint.apib
# My API
## GET /message
+ Response 200 (text/plain)

        Hello World!
EOF

$ snowcrash blueprint.apib 
_version: 2.0
metadata:
name: "My API"
 ...
```

Refer to [AST Serialization Media Types](https://github.com/apiaryio/api-blueprint-ast) for the details on serialized media types. See [parse feature](features/parse.feature) for the details on using the `snowcrash` command line tool.

## Build
1. Clone the repo + fetch the submodules:

	```sh
	$ git clone --recursive git://github.com/apiaryio/snowcrash.git
	$ cd snowcrash
	```

2. Build & test Snow Crash:

	```sh
	$ ./configure
	$ make test
	```

	To include integration tests (using Cucumber) use the `--include-integration-tests` flag: 

	```sh
	$ ./configure --include-integration-tests
	$ make test
	```
	
We love **Windows** too! Please refer to [Building on Windows](https://github.com/apiaryio/snowcrash/wiki/Building-on-Windows).
		
### Snow Crash command line tool
1. Build `snowcrash`:
	
	```sh
	$ make snowcrash
	```

2. Install & use `snowcrash`:

	```sh
	$ sudo make install
	$ snowcrash --help
	```

## Bindings
Snow Crash bindings in other languages:

- [Protagonist](https://github.com/apiaryio/protagonist) (Node.js)
- [Snow Crash .NET](https://github.com/brutski/snowcrash-dot-net-wrapper) (.NET)


## Contribute
Fork & Pull Request

If you want to create a binding for Snow Crash please refer to the [Writing a Binding](https://github.com/apiaryio/snowcrash/wiki/Writing-a-binding) article.

## License
MIT License. See the [LICENSE](https://github.com/apiaryio/snowcrash/blob/master/LICENSE) file.
