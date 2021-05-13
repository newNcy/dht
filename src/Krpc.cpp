#include "Krpc.h"
#include <openssl/sha.h>
#include <thread>

Id Krpc::generateID(const std::string & msg)
{
	SHA_CTX s;
	int i, size;
	unsigned char hash[20];

	SHA1_Init(&s);
	SHA1_Update(&s, msg.c_str(), msg.length());
	SHA1_Final(hash, &s);

	Id id;
	id.fromBuffer(hash);
	return id;
}

Krpc::Krpc()
{
	self.id = generateID("crawler");
	socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(1224);

    bind (socketfd, (sockaddr*)&addr, sizeof(addr));
}
		
void Krpc::recvBack()
{
	
}

KrpcMessage Krpc::rpc(const KrpcMessage & msg, const Node & target)
{
	auto encodedMsg = msg.encode();
	printf("send to %s %d %d bytes\n ", network::Ip4(target.peer.ip).toString().c_str(), target.peer.port, encodedMsg.length());
    for (int i = 0 ; i < encodedMsg.length(); ++ i) {
        unsigned c = 0xff & encodedMsg[i];
        if (0x20 <= c && c <= 0x7e)
            printf(" %c ", c);
        else
            printf("%02x ", c);
    }
	fflush(stdout);
	network::sendto(socketfd, target.peer.ip, target.peer.port, encodedMsg.c_str(), encodedMsg.length());

	char buff[512] = {0};
	sockaddr_in  addr;
	int rc;
	for (;;) {
		rc = network::recvfrom(socketfd, target.peer.ip, target.peer.port, buff, 512, addr);
		if (rc) {
			break;
		}
	}

	KrpcMessage ret;
	int pos = 0;
	ret.decode(buff, pos);
	printf("recv %d %s\n", rc, buff);
	return ret;
}

KrpcMessage Krpc::findNode(Id targetId, const Node & toNode)
{
	KrpcMessage query;
	query["y"] = "q";
	query["t"] = "aa";
	query["q"] = "find_node";

	KrpcMessage a;
	a["id"] = self.id.toString();
	a["target"] = generateID().toString();

	query["a"] = a;
	return rpc(query, toNode);
}
