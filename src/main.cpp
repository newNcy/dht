#include <cstdio>
#include "network.h"
#include "bencode.h"

class Krpc
{
};


int main (int argc, char * argv[])
{
    if (!network::init()) {
        return 0;
    }


    network::release();
	printf("exit\n");
    return 0;
}
