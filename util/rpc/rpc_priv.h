#ifndef __rpc_priv_h
#define __rpc_priv_h	1

#include rpc

#define RPC_MAX_ARGS		20
#define RPC_MAX_MSG_SIZE	8192

#define ARG_TYPE_INT		0x11
#define ARG_TYPE_SHORT		0x12
#define ARG_TYPE_QUAD		0x13
#define ARG_TYPE_CHAR		0x14
#define ARG_TYPE_FLOAT		0x15
#define ARG_TYPE_DOUBLE		0x16
#define ARG_TYPE_STRUCT		0x17

#define ARG_ACCESS_READ		0x21
#define ARG_ACCESS_WRITE	0x22
#define ARG_ACCESS_RDWR		0x23

typedef struct priv_rpc_handle_tag {
  int connect_state;
  int connect_type;
  SYS_TIME_TYPE timeout;
  RPC_PORT data_port;
  RPC_BUF buf;
} PRIV_RPC_HANDLE_TYPE;
typedef PRIV_RPC_HANDLE_TYPE *PRIV_RPC_HANDLE_PTR;

#endif
