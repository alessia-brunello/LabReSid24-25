from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import Node
from mininet.cli import CLI
from mininet.log import setLogLevel
from mininet.link import TCLink

class GenericRouter(Node):

	def config(self,**params):
		super(GenericRouter, self).config(**params)
		self.cmd('sysctl net.ipv4.ip_forward=1')

	def terminate(self):
		self.cmd('sysctl net.ipv4.ip_forward=0')
		super(GenericRouter, self).terminate()

class RouterTopo(Topo):

	def build(self):
		h1 = self.addHost('h1', ip='10.0.1.1/24')
		h2 = self.addHost('h2', ip='10.0.2.1/24')
	
		r1 = self.addNode('r1', cls=GenericRouter)
		
		self.addLink(h1, r1, intfName2='r1-eth0', cls=TCLink,
				bw=100,delay='75ms')

		self.addLink(h2, r1, intfName2='r1-eth1', cls=TCLink,
				bw=100,delay='75ms')

def run():
	topo = RouterTopo()
	net = Mininet(topo=topo, controller=None, link=TCLink)
	net.start()

	h1 = net.get('h1')
	h2 = net.get('h2')

	r1 = net.get('r1')

	#Configurazione router

	r1.cmd('ip addr add 10.0.1.254/24 dev r1-eth0')
	r1.cmd('ip addr add 10.0.2.254/24 dev r1-eth1')

	#Default gateway host
	h1.cmd('ip route add default via 10.0.1.254')
	h2.cmd('ip route add default via 10.0.2.254')

	CLI(net)
	net.stop()
if __name__ == '__main__':
	setLogLevel('info')
	run() 

