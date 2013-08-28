# Snow Crash [![Build Status](https://travis-ci.org/apiaryio/snowcrash.png?branch=master)](https://travis-ci.org/apiaryio/snowcrash)

### API Blueprint Parser
Snow Crash is the reference [API Blueprint](http://apiblueprint.org) parser built on top of the [Sundown](https://github.com/vmg/sundown) Markdown parser.

API Blueprint is Web API documentation language. You can find API Blueprint documentation on the [API Blueprint site](http://apiblueprint.org).

## Status
Snow Crash is currently Work in Progress. See its [v1.0 Milestone](https://github.com/apiaryio/snowcrash/issues?milestone=1&state=open) for implemetnation status details.

## Bindings
Snow Crash bindings in other languages:

- [Protagonist](https://github.com/apiaryio/protagonist) (Node.js)
- [snowcrashCLR](https://github.com/SeriousM/snowcrashCLR) (.NET)

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
	$ git clone git://github.com/apiaryio/snowcrash.git
	$ cd snowcrash
	$ git submodule update --init --recursive
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

## Contribute
Fork & Pull Request. 

Make sure to follow the [API Blueprint Developers discussion group](https://groups.google.com/forum/#!forum/apiblueprint-dev). Also check the Snow Crash [issues page](https://github.com/apiaryio/snowcrash/issues?state=open).

If you want to create a binding for Snow Crash please refer to the [Writing a binding](https://github.com/apiaryio/snowcrash/wiki/Writing-a-binding) Wiki article.

## License
MIT License. See the [LICENSE](https://github.com/apiaryio/snowcrash/blob/master/LICENSE) file.
