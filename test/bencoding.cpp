#include "bencode.h"

int main ()
{
	BValue i = 12;
	BValue str = "abc";

	printf("int:%s\nstr:%s\n", i.encode().c_str(),str.encode().c_str());

    BValue bdict = 1;
    BValue blist;
    bdict["a"] = "aa";
    bdict["b"] = 1;
    bdict["c"] = blist;
    bdict["d"] = bdict;

    blist.push(1);
    blist.push("dd");
    blist.push(bdict);
    blist.push(blist);

    printf("dict:%s\nlist:%s\n", bdict.encode().c_str(), blist.encode().c_str());
	return 0;
}
