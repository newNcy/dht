#include "dht.h"
/*
extern char * const _q;
extern char * const _r;
extern char * const _e;
extern char * const _failed;

extern char * const _ping;
extern char * const _find_node;
extern char * const _get_peers;
extern char * const _announce_peer;

*/
void buffer_stream_init(buffer_stream_t * this)
{
	if (!this) return;
	this->w_pos = 0;
	this->r_pos = 0;
	this->len = MSG_LEN_MAX;
	memset(this->buf, 0, this->len);
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

int buffer_stream_get_int(buffer_stream_t * this)
{
	if (!this) return 0;

	int ret = atoi(this->buf + this->r_pos);
	for (;;) {
		char c = buffer_stream_getch(this);
		if ( '0' > c || c > '9') {
			this->r_pos --;
			break;
		} 
	}

	return ret;
}


int buffer_stream_read(buffer_stream_t * this, byte_t * buf, int len)
{
	if (!this || !buf || len <=0 || this->r_pos == this->w_pos) return 0;
	if (len > this->w_pos - this->r_pos) len = this->w_pos - this->r_pos;
	this->r_pos += len;

	memcpy(buf, this->buf, len);
	return len;
}

typedef enum {
	MATCH_MOVE,
	MATCH_PEEK,
	MATCH_MOVE_IF
}match_type_t;

int buffer_stream_match(buffer_stream_t * this, char * prefix)
{
	if (!prefix) return 0;
	if (!this || this->r_pos >= this->w_pos) return 0;

	int pos = this->r_pos;
	while (*prefix && pos < this->w_pos) {
		if (*prefix++ != this->buf[pos++]) {
			return 0;
		}
	}
	this->r_pos = pos;
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

int bencode_query(krpc_msg_t * msg, buffer_stream_t * bs)
{
	buffer_stream_printf(bs,"d1:ad2:id20:");
	buffer_stream_printf_node_id(bs, msg->a.id);

	if (msg->q == _ping) {

	}else if (msg->q == _find_node) {
		buffer_stream_printf(bs, "6:target20:");
		buffer_stream_printf_node_id(bs, msg->a.target);
		buffer_stream_printf(bs,"e");
	}else if (msg->q == _get_peers) {

	}else if (msg->q == _announce_peer) {

	}
	if (!msg->q) return 0;
	int op_len = strlen(msg->q);
	buffer_stream_printf(bs, "1:q%d:%s", op_len, msg->q);
	return 1;
}


krpc_msg_t * bdecode_response(buffer_stream_t * bs, krpc_msg_t * msg)
{
	if (!bs || !msg) return 0;
	if (!buffer_stream_match(bs, "d2:id20:")) {
		return 0;
	}

	byte_t id[ID_LEN] = {0};
	buffer_stream_read(bs, id, ID_LEN);

}

krpc_msg_t * krpc_bdecode(krpc_t * this, buffer_stream_t * bs)
{
	if (!bs ) return 0;
	int tab = 0;
	if (!buffer_stream_match(bs, "d1:")) {
		return 0;
	}

	krpc_msg_t * ret = (krpc_msg_t*)malloc(sizeof(krpc_msg_t));
	switch(buffer_stream_getch(bs)) {
		case 'r':
			{
				debug("match this");
				ret->y = _r;
				if (!buffer_stream_match(bs, "d2:id20:")) {
					return 0;
				}
				buffer_stream_read(bs, ret->r.id, ID_LEN);

				if (buffer_stream_match(bs, "5:nodes")) {
					int bytes_count = buffer_stream_get_int(bs);
					debug("nodes_count = %d/%ld=%ld", bytes_count, sizeof(compacked_node_info_t), bytes_count/sizeof(compacked_node_info_t));
				}else if (buffer_stream_match(bs, "5:peers")) {
				}
			}
			break;
	}
	return 0;
}

int krpc_bencode(krpc_t * this, krpc_msg_t * msg, buffer_stream_t * bs)
{
	if (!bs) return 0;
	if (msg->y == _q) {
		this->msg_type[msg->t] = msg->y;
		bencode_query(msg, bs);
		buffer_stream_printf(bs, "1:t2:%c%c", ((char*)&msg->t)[0], ((char*)&msg->t)[1]);
		//buffer_stream_print_hex(bs, (byte_t*)&msg->t, 2);
		buffer_stream_printf(bs,"1:y1:%se", msg->y);
	}else if (msg->y == _r) {
	}else if (msg->y == _e) {
	}

	return 1;
}


int krpc_init(krpc_t * this, unsigned short port)
{
	this->cur_t = 0;	
	this->socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (this->socket_fd < 0) {
		perror("socket");
		return 0;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	int err = bind(this->socket_fd, (sockaddr*)&addr, sizeof(addr));
	if (err) {
		perror("bind");
		return 0;
	}

	memset(this->msg_type, 0, T_MAX);
	return 1;
}

krpc_msg_t * krpc_recv(krpc_t * this)
{
	buffer_stream_t ret_stream;
	buffer_stream_init(&ret_stream);

	sockaddr_in remote;
	socklen_t len = sizeof(remote);
	int rc = recvfrom(this->socket_fd, ret_stream.buf, ret_stream.len, 0, (sockaddr*)&remote, &len);
	ret_stream.w_pos = rc;
	if (rc <= 0) return NULL;
	debug("recv from %s:%d %d bytes", inet_ntoa(remote.sin_addr), ntohs(remote.sin_port), rc);
	debug("%s", ret_stream.buf);
	return krpc_bdecode(this, &ret_stream);
}

void krpc_send(krpc_t * this, krpc_msg_t * msg, compacked_node_info_t target)
{
	if (this->msg_type[this->cur_t]) {
		debug("无可用消息号");
		return;
	}
	msg->t = this->cur_t;
	this->msg_type[msg->t] = msg->y;
	this->cur_t = (this->cur_t+1)%T_MAX;

	buffer_stream_t data;
	buffer_stream_init(&data);
	krpc_bencode(this, msg, &data);

	sockaddr_in remote;
	remote.sin_family = AF_INET;
	remote.sin_addr.s_addr = target.peer.ip;
	remote.sin_port = target.peer.port;

	int sc = sendto(this->socket_fd, data.buf, data.w_pos, 0, (sockaddr*)&remote, sizeof(remote)); 
	if (sc <= 0) {
		debug("send faild %d",sc);
		perror("sendto");
	}else {
		debug("send to %s:%d %d bytes", inet_ntoa(remote.sin_addr), ntohs(remote.sin_port),  sc);
		debug("%s",data.buf);
	}
}
