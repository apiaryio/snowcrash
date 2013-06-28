# Snow Crash [![Build Status](https://travis-ci.org/apiaryio/snowcrash.png?branch=master)](https://travis-ci.org/apiaryio/snowcrash)

### API Blueprint Parser
Snow Crash is the reference [API Blueprint](http://apiblueprint.org) parser built on top of the [Sundown](https://github.com/vmg/sundown) Markdown parser.

API Blueprint is Web API documentation language. You can find API Blueprint documentation on the [API Blueprint site](http://apiblueprint.org).

## Status
Snow Crash is Work in Progress. See its [Wiki status page](https://github.com/apiaryio/snowcrash/wiki/API-Blueprint-Implementation-Status) for details.

## Bindings
Snow Crash bindings in other languages:

* [Protagonist](https://github.com/apiaryio/protagonist) (Node.js)

## Build
1. Clone the repo + fetch the submodules:

	```sh
	$ git clone git://github.com/apiaryio/snowcrash.git
	$ cd snowcrash
	$ git submodule update --init --recursive
	```

2. Build & test Snow Crash:

	```sh
	$ ./configure
	$ make test
	```
		
### Build `snowcrash` command line tool
To build the Snow Crash command line tool use:
```sh
$ make snowcrash
```

To install and use the Snow Crash command line tool globally use:

```sh
$ make install
$ snowcrash --help
usage: snowcrash [options] ... <input file>

API Blueprint Parser
If called without <input file>, `snowcrash` will listen on stdin.

options:
  -o, --output      save output AST into file (string [=])
  -f, --format      output AST format (string [=yaml])
  -h, --help        display this help message
  -v, --validate    validate input only, do not print AST
```

## Contribute
Fork & Pull Request. 

Make sure to follow the [API Blueprint Developers discussion group](https://groups.google.com/forum/#!forum/apiblueprint-dev). Also check the Snow Crash [issues page](https://github.com/apiaryio/snowcrash/issues?state=open).

## License
MIT License. See the [LICENSE](https://github.com/apiaryio/snowcrash/blob/master/LICENSE) file.
