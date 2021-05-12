#include <cstdio>
#include "network.h"
#include "bencode.h"

class Krpc
{
};


int main (int argc, char * argv[])
{
    printf("ok\n");
    if (!network::init()) {
        return 0;
    }

    printf("ok\n");
    fflush(stdout);

    BValue bdict;
    BValue blist;
    bdict["a"] = "aa";
    printf("ok\n");
    fflush(stdout);
    bdict["b"] = 1;
    printf("ok\n");
    fflush(stdout);
    bdict["c"] = blist;
    printf("ok\n");
    fflush(stdout);
    bdict["d"] = bdict;

    printf("ok\n");
    fflush(stdout);
    blist.push(1);
    printf("ok\n");
    fflush(stdout);
    blist.push("dd");
    printf("ok\n");
    fflush(stdout);
    blist.push(blist);
    printf("ok\n");
    fflush(stdout);
    blist.push(bdict);

    printf("ok\n");
    fflush(stdout);
    printf("dict:\n%s\n list:\n%s\n", bdict.encode().c_str(), blist.encode().c_str());
    network::release();
    return 0;
}
