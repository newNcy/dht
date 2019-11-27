

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
#include <unistd.h>
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

int dht_on_ping(dht_t * this,krpc_t * krpc,  krpc_msg_t * msg)
{
	return 1;
}
int dht_on_find_node(dht_t * this, krpc_t * krpc, krpc_msg_t * msg)
{
	return 1;
}
int dht_on_get_peers(dht_t * this, krpc_t * krpc, krpc_msg_t * msg)
{
	return 1;
}
int dht_on_announce(dht_t * this, krpc_t * krpc, krpc_msg_t * msg)
{
	return 1;
}


int dht_on_ping_back(dht_t * this, krpc_t * krpc, krpc_msg_t * msg)
{
	debug("recv ping back");
	return 1;
}
int dht_on_find_node_back(dht_t * this, krpc_t * krpc, krpc_msg_t * msg)
{
	
	for (int i = 0; i < msg->r.nodes_count; i++) {
		compacked_node_info_t node = msg->r.nodes[i];
		buffer_stream_t log;
		buffer_stream_init(&log);
		buffer_stream_print_ip(&log, node.peer.ip);
		buffer_stream_printf(&log, ":%d", node.peer.port);
		debug("%s", log.buf);
		krpc_send_ping(krpc, this->info.id, node);
	}
	return 1;
}
int dht_on_get_peers_back(dht_t * this, krpc_t * krpc, krpc_msg_t * msg)
{
	return 1;
}
int dht_on_announce_back(dht_t * this, krpc_t * krpc, krpc_msg_t * msg)
{
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
	krpc.callback_owner = &dht;

	krpc.on_ping			= (krpc_callback_t)dht_on_ping;
	krpc.on_find_node		= (krpc_callback_t)dht_on_find_node;
	krpc.on_get_peers		= (krpc_callback_t)dht_on_get_peers;
	krpc.on_announce_peer	= (krpc_callback_t)dht_on_announce;
	
	krpc.on_ping_back			= (krpc_callback_t)dht_on_ping_back;
	krpc.on_find_node_back		= (krpc_callback_t)dht_on_find_node_back;
	krpc.on_get_peers_back		= (krpc_callback_t)dht_on_get_peers_back;
	krpc.on_announce_peer_back	= (krpc_callback_t)dht_on_announce_back;
	

	if (!ok) return 0;

	buffer_stream_t bs;
	buffer_stream_init(&bs);
	buffer_stream_print_hex(&bs, dht.info.id, ID_LEN);
	

	krpc_msg_t msg = {
		.y = _q,
		.q = _find_node,
	};
	strncpy((char*)msg.a.id, (char*)dht.info.id, ID_LEN);
	strncpy((char*)msg.a.info_hash, (char*)dht.info.id, ID_LEN);

	krpc_recv_loop(&krpc);
	
	for (int i = 0; i < sizeof(boostrap_nodes)/sizeof(boostrap_nodes[0]); i++) {
		compacked_node_info_t target;
		target.peer.ip = get_ip_by_name(boostrap_nodes[i].host_name);
		target.peer.port = htons(boostrap_nodes[i].port);
		krpc_send_find_node(&krpc, dht.info.id, dht.info.id, target);
	}
	while(1);
	return 0;
}
