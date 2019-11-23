

#include <arpa/inet.h>
#include <stdint.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <error.h>
#include <netdb.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include "dht.h"



byte_t rand_byte()
{
	return  rand() % (0xff+1);
}

void gen_node_id(byte_t * id)
{
	/*long s = rand();
	SHA1((byte_t*)&s, ID_LEN, id);
	return;
	*/
	if (!id) return;
	for (int i = 0 ; i < ID_LEN; i++) {
		id[i] = rand_byte();
	}
}

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

void print_node_id(compacked_node_info_t node)
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

int dht_init(dht_t * this)
{
	if (!this) return 0;

	gen_node_id(this->info.id);
	return 1;
}

///////////////////////////////////////

void on_ctrl_c(int sig)
{
	if (sig == SIGINT) {
		printf("exit ...\n");
		exit(0);
	}
}

void handle_signal()
{
	signal(SIGINT, on_ctrl_c);
}
static const struct 
{
	char * host_name;
	unsigned short port;
}
boostrap_nodes [] =
{
	{ "dht.transmissionbt.com", 6881},
};


int main (int argc, char * argv[])
{
	srand(time(0));
	handle_signal();

	dht_t dht;
	int ok = dht_init(&dht);
	if (!ok) return 0;

	krpc_t krpc;
	ok = krpc_init(&krpc, 1224);
	if (!ok) return 0;

	buffer_stream_t bs;
	buffer_stream_init(&bs);


	buffer_stream_print_hex(&bs, dht.info.id, ID_LEN);

	ip4_t ip = get_ip_by_name(boostrap_nodes[0].host_name);
	compacked_node_info_t target;
	target.peer.ip= ip;
	target.peer.port = htons(boostrap_nodes[0].port);



	krpc_msg_t msg = {
		.y = _q,
		.q = _find_node,
	};
	strncpy((char*)msg.a.id, (char*)dht.info.id, ID_LEN);
	strncpy((char*)msg.a.target, (char*)dht.info.id, ID_LEN);

	krpc_send(&krpc, &msg, target);
	while(1) {
		krpc_msg_t * ret = krpc_recv(&krpc);
	}
	return 0;
}
