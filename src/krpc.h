#pragma once
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "defs.h"
//////////////////////////////////////
//			krpc消息				//
//////////////////////////////////////
typedef struct
{
	byte_t node_id[ID_LEN];
	ip4_t ip;
	unsigned short port;
}compacked_node_info __attribute__((aligned(1)));


typedef struct 
{	
	unsigned short cur_t;
	char * msg_type[T_MAX];
}krpc_t;


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

void buffer_stream_init(buffer_stream_t * this)
{
	if (!this) return;
	this->w_pos = 0;
	this->r_pos = 0;
	this->len = MSG_LEN_MAX;
	memset(this->buf, 0, this->len);
}

int buffer_stream_scanf(buffer_stream_t * this, const char * f, ...)
{
	if (!this || !f) return 0;
	if (this->r_pos >= this->w_pos) return 0;
	va_list va; 
	va_start (va, f);       
	int ret = this->r_pos += vsscanf(this->buf + this->r_pos, f , va); 
	va_end(va); 
	return ret;

}

int buffer_stream_printf(buffer_stream_t * this,const char *f, ...)
{
	if (!this || !f) return 0;
	if (this->w_pos == this->len) return 0;
	va_list va; 
	va_start (va, f);       
	int ret = this->w_pos += vsnprintf(this->buf + this->w_pos, this->len, f, va); 
	va_end(va); 
	return ret;
}
void buffer_stream_print_hex(buffer_stream_t * this, byte_t * bytes, int len)
{
	if (!this || !bytes) return;
	for (int i = 0 ; i < len ; i ++) {
		buffer_stream_printf(this, "%02x", bytes[i]);
	}
}
void buffer_stream_printf_node_id(buffer_stream_t * this, byte_t * id)
{
	if (!this || !id) return;
	for (int i = 0; i < ID_LEN; i++) {
		buffer_stream_printf(this, "%c", id[i]);
	}
}
void buffer_stream_print_ip(buffer_stream_t * this, ip4_t _ip)
{
	byte_t * ip = (byte_t*)&_ip;
	buffer_stream_printf(this, "%d.%d.%d.%d",
			ip[0],
			ip[1],
			ip[2],
			ip[3]
			);
}


char buffer_stream_getch(buffer_stream_t * this)
{
	return this->buf[this->r_pos++];
}

int buffer_stream_read(buffer_stream_t * this, byte_t * buf, int len)
{
	if (!this || !buf || len <=0 || this->r_pos == this->w_pos) return 0;
	if (len > this->w_pos - this->r_pos) len = this->w_pos - this->r_pos;
	this->r_pos += len;

	memcpy(buf, this->buf, len);
	return len;
}

int buffer_stream_match(buffer_stream_t * this, char * prefix)
{
	if (!prefix) return 0;
	if (!this || this->r_pos >= this->w_pos) return 0;
	while (*prefix && this->r_pos < this->w_pos) {
		if (*prefix != this->buf[this->r_pos++]) return 0;
	}
	return 1;
}

int buffer_stream_value(buffer_stream_t * this)
{
	if (!this || this->r_pos > this->r_pos) return 0;
	return this->w_pos - this->r_pos;
}
void buffer_stream_dump_hex(buffer_stream_t * this)
{
	if (!this) return;
	printf("[%d]", this->w_pos);
	for (int i = 0; i < this->w_pos; i ++) {
		printf("%02x", 0xff & this->buf[i]);
	}
	printf("\n");
}

typedef struct
{
	byte_t id[ID_LEN]; //ping
	union 
	{
		byte_t target[ID_LEN]; //find_node
		struct 
		{
			byte_t info_hash[ID_LEN];	// get_peers
			struct {					//announce_peer
				unsigned short port;
				int token_len;
				char token[];
			};
		};
	};
}query_t;

typedef union 
{
	struct 
	{
		byte_t id[ID_LEN];
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



int bencode_query(krpc_msg_t msg, buffer_stream_t * bs)
{
	buffer_stream_printf(bs,"d1:ad2:id20:");
	buffer_stream_printf_node_id(bs, msg.a.id);
	if (msg.q == _ping) {

	}else if (msg.q == _find_node) {
		buffer_stream_printf(bs, "6:target20:");
		buffer_stream_printf_node_id(bs, msg.a.target);
		buffer_stream_printf(bs,"e");
	}else if (msg.q == _get_peers) {

	}else if (msg.q == _announce_peer) {

	}
	if (!msg.q) return 0;
	int op_len = strlen(msg.q);
	buffer_stream_printf(bs, "1:q%d:%s", op_len, msg.q);
	return 1;
}


int bdecode_response(buffer_stream_t * bs, krpc_msg_t * msg)
{
	if (!bs || !msg) return 0;
	if (!buffer_stream_match(bs, "d2:id20:")) {
		return 0;
	}

	byte_t id[ID_LEN] = {0};
	buffer_stream_read(bs, id, ID_LEN);
	
}

int bdecode(buffer_stream_t * bs, krpc_msg_t * msg)
{
	if (!bs || !msg) return 0;
	int tab = 0;
	if (!buffer_stream_match(bs, "d1:")) {
		return 0;
	}
	switch(buffer_stream_getch(bs)) {
		case 'r':
			msg->y = _r;
			bdecode_response(bs, msg);
			break;
	}
	return 0;
}

int bencode(krpc_msg_t msg, buffer_stream_t * bs)
{
	if (!bs) return 0;
	if (msg.y == _q) {
		bencode_query(msg, bs);
		buffer_stream_printf(bs, "1:t2:%c%c",((char*)&msg.t)[0], ((char*)&msg.t)[0]);
		buffer_stream_printf(bs,"1:y1:%se", msg.y);
	}else if (msg.y == _r) {
	}else if (msg.y == _e) {
	}

	return 1;
}


