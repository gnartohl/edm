#ifndef __msg_h
#define __msg_h 1

#include "msg.msh"
#include "util.fac"

#ifdef __cplusplus
extern "C" {
#endif

/***
* prototypes
*/
int msg_get_severity(
  int error_code
);

int msg_get_facility(
  int error_code
);

int msg_get_code(
  int error_code
);

#ifdef __cplusplus
}
#endif

#endif
