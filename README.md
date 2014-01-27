![logo](https://raw.github.com/apiaryio/api-blueprint/gh-pages/assets/logo_apiblueprint.png)

# Snow Crash [![Build Status](https://travis-ci.org/apiaryio/snowcrash.png?branch=master)](https://travis-ci.org/apiaryio/snowcrash)

### API Blueprint Parser
Snow Crash is the reference [API Blueprint](http://apiblueprint.org) parser built on top of the [Sundown](https://github.com/vmg/sundown) Markdown parser.

API Blueprint is Web API documentation language. You can find API Blueprint documentation on the [API Blueprint site](http://apiblueprint.org).

## Status
- [Format 1A](https://github.com/apiaryio/api-blueprint/tree/format-1A) fully implemented 

## Bindings
Snow Crash bindings in other languages:

- [Protagonist](https://github.com/apiaryio/protagonist) (Node.js)
- [Snow Crash .NET](https://github.com/brutski/snowcrash-dot-net-wrapper) (.NET)

## Install
OS X using Homebrew:

```sh
$ brew install --HEAD \
  https://raw.github.com/apiaryio/snowcrash/master/tools/homebrew/snowcrash.rb
```

Other systems refer to [build notes](#build).

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

## AST
Parsed API Blueprint is available in the form of AST as defined in [Blueprint.h](src/Blueprint.h). The `snowcrash` command line tool offers AST serialization as defined in [API Blueprint AST Serialization Media Types](https://github.com/apiaryio/api-blueprint-ast).

## Contribute
Fork & Pull Request

If you want to create a binding for Snow Crash please refer to the [Writing a Binding](https://github.com/apiaryio/snowcrash/wiki/Writing-a-binding) article.

## License
MIT License. See the [LICENSE](https://github.com/apiaryio/snowcrash/blob/master/LICENSE) file.
