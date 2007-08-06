#ifndef __ipport_priv_h
#define __ipport_priv_h 1

/* ipport_priv.h - private include file for network port definitions and types */

#include "ipport.h"

#define IPPORT_K_NAMED		1
#define IPPORT_K_UNNAMED	2
#define IPPORT_K_CLIENT		3
#define IPPORT_K_SERVER		4

typedef struct priv_ipport_tag {
  struct sockaddr_in sin;
  int sockfd;
  int event_sockfd;
  short port_type;
  short conn_type;
  int mbx_max_msg;
  char *net_obj_name;
  char *client_data;
  unsigned int client_data_size;
  char *port_label;
  void *app_data;
} PRIV_IPPORT_T;

typedef PRIV_IPPORT_T *PRIV_IPPORT_P;

#endif
