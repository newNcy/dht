#include "network.h"

namespace network
{
    bool init()
    {
#ifdef _WIN32
        WSADATA data;
        return WSAStartup(MAKEWORD(2, 2), &data) == 0;
#endif
    }

    void release()
    {
#ifdef _WIN32
        WSACleanup();
#endif
    }

	Ip4::Ip4(const std::string & str) 
	{
		ip = inet_addr(str.c_str());
	}

	std::string Ip4::toString() const
	{
		char buff[16] = {0};
		sprintf(buff, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
		return buff;
	}

	Ip4 getip(const std::string & name)
	{
		auto host = gethostbyname(name.c_str());
		if (!host) {
			return 0;
		}
		return *(uint32_t*)host->h_addr_list[0];
	}

	sockaddr_in fillSockAddr(Ip4 ip, unsigned short port)
	{
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = ip;
		addr.sin_port = port;
		
		return addr;
	}

	uint32_t sendto(int socketfd, Ip4 ip, unsigned port, const char * bytes, uint32_t length)
	{

		auto addr = fillSockAddr(ip, port);

		return ::sendto(socketfd, bytes, length, 0, (sockaddr*)&addr, sizeof(addr));
	}

	uint32_t recvfrom(int socketfd, Ip4 ip, unsigned port,char * bytes, uint32_t length, sockaddr_in & addr)
	{
		int addrLen = sizeof(addr);
		return ::recvfrom(socketfd, bytes, length, 0, (sockaddr*)&addr, &addrLen);
	}
}
