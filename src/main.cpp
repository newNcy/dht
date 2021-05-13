#include <cstdio>
#include "network.h"
#include "bencode.h"
#include "Krpc.h"


int main (int argc, char * argv[])
{
    if (!network::init()) {
        return 0;
    }

	Krpc krpc;

	auto ip4 = network::getip("dht.transmissionbt.com");
	printf("boostrap ip %s\n", ip4.toString().c_str());

	Node boostrap;
	boostrap.peer.ip = ip4;
	boostrap.peer.port = 6881;

	for (;;)
	{
		auto response = krpc.findNode(krpc.getNode().id, boostrap);
	}
	network::release();
	printf("exit\n");
	return 0;
}
