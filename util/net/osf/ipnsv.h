#ifndef __nsv_h
#define __nsv_h 1

#include "ipport.h"
#include "ipnsv.msh"
#include "sys_types.h"

#ifdef __cplusplus
#extern "C" {
#endif

int ipnsv_create_named_port (
  char *service,
  int max_msgs,
  int max_msg_size,
  char *label,
  IPRPC_PORT *port
);

int ipnsv_create_port (
  int max_msgs,
  int max_msg_size,
  char *label,
  IPRPC_PORT *port
);

int ipnsv_disconnect (
  IPRPC_PORT port
);

int ipnsv_delete_port (
  IPRPC_PORT *port
);

int ipnsv_accept_connection (
  IPRPC_PORT connect_port,
  IPRPC_PORT data_port,
  char *optional_data
);

int ipnsv_wait_on_port (
  IPRPC_PORT port,
  SYS_TIME_TYPE *sys_timeout,
  int *result
);

int ipnsv_receive_msg (
  IPRPC_PORT port,
  int buf_len,
  int *msg_len,
  char *msg
);

int ipnsv_send_msg (
  IPRPC_PORT port,
  int msg_len,
  char *msg
);

int ipnsv_send_event_msg (
  IPRPC_PORT port,
  int msg_len,
  char *msg
);

#ifdef __cplusplus
}
#endif

#endif
