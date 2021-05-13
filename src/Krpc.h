#pragma once
#include <cstring>
#include "bencode.h"
#include "network.h"


struct Id
{
	enum { LEN = 20 };
	unsigned char bytes[LEN] = {0};
	Id operator ^ (const Id & rhs) 
	{
		Id id;
		for (int i = 0; i < LEN; ++i) {
			id.bytes[i] = bytes[i] ^ rhs.bytes[i];
		}
		return id;
	}
	std::string toHexString() const 
	{
		char buff[41] = {0};
		for (int i = 0; i < LEN; ++ i) {
			sprintf(buff+i*2, "%02x", bytes[i]);
		}
		return buff;
	}

	std::string toString() const 
	{
		return std::string((char *)(bytes), LEN);
	}

	void fromBuffer(unsigned char * buffer)
	{
		memcpy(bytes, buffer, LEN);
	}

};

typedef uint32_t PODIp;
struct Peer
{
	PODIp ip;
	unsigned short port;
}__attribute__((packed));

struct Node
{
	Id id;
	Peer peer;
}__attribute__((packed));


using KrpcMessage = BValue;

/*
 *
 */
class Krpc
{
	public:
		static Id generateID(const std::string & msg = "");
		Krpc();
		KrpcMessage ping(const Node & target);
		KrpcMessage findNode(Id targetId, const Node & nextNode);
		Node & getNode() { return self; }
	private:
		Node self;
		int socketfd = -1;
		KrpcMessage rpc(const KrpcMessage & msg, const Node & target);
		void recvBack();
};

