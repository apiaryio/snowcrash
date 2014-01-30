# Snow Crash Development Tools

## Gyp
Snow Crash uses (bundled) [Gyp](http://code.google.com/p/gyp/) build automation tool to generates its Makefiles. 

### Actual Bundled Version
**r1846**

### Update
1. Get the latest version from Gyp's repository:
	
	```sh
	$ svn checkout http://gyp.googlecode.com/svn/trunk/ gyp
	```

2. Remove unnecessary files:

	```sh
	$ cd gyp
	$ rm -rf .svn
	$ rm -rf test
	```

3. Update bundled version in this README

### More info
See [Gyp Hello World](https://github.com/springmeyer/hello-gyp) for a primer on using Gyp.
