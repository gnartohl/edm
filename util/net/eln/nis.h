#ifndef __nis_h
#define __nis_h 1

#define NIS_MAX_SERVICE_NAME	31

int nis_get_service (
  int num_connect_fails,
  char *generic_service_name,
  char *service_name,
  char *node,
  int *mode
);

globalvalue NIS_SUCCESS;
globalvalue NIS_UNKSVC;

#endif
