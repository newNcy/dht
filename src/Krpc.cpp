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
	socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketfd < 0) {
		perror("socket");
	}

#ifdef _WIN32
	int imode=1;  
	ioctlsocket(socketfd, FIONBIO,(u_long *)&imode); 
#else
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
    if (setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
        perror("setsockopt failed:");
    }
#endif

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(1224);

    bind (socketfd, (sockaddr*)&addr, sizeof(addr));
	//std::thread(&Krpc::recvBack, this).detach();
}
		
void Krpc::recvBack()
{
	sockaddr_in  addr;
	int rc;
	for (;;) {
		char buff[512] = {0};
		rc = network::recvfrom(socketfd, buff, 512, addr);
		printf("recv %d %s\n", rc, buff);
		fflush(stdout);
	}
}

void print_packet(const char * encodedMsg, int max)
{
	for (int i = 0 ; i < max; ++ i) {
        unsigned c = 0xff & encodedMsg[i];
        if (0x20 <= c && c <= 0x7e)
            printf(" %c ", c);
        else
            printf("%02x ", c);
		if ((i+1)%20 == 0) 
			printf("\n");
    }
	printf("\n");
}

KrpcMessage Krpc::rpc(const KrpcMessage & msg, const Node & target)
{
	auto encodedMsg = msg.encode();
	int sc = network::sendto(socketfd, target.peer.ip, target.peer.port, encodedMsg.c_str(), encodedMsg.length());
	printf("send to %s %d %d bytes\n", network::Ip4(target.peer.ip).toString().c_str(), target.peer.port, sc);
	print_packet(encodedMsg.c_str(), encodedMsg.length());

	
	KrpcMessage ret;
	int pos = 0;
	sockaddr_in  addr;
	int rc;
	for (;;) {
		char buff[512] = {0};
		rc = network::recvfrom(socketfd, buff, 512, addr);
		printf("recv %d bytes from %s:%d\n", rc, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		print_packet(buff, rc);
		if (rc) {
			if (rc > 0) 
				ret.decode(std::string(buff, rc), pos);
			break;
		}
		fflush(stdout);
	}


	fflush(stdout);
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
