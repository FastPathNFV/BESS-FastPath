# BESS-FastPath
BESS-FastPath is a [BESS](https://github.com/NetSys/bess) system integrated with FastPath. BESS-FastPath do not change the commands in bessctl and formats in bess files, original BESS users could use this system as usual.  Users could also run NFs easliy with following actions(mainliy copied from original page [Build and Install BESS](https://github.com/NetSys/bess/wiki/Build-and-Install-BESS))
## Usage
### Host configuration
BESS runs best under Ubuntu 16.04 LTS with Linux kernel 4.4.0 x86_64. It's easiest to start with a fresh install of the operating system.
#### Clone BESS
Once you have your host configured, go ahead and clone the BESS codebase and cd to the new directory:
```bash
git clone https://github.com/FastPathNFV/BESS-FastPath.git
cd BESS-FastPath/
```
#### Install dependencies
BESS conveniently comes with all of its dependencies bundled in an Ansible script -- all you have to do is run the script and it will set everything up. A warning: the script will aggressively replace any conflicting packages (this is why we recommend a fresh install). Here's what to run in your shell:
```bash
sudo apt-get install -y software-properties-common
sudo apt-add-repository -y ppa:ansible/ansible
sudo apt-get update
sudo apt-get install -y ansible
ansible-playbook -K -t package -i localhost, -c local env/bess.yml
sudo reboot
```
#### Start hugepages
Every time you reboot the system, you have to set up hugepages. Note that this step is not just for installation! Every time you reboot and want to use bess, you need to run the following:
```bash
# For single-node systems
sudo sysctl vm.nr_hugepages=1024

# For multi-node (NUMA) systems
echo 1024 | sudo tee /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
echo 1024 | sudo tee /sys/devices/system/node/node1/hugepages/hugepages-2048kB/nr_hugepages
```
#### And build!
Return to your ```BESS-FastPath/``` directory (the downloaded, cloned repository) and run ```./build.py```. This should build BESS for you.

### Start up BESS and run a sample configuration
Assuming building BESS was successful, you should now be ready to use bessctl to start up a bessd switch and configure it. Here's what to do from your bess/ directory:
```
cd bessctl/
./bessctl
```
You will now enter a command line interface. You should see an error saying that no bess daemon is running -- here's how to start bessd:
```text
> daemon start
```
You should now see a command prompt that says localhost:10514. This means that you have successfully started a bessd instance (which is currently forwarding no packets) and that your bessctl instance is communicating with bessd over a local socket. To make your bessd instance forward some packets, try running a sample script!
```text
> run sample/flowgen
> monitor pipeline
```
```run samples/flowgen```starts up a sample configuration of modules. bessd is not connected to any network interface cards or VMs so it creates its own packets to forward using a Source module; it then forwards them through an ACL module which filters out half of the packets (those that match a blacklisted term). ```monitor pipeline``` is a quick way to see how packets are flowing in your bessd configuration -- you can always type monitor pipeline to see all of the modules and ports in the system and how many packets per second are flowing through them (**Note: command ```monitor pipeline``` will not show packet number handled in Fast-Path**).

## Development
We have developed several sample NFs (network functions, or modules in BESS) including ACL, NAT, Load Balance, Monitor (Flow Counter) and Snort. Developers can easily modify existing modules into FastPath paradigm (in foder ```core/modules/```). 

During runtime, every NF acts as an module in the service chain, which needs to register a per-flow processing rule (containing Header Actions and State Functions). Then FastPath consolidate these rules together and build a logic-equivalent fast data path.

