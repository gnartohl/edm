#ifndef __nsv_h
#define __nsv_h 1

#include "sys_types.h"
#include "ipport.h"
#include "ipnsv.msh"

#ifdef __cplusplus
extern "C" {
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
  IPRPC_PORT connect_port, /* port through which connect requests enter */
  IPRPC_PORT data_port,	 /* new port through which subsequent
			    messages are exchanged */
  char *optional_data	 /* up to 16 chars + zero */
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
