#ifndef __iprpc_h
#define __iprpc_h	1

/***
/* global values for status codes
*/
globalvalue IPRPC_SUCCESS;
globalvalue IPRPC_NOMEM;
globalvalue IPRPC_BADPARAM;
globalvalue IPRPC_ARGBUFOVFLO;
globalvalue IPRPC_INVTYPE;
globalvalue IPRPC_DATAOVFLO;
globalvalue IPRPC_BADSEQ;
globalvalue IPRPC_BADFUNCINDEX;
globalvalue IPRPC_NOSUCHFUNC;
globalvalue IPRPC_NUMARGSINV;

#define IPRPC_K_UNCONNECTED		1
#define IPRPC_K_CONNECTION_FAILED	2
#define IPRPC_K_RECONNECTING		3
#define IPRPC_K_CONNECTED		4

#define IPRPC_K_MASTER		0x11
#define IPRPC_K_LOCAL		0x12

typedef void *IPRPC_BUF;
typedef void *IPRPC_HANDLE;

#endif
