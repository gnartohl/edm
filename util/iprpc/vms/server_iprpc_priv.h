#ifndef __server_iprpc_priv_h
#define __server_iprpc_priv_h	1

#include server_iprpc
#include iprpc_priv

typedef struct arg_desc_tag {
  unsigned char type_code;
  unsigned char access_code;
  unsigned short buf_size;
  int ofs;
  void *client_addr;
  void *dummy; /* for 64 bit counterparts */
} ARG_DESC_TYPE;

typedef struct msg_buf_tag {
  unsigned int send_seq_num;
  unsigned int rcv_seq_num;
  short func_id;
  short num_args;
  char data[IPRPC_MAX_MSG_SIZE];
} MSG_BUF_TYPE;

typedef void (*FUNC_TYPE)();
typedef FUNC_TYPE *FUNC_PTR;

typedef struct buf_tag {
  IPRPC_PORT iprpc_port;
  void *server_data;
  int msg_size;
  int max_funcs;
  FUNC_PTR func;
  MSG_BUF_TYPE msg_buf;
} PRIV_IPRPC_BUF_TYPE;
typedef PRIV_IPRPC_BUF_TYPE *PRIV_IPRPC_BUF_PTR;

#endif
