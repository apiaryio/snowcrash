Vagrant.configure("2") do |config|
  config.vm.box = "precise64"
  config.vm.box_url = "http://files.vagrantup.com/precise64.box"

  # VirtualBox
  config.vm.provider :virtualbox do |vb|
    vb.customize ["modifyvm", :id, "--memory", "2048"]
    vb.customize ["modifyvm", :id, "--cpus", "1"]  
  end

  # VMWare Fusion
  config.vm.provider :vmware_fusion do |vb|
    vb.vmx["memsize"] = "4096"
    vb.vmx["numvcpus"] = "1" 
  end

  config.vm.network :private_network, ip: "10.3.3.3"
  config.vm.synced_folder ".", "/vagrant", id: "vagrant-root", type: "nfs"
  config.vm.provision :shell, :path => "./provisioning.sh"
end
