Vagrant.configure("2") do |config|
  config.vm.box = "precise64"
  config.vm.box_url = "http://files.vagrantup.com/precise64.box"

  cpus = "1"
  if ENV['VAGRANT_CPUS']
    cpus = ENV['VAGRANT_CPUS']
  end

  memory = "4096"
  if ENV['VAGRANT_MEMORY']
    memory = ENV['VAGRANT_MEMORY']
  end

  # VirtualBox
  config.vm.provider :virtualbox do |vb|
    vb.customize ["modifyvm", :id, "--memory", memory]
    vb.customize ["modifyvm", :id, "--cpus", cpus]
  end

  # VMWare Fusion
  config.vm.provider :vmware_fusion do |vb|
    vb.vmx["memsize"] = memory
    vb.vmx["numvcpus"] = cpus
  end

  config.vm.network :private_network, ip: "10.3.3.3"
  config.vm.synced_folder ".", "/vagrant", id: "vagrant-root", type: "nfs"
  config.vm.provision :shell, :path => "./provisioning.sh"
end
