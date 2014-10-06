require 'formula'

class Snowcrash < Formula
  homepage 'http://apiblueprint.org'
  head 'https://github.com/apiaryio/snowcrash.git', :tag => 'v0.15.0'

  def install
    system "./configure"
    system "make", "snowcrash"
    bin.install Dir["bin/snowcrash"]
  end
end
