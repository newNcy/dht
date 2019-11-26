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


typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef unsigned char byte_t;
typedef int32_t ip4_t;

#define BIT_LEN 160
#define ID_LEN (BIT_LEN/8)
#define K 8
#define MSG_LEN_MAX 512
#define debug(f, ...) fprintf(stdout, "[debug] -- " f "\n", ##__VA_ARGS__)
#define T_MAX 0xffff
#define TOKEN_LEN_MAX 32
//////////////////////////////////////
//			krpc消息				//
//////////////////////////////////////


char * const _q;
char * const _r;
char * const _e;
char * const _failed;

char * const _ping;
char * const _find_node;
char * const _get_peers;
char * const _announce_peer;

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
	byte_t id[ID_LEN];
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
	byte_t id[ID_LEN]; //ping
	union 
	{
		byte_t target[ID_LEN]; //find_node
		struct 
		{
			byte_t info_hash[ID_LEN];	// get_peers
			struct 
			{					//announce_peer
				int implied_port;
				unsigned short port;
				byte_t token[TOKEN_LEN_MAX];
			};
		};
	};
}query_t;

typedef	struct 
{
	byte_t id[ID_LEN]; //ping ret
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
	unsigned short t;
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



typedef int (*krpc_callback_t) (void * owner, krpc_msg_t * msg);

typedef struct 
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
void krpc_send(krpc_t * this, krpc_msg_t * msg, compacked_node_info_t target);
void krpc_recv(krpc_t * this);
void krpc_recv_loop(krpc_t * this);
