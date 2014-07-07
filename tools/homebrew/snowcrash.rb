require 'formula'

class Snowcrash < Formula
  homepage 'http://apiblueprint.org'
  head 'https://github.com/apiaryio/snowcrash.git', :tag => 'v0.12.0'

  def install
    system "./configure"
    system "make", "install"
  end
end
