require 'aruba/cucumber'

Before do
  @dirs << "../../features/fixtures"
  
  ENV['PATH'] = "./bin#{File::PATH_SEPARATOR}#{ENV['PATH']}"  
end