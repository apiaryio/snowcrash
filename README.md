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
Snow Crash uses [Gyp](http://code.google.com/p/gyp/) build automation tool to generates its Makefiles.

1. If needed, install Gyp. Either from its package:
	
		$ sudo apt-get install gyp

	or from source:

		$ svn checkout http://gyp.googlecode.com/svn/trunk/ gyp
		$ cd ./gyp
		$ sudo python setup.py install	
		
	See [Gyp Hello World](https://github.com/springmeyer/hello-gyp) for a primer on installing and using Gyp.
	
2. Clone the repo + fetch the submodules:

		$ git clone git://github.com/apiaryio/snowcrash.git
		$ cd snowcrash
		$ git submodule update --init --recursive

3. Build & test Snow Crash:

		$ ./configure
		$ make test

		
### Build `snowcrash` command line tool
	
To build & run Snow Crash command line tool use:

	$ make snowcrash
	$ ./bin/snowcrash

## License
MIT License. See the [LICENSE](https://github.com/apiaryio/snowcrash/blob/master/LICENSE) file.
