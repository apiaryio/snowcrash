# Snow Crash Development Tools

## Gyp
Snow Crash uses (bundled) [Gyp](https://gyp.gsrc.io/) build automation tool to
generate its Makefiles.

### Actual Bundled Version
[**SHA d84fed246cd42612c31a56a897e2d032e254a58a**](https://chromium.googlesource.com/external/gyp/+/d84fed246cd42612c31a56a897e2d032e254a58a)

### Update
1. Download the latest stable version of Gyp from its [repository](https://chromium.googlesource.com/external/gyp/+/refs/heads/master)

2. Remove unnecessary files:

	```sh
	$ rm -rf .svn
	$ rm -rf test
	```

3. Update bundled version in this README

### More info
See [Gyp Hello World](https://github.com/springmeyer/hello-gyp) for a primer on
using Gyp.
