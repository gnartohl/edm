#ifndef __client_rpc_priv_h
#define __client_rpc_priv_h	1

#include client_rpc
#include rpc_priv

typedef struct arg_desc_tag {
  unsigned char type_code;
  unsigned char access_code;
  unsigned short buf_size;
  int ofs;
  void *client_addr;
} ARG_DESC_TYPE;

typedef struct msg_buf_tag {
  unsigned int send_seq_num;
  unsigned int rcv_seq_num;
  short func_id;
  short num_args;
  char data[RPC_MAX_MSG_SIZE];
} MSG_BUF_TYPE;

typedef struct rpc_buf_tag {
  int cur_index;
  int cur_ofs;
  RPC_PORT rpc_port;
  int msg_size;
  MSG_BUF_TYPE msg_buf;
} PRIV_RPC_BUF_TYPE;
typedef PRIV_RPC_BUF_TYPE *PRIV_RPC_BUF_PTR;

#endif
