#include <cstdio>
#include <unistd.h>
#include "network.h"
#include "bencode.h"
#include "Krpc.h"


int main (int argc, char * argv[])
{
    if (!network::init()) {
        return 0;
    }

	Krpc krpc;

	std::vector<Peer> boostrap = {
		{network::getip("dht.transmissionbt.com"), 6881},
		{network::getip("router.utorrent.com"), 6881},
		{network::getip("router.bittorrent.com"), 6881},
		{network::Ip4("160.120.47.92"), 6829},
	};
	for (auto & peer : boostrap) {
		Node node;
		node.peer = peer;
		auto response = krpc.findNode(krpc.getNode().id, node);
		if (!response.valid()) continue;

		auto r = response["r"];
		Id id;
		id.fromBuffer((unsigned char *)r["id"].getString().data());
		printf("id:%s\n", id.toHexString().c_str());

		auto ip = response["ip"];
		int iplen = 4;
		if (ip.type != BType::NONE) {
			iplen = ip.getString().length();
		}
		printf("ip len %d\n", iplen);
		auto nodes = r["nodes"].getString();
		int count = nodes.length()/(iplen + 2);
		printf("node package %d node count %d\n", nodes.length(), count);
		if (iplen == 4) {
			Peer * peers = (Peer*)(nodes.data());
			for (int i = 0 ; i < count; ++i) {
				printf("%s:%d\n", network::Ip4(peers[i].ip).toString().c_str(), ntohs(peers[i].port));
			}
		}
		printf("\n");

	}
	network::release();
	printf("exit\n");
	return 0;
}
