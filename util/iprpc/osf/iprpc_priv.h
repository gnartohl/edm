#ifndef __iprpc_priv_h
#define __iprpc_priv_h	1

#include "iprpc.h"

#define IPRPC_MAX_ARGS		20
#define IPRPC_MAX_MSG_SIZE	8192

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

typedef struct priv_iprpc_handle_tag {
  int connect_state;
  int connect_type;
  SYS_TIME_TYPE timeout;
  IPRPC_PORT data_port;
  IPRPC_BUF buf;
} PRIV_IPRPC_HANDLE_TYPE;
typedef PRIV_IPRPC_HANDLE_TYPE *PRIV_IPRPC_HANDLE_PTR;

#endif
