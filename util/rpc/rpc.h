#ifndef __rpc_h
#define __rpc_h	1

/***
/* global values for status codes
*/
globalvalue RPC_SUCCESS;
globalvalue RPC_NOMEM;
globalvalue RPC_BADPARAM;
globalvalue RPC_ARGBUFOVFLO;
globalvalue RPC_INVTYPE;
globalvalue RPC_DATAOVFLO;
globalvalue RPC_BADSEQ;
globalvalue RPC_BADFUNCINDEX;
globalvalue RPC_NOSUCHFUNC;
globalvalue RPC_NUMARGSINV;

#define RPC_K_UNCONNECTED	1
#define RPC_K_CONNECTION_FAILED	2
#define RPC_K_RECONNECTING	3
#define RPC_K_CONNECTED		4

#define RPC_K_MASTER		0x11
#define RPC_K_LOCAL		0x12

typedef void *RPC_BUF;
typedef void *RPC_HANDLE;

#endif
