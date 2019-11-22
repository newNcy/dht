#pragma once
#include "defs.h"
#include <time.h>
typedef struct 
{
	byte_t id[ID_LEN];
	sockaddr_in addr;
	time_t last_touch;
}node_info_t;

typedef struct bucket_t
{
	node_info_t nodes[K];
	time_t last_change;
	struct bucket_t * next;
}bucket_t;

typedef struct 
{
	bucket_t buckets[BIT_LEN];
}table_t;


typedef struct 
{
	node_info_t info;
	int socket_fd;
	table_t table;
}dht_t;



