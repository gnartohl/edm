#ifndef __ipncl_h
#define __ipncl_h 1

#include "ipport.h"
#include "ipncl.msh"

#ifdef __cplusplus
extern "C" {
#endif

int ipncl_network_link_fail (
  int stat
);

int ipncl_create_port (
  int max_msgs,
  int max_msg_size,
  char *label,
  IPRPC_PORT *port
);

int ipncl_disconnect (
  IPRPC_PORT port
);

int ipncl_delete_port (
  IPRPC_PORT *port
);

int ipncl_connect (
  char *host,		/* host name or ip address (asciz) */
  char *service,	/* service name or port num (asciz) */
  char *optional_data,	/* up to 14 bytes */
  IPRPC_PORT port	/* new port through which subsequent
			   messages are exchanged */
);

int ipncl_wait_on_port (
  IPRPC_PORT port,
  SYS_TIME_PTR sys_timeout,
  int *result
);

int ipncl_receive_msg (
  IPRPC_PORT port,
  int buf_len,
  int *msg_len,
  char *msg
);

int ipncl_send_msg (
  IPRPC_PORT port,
  int msg_len,
  char *msg
);

int ipncl_wait_on_event (
  IPRPC_PORT port,
  SYS_TIME_PTR sys_timeout,
  int *result
);

int ipncl_receive_event_msg (
  IPRPC_PORT port,
  int buf_len,
  int *msg_len,
  char *msg
);

#ifdef __cplusplus
}
#endif

#endif
