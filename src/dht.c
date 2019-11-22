

#include <arpa/inet.h>
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
#include "defs.h"
#include "dht.h"
#include "krpc.h"
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


int node_compare(node_info_t a, node_info_t b)
{
	for (int i = 0; i < ID_LEN; i++) {
	}
	return 0;
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


int dht_send(dht_t * this, node_info_t target, byte_t * buf, int len)
{
	buffer_stream_t bs;
	buffer_stream_init(&bs);
	buffer_stream_print_ip(&bs, target.addr.sin_addr.s_addr);

	int sc = sendto(this->socket_fd, buf,len, 0,  (sockaddr *)&target.addr,sizeof(target.addr)); 
	//debug("sendto %s:%d size:%d", bs.buf, ntohs(target.addr.sin_port), sc);
	return sc;
}

int dht_recv(dht_t * this,buffer_stream_t * bs)
{
	buffer_stream_init(bs);
	sockaddr_in remote;
	socklen_t len = sizeof(remote);
	int rc = recvfrom(this->socket_fd, bs->buf, bs->len, 0, (sockaddr*)&remote,&len);

	buffer_stream_t log;
	buffer_stream_init(&log);
	buffer_stream_printf(&log, "recvfrom ");
	buffer_stream_print_ip(&log, remote.sin_addr.s_addr);
	buffer_stream_printf(&log, ":%d", ntohs(remote.sin_port));

	debug("%s", log.buf);
	return rc;
}

void * dht_recv_loop(dht_t * this)
{	
	buffer_stream_t bs;
	while (1) {
		buffer_stream_init(&bs);
		sockaddr_in remote;
		socklen_t len = sizeof(remote);
		int count = recvfrom(this->socket_fd, bs.buf, bs.len, 0, (sockaddr*)&remote, &len);
		debug("recvfrom %s:%d size:%d", inet_ntoa(remote.sin_addr),ntohs(remote.sin_port), count);
		bs.w_pos = count;
		bdecode(&bs, NULL);
	}
	return NULL;
}

void dht_start_recv(dht_t * this, int inback)
{
	if (inback) {
		typedef void *(*start_route_t)(void *);
		pthread_t thread_id;
		int err = pthread_create(&thread_id, NULL, (start_route_t)dht_recv_loop, this);
		if (err) {
			perror("create thread");
			return;
		}
		debug("thread--%ld-- linstning ...", thread_id);
	}else {
		dht_recv_loop(this);
	}
}


int dht_init(dht_t * this, unsigned short port)
{
	if (!this) return 0;
	this->socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (this->socket_fd < 0) {
		perror("socket");
		return 0;
	}

	this->info.addr.sin_family = AF_INET;
	this->info.addr.sin_addr.s_addr = INADDR_ANY;
	this->info.addr.sin_port = htons(port);

	int err = bind(this->socket_fd, (sockaddr*)&this->info.addr, sizeof(this->info.addr));
	if (err) {
		perror("bind");
		return 0;
	}

	/*
	int flag = fcntl(this->socket_fd, F_GETFL,0);
	if (flag < 0) perror("get flag");
	else fcntl(this->socket_fd, F_SETFL , flag | O_NONBLOCK);
	*/

	gen_node_id(this->info.id);
	return 1;
}

int dht_send_krpc(dht_t * this,node_info_t target,  krpc_msg_t msg)
{
	buffer_stream_t bs;
	buffer_stream_init(&bs);

	bencode(msg, &bs);
	if (!bs.w_pos) return 0;

	dht_send(this, target, (byte_t*)bs.buf, bs.w_pos);
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

int main (int argc, char * argv[])
{
	srand(time(0));
	handle_signal();


	dht_t dht;
	int ok = dht_init(&dht, 1224);
	if (!ok) return 0;

	buffer_stream_t bs;
	buffer_stream_init(&bs);
	buffer_stream_print_hex(&bs, dht.info.id, ID_LEN);

	printf("%s\n", bs.buf);
	static const struct 
	{
		char * host_name;
		unsigned short post;
	}
	boostrap_nodes [] =
	{
		{ "dht.transmissionbt.com", 6881},
	};

	ip4_t ip = get_ip_by_name(boostrap_nodes[0].host_name);

	node_info_t target;
	target.addr.sin_family = AF_INET;
	target.addr.sin_addr.s_addr = ip;
	target.addr.sin_port = htons(boostrap_nodes[0].post);



	krpc_msg_t msg = {
		.t = 1224,
		.y = _q,
		.q = _find_node,
		.a = { 0 }
	};
	strncpy((char*)msg.a.id, (char*)dht.info.id, ID_LEN);
	strncpy((char*)msg.a.target, (char*)dht.info.id, ID_LEN);

	dht_send_krpc(&dht, target, msg); 
	dht_start_recv(&dht, 1);
	while (1) {
	}
	return 0;
}
