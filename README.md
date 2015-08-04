![logo](https://raw.github.com/apiaryio/api-blueprint/master/assets/logo_apiblueprint.png)

# Snow Crash [![Build Status](https://travis-ci.org/apiaryio/snowcrash.svg?branch=master)](https://travis-ci.org/apiaryio/snowcrash) [![Build status](https://ci.appveyor.com/api/projects/status/gupp1f7ymifxh3yn/branch/master?svg=true)](https://ci.appveyor.com/project/Apiary/snowcrash)


### API Blueprint Parser
Snow Crash is the reference [API Blueprint](http://apiblueprint.org) parser built on top of the [Sundown](https://github.com/vmg/sundown) Markdown parser.

API Blueprint is Web API documentation language. You can find API Blueprint documentation on the [API Blueprint site](http://apiblueprint.org).

## Status
- [Format 1A8](https://github.com/apiaryio/api-blueprint/releases/tag/format-1A8) fully implemented

## Use

### C++ library

```c++
#include "snowcrash.h"

mdp::ByteBuffer blueprint = R"(
# My API
## GET /message
+ Response 200 (text/plain)

        Hello World!
)";

snowcrash::ParseResult<snowcrash::Blueprint> ast;
snowcrash::parse(blueprint, 0, ast);

std::cout << "API Name: " << ast.node.name << std::endl;
```

Refer to [`Blueprint.h`](src/Blueprint.h) for the details about the Snow Crash AST and [`BlueprintSourcemap.h`](src/BlueprintSourcemap.h) for details about Source Maps tree.

### Command line tool

CLI was removed. It is replaced by utility named [drafter](https://github.com/apiaryio/drafter)

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

We love **Windows** too! Please refer to [Building on Windows](https://github.com/apiaryio/snowcrash/wiki/Building-on-Windows).


## Contribute
Fork & Pull Request

## License
MIT License. See the [LICENSE](https://github.com/apiaryio/snowcrash/blob/master/LICENSE) file.
