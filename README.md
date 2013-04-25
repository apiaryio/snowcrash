# Snow Crash – API Blueprint Parser

Snow Crash is **[Apiary](http://Apiary.io) API Blueprint Parser** built on top of [Sundown](https://github.com/vmg/sundown) Markdown parser.

## Bindings
* [Apiary Blueprint Parser](https://github.com/apiaryio/blueprint-parser) (Node.js)

## Build
Snow Crash uses [Gyp](http://code.google.com/p/gyp/) build automation tool to generates its Makefile. Before proceeding you need to have Gyp installed on your system. See [Gyp Hello World](https://github.com/springmeyer/hello-gyp) for a primer on installing and using Gyp. You can also use Gyp bundled with [node-gyp](https://github.com/TooTallNate/node-gyp/tree/master/gyp).
	
1. Clone the repo + fetch the submodules:
	```
	$ git clone git@github.com:apiaryio/snowcrash.git
	$ cd snowcrash
	$ git submodule update
	```

2. Get the gyp build tool working:
	```
	$ svn checkout http://gyp.googlecode.com/svn/trunk/ gyp
	$ cd ./gyp
	$ sudo python setup.py install
	```

3. Build Snow Crash:
	```
	$ ./configure
	$ make
	$ make test
	```

## License
See LICENSE file.
