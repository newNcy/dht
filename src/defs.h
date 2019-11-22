#pragma once
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
