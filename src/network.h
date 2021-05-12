#pragma once

#ifdef _WIN32
#include <winsock2.h>
#else
#endif

namespace network
{
    bool init();
    void release();
}
