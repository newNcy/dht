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


    BValue bdict;
    BValue blist;
    bdict["a"] = "aa";
    bdict["b"] = 1;
    bdict["c"] = blist;
    bdict["d"] = bdict;

    blist.push(1);
    blist.push("dd");
    blist.push(blist);
    blist.push(bdict);

    network::release();
    return 0;
}
