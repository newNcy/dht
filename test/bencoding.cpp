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

	BValue query;
	query["t"] = "aa";
	query["y"] = "q";
	query["q"] = "ping";
	BValue body;
	body["id"] = "abcdefghij0123456789";
	query["a"] = body;

	BValue response;
	int pos = 0;

	response.decode("i123456e", pos);
	pos = 0;
	response.decode("3:ddd", pos);
	pos = 0;
	response.decode("l3:ddde", pos);
	pos = 0;
	response.decode("d1:rd2:id20:mnopqrstuvwxyz123456e1:t2:aa1:y1:re", pos);

	printf("query:%s\n", query.encode().c_str());
	printf("response:%s\n", response.encode().c_str());
	return 0;
}
