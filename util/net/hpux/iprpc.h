#ifndef __iprpc_h
#define __iprpc_h	1

#include "iprpc.msh"

#define IPRPC_K_UNCONNECTED	1
#define IPRPC_K_CONNECTION_FAILED	2
#define IPRPC_K_RECONNECTING	3
#define IPRPC_K_CONNECTED		4

#define IPRPC_K_MASTER		0x11
#define IPRPC_K_LOCAL		0x12

typedef void *IPRPC_BUF;
typedef void *IPRPC_HANDLE;

#endif
