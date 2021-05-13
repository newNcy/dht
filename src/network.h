#pragma once

#ifdef _WIN32
#include <winsock2.h>
#else
#endif

#include <stdint.h>
#include <string>
namespace network
{
    bool init();
    void release();

	class Ip4 
	{
		public:
			Ip4() {}
			Ip4(uint32_t value):ip(value) {}
			explicit Ip4(const std::string & str);
			operator uint32_t() const { return ip; }
			std::string toString() const;
		private:
			union 
			{
				uint32_t ip = 0;
				unsigned char bytes[4];
			};
	};

	Ip4 getip(const std::string & name);
	uint32_t sendto(int socketfd, Ip4 ip, unsigned port,const char * bytes, uint32_t length);
	uint32_t recvfrom(int socketfd, Ip4 ip, unsigned port, char * bytes, uint32_t length, sockaddr_in & addr);
}
