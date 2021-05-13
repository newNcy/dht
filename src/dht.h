#pragma once
#include <sys/types.h>
#include <time.h>
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/cdefs.h>
#include <arpa/inet.h>

#define BIT_LEN 160
#define ID_LEN (BIT_LEN/8)
#define K 8
#define MSG_LEN_MAX 512
#define debug(f, ...) fprintf(stdout, "[debug] -- " f "\n", ##__VA_ARGS__)
#define T_MAX 0xffff
#define T_LEN_MAX 32
#define TOKEN_LEN_MAX 32


static void * _new_operator(unsigned long size)
{
	void * ret = malloc(size);
	memset(ret, 0, size);
	return ret;
}
#define new(Type) (_new_operator(sizeof(Type)))

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef unsigned char byte_t;
typedef byte_t node_id_t[ID_LEN];
typedef node_id_t info_hash_t;
typedef int32_t ip4_t;
//////////////////////////////////////
//			krpc消息				//
//////////////////////////////////////


static char * const _q		= "q";
static char * const _r		= "r";
static char * const _e		= "e";
static char * const _failed = "failed";

static char * const _ping			= "ping";
static char * const _find_node		= "find_node";
static char * const _get_peers		= "get_peers";
static char * const _announce_peer	= "announce_peer";

typedef struct
{
	char buf[MSG_LEN_MAX];
	int r_pos;
	int w_pos;
	int len;
}buffer_stream_t;



typedef struct 
{
	ip4_t ip;
	unsigned short port;
}__attribute__((packed)) compacked_peer_info_t ;

typedef struct //in network byte order
{
	node_id_t id;	
	compacked_peer_info_t peer;
}__attribute__((packed)) compacked_node_info_t ;


typedef struct bucket_t
{
	compacked_node_info_t nodes[K];
	time_t last_change;
	struct bucket_t * next;
}bucket_t;

typedef struct 
{
	bucket_t buckets[BIT_LEN];
}table_t;


typedef struct 
{
	compacked_node_info_t info;
	table_t table;
}dht_t;




typedef struct
{
	node_id_t id;
	union 
	{
		node_id_t target;
		struct 
		{
			info_hash_t info_hash;	
			struct 
			{					//announce_peer
				int implied_port;
				unsigned short port;
				int token_len;
				byte_t token[TOKEN_LEN_MAX];
			};
		};
	};
}query_t;

typedef	struct 
{
	node_id_t id;
	int token_len;
	byte_t token[TOKEN_LEN_MAX];
	int with_peers;
	union 
	{
		struct 
		{
			int nodes_count;
			compacked_node_info_t nodes[];
		};
		struct 
		{
			int peers_count;
			compacked_peer_info_t peers[];
		};
	};
}response_t;

typedef struct
{
}error_t;


typedef struct 
{
	int t_len;
	byte_t t[T_LEN_MAX];
	char * y;
	union 
	{
		struct {
			char * q;
			query_t a;
		};
		response_t r;
		error_t e;
	};
}krpc_msg_t;


struct  krpc_t;

typedef int (*krpc_callback_t) (void * owner, struct krpc_t * krpc, krpc_msg_t * msg);

typedef struct  krpc_t
{	
	int socket_fd;
	unsigned short cur_t;
	char * msg_type[T_MAX];
	void * callback_owner;
	
	krpc_callback_t on_ping;
	krpc_callback_t on_find_node;
	krpc_callback_t on_get_peers;
	krpc_callback_t on_announce_peer;

	krpc_callback_t on_ping_back;
	krpc_callback_t on_find_node_back;
	krpc_callback_t on_get_peers_back;
	krpc_callback_t on_announce_peer_back;

}krpc_t; 



void buffer_stream_init(buffer_stream_t * this);
int buffer_stream_printf(buffer_stream_t * this,const char *f, ...);
void buffer_stream_print_hex(buffer_stream_t * this, byte_t * bytes, int len);

void buffer_stream_printf_node_id(buffer_stream_t * this, byte_t * id);
void buffer_stream_print_ip(buffer_stream_t * this, ip4_t _ip);
char buffer_stream_getch(buffer_stream_t * this);
int buffer_stream_get_int(buffer_stream_t * this);
int buffer_stream_read(buffer_stream_t * this, byte_t * buf, int len);
int buffer_stream_match(buffer_stream_t * this, char * prefix);
int buffer_stream_value(buffer_stream_t * this);
void buffer_stream_dump(buffer_stream_t * this);


//////////////////////////////////////
//				krpc接口			//
//////////////////////////////////////
int krpc_init(krpc_t * this, unsigned short port);

void krpc_send_ping(krpc_t * this, node_id_t id, compacked_node_info_t target);
void krpc_send_find_node(krpc_t * this, node_id_t id, node_id_t target_id, compacked_node_info_t target); 
void krpc_send_get_peers(krpc_t * this, node_id_t id, info_hash_t info_hash, compacked_node_info_t target);
void krpc_send_announce_peer(
		krpc_t * this,
		node_id_t id,
		int implied_port,
		info_hash_t info_hash,
		unsigned short port,
		byte_t * token,
		int token_len,
		compacked_node_info_t target
		);
void krpc_send_ping_back(krpc_t * this, node_id_t id, compacked_node_info_t target);
void krpc_send_find_node_back(krpc_t * this, node_id_t id, compacked_node_info_t *nodes, int nodes_count, compacked_node_info_t target); 

void krpc_send_get_peers_back_with_nodes(krpc_t * this, node_id_t id, compacked_node_info_t * nodes,int nodes_count, compacked_node_info_t target );

void krpc_send_get_peers_back_with_peers(krpc_t * this, node_id_t id, compacked_peer_info_t * peers,int peers_count, compacked_node_info_t target );

void krpc_send_announce_peer_back(
		krpc_t * this,
		node_id_t id,
		int implied_port,
		info_hash_t info_hash,
		unsigned short port,
		byte_t * token,
		int token_len
		);


void krpc_send(krpc_t * this, krpc_msg_t * msg, compacked_node_info_t target);
void krpc_recv(krpc_t * this);
void krpc_recv_loop(krpc_t * this);
