#ifndef __port_priv_h
#define __port_priv_h 1

/* port_priv.h - private include file for network port definitions and types */

#include port

#define PORT_K_NAMED	1
#define PORT_K_UNNAMED	2
#define PORT_K_CLIENT	3
#define PORT_K_SERVER	4

#define MBX_DATA_MAX	16

typedef struct port_io_stat_blk_tag {
  short int status;
  short int msg_len;
  int unused;
} PORT_IO_STAT_BLK_T;

typedef struct port_msg_tag {
  PORT_IO_STAT_BLK_T iosb;
  char *msg;
  int msg_len;
  struct port_msg_tag *next;
} PORT_MSG_T;
typedef PORT_MSG_T *PORT_MSG_P;

typedef struct priv_port_tag {
  unsigned int port_event_flag;
  short port_type;
  short conn_type;
  short net_obj_mbx_chan;
  short net_dev_chan;
  int net_obj_mbx_unit;
  int net_dev_unit;
  int mbx_max_msg;
  int mbx_buf_quo;
  char *net_obj_name;
  char *client_data;
  unsigned int client_data_size;
  char *port_label;
  void *app_data;
  int max_msgs;
  int max_msg_size;
  int msg_count;
  PORT_MSG_P head;
  PORT_MSG_P tail;
  int incomplete_io_count;
  PORT_MSG_P incomplete_io_head;
  PORT_MSG_P incomplete_io_tail;
  short msg_in_mbox;
  short mbox_msg_len;
  char mbx_data[MBX_DATA_MAX+1];
} PRIV_PORT_T;

typedef PRIV_PORT_T *PRIV_PORT_P;

#endif
