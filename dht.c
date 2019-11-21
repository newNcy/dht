#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <error.h>
#include <netdb.h>
#include <stdlib.h>

/*Please note the terminology used in this document to avoid confusion. A "peer" is a client/server listening on a TCP port that implements the BitTorrent protocol. A "node" is a client/server listening on a UDP port implementing the distributed hash table protocol. The DHT is composed of nodes and stores the location of peers. BitTorrent clients include a DHT node, which is used to contact other nodes in the DHT to get the location of peers to download from using the BitTorrent protocol.
 */


#define BIT_LEN 160
#define ID_LEN (BIT_LEN/8)
#define K 8
#define MSG_LEN_MAX 512

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef unsigned char byte_t;
typedef int32_t ip4_t;

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

byte_t rand_byte()
{
	return  rand() % (0xff+1);
}

void gen_node_id(byte_t * id)
{
	if (!id) return;
	for (int i = 0 ; i < ID_LEN; i++) {
		id[i] = rand_byte();
	}
}


int node_compare(node_info_t a, node_info_t b)
{
	for (int i = 0; i < ID_LEN; i++) {
	}
	return 0;
}

static const struct 
{
	char * host_name;
	unsigned short post;
}
boostrap_nodes [] =
{
	{ "router.bittorrent.com", 80}
};


void print_ip(ip4_t _ip)
{
	byte_t * ip = (byte_t*)&_ip;
	printf("%d.%d.%d.%d\n",
			ip[0],
			ip[1],
			ip[2],
			ip[3]
			);
}

void print_hex(byte_t * bytes, int len)
{
	for (int i = 0 ; i < len; i++) {
		printf("%02x", bytes[i]);
	}
	printf("\n");
}

void print_node_id(node_info_t node)
{
	print_hex(node.id, ID_LEN);
}

ip4_t get_ip_by_name(const char * name)
{
	struct hostent * host = gethostbyname(name);
	if (!host) return 0;	
	char * addr = host->h_addr_list[0];
	return *(ip4_t*)addr;
}


void dht_send(dht_t * this, node_info_t node, byte_t * buf, int len)
{
	sendto(this->socket_fd, buf,len, 0,  (sockaddr *)&this->info.addr,sizeof(node.addr)); 
}

int main (int argc, char * argv[])
{
	srand(time(0));
	int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	sockaddr_in my_addr;

	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = INADDR_ANY;
	my_addr.sin_port = htons(1224);

	int err = bind(socket_fd, (sockaddr*)&my_addr, sizeof(my_addr));
	if (err) {
		perror("bind");
		return 0;
	}

	ip4_t ip = get_ip_by_name(boostrap_nodes[0].host_name);
	print_ip(ip);

	node_info_t me;
	gen_node_id(me.id);
	print_node_id(me);

	byte_t find_me [MSG_LEN_MAX] = {0};
	sprintf(find_me, "d1:ad2:id20:abcdefghij01234567896:target20:%se1:q9:find_node1:t3:tx11:y1:qe", );

	return 0;
}
