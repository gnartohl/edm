#ifndef __ipnis_h
#define __ipnis_h 1

#include "ipnis.msh"

#define IPNIS_MAX_SERVICE_NAME	31
#define RPC_K_MASTER 1
#define RPC_K_LOCAL 2

#ifdef __cplusplus
#extern "C" {
#endif

int ipnis_get_service (
  int num_connect_fails,
  char *generic_service_name,
  char *service_name,
  char *node,
  int *mode
);

#ifdef __cplusplus
}
#endif

#endif
