require 'aruba/cucumber'
require 'rbconfig'
include Config

Before do
  @dirs << "../../features/fixtures"
  
  case RbConfig::CONFIG['host_os']
    when /mswin|msys|mingw|cygwin|bccwin|wince|emc/
      ENV['PATH'] = "../../build/Release"
    else
      ENV['PATH'] = "./bin#{File::PATH_SEPARATOR}#{ENV['PATH']}"

end

end