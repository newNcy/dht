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
}
