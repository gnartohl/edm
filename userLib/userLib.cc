#include <stdio.h>
#include <string.h>

#include "act_grf.h"
#include "act_win.h"
#include "app_pkg.h"

#ifdef __cplusplus
extern "C" {
#endif

int testRule (
  void *classPtr,
  int n,
  void *values
) {

typedef struct {
  double v1;
  double v2;
} *valPtr;

int i;
//double *v = (double *) values;
valPtr v = (valPtr) values;

#if 0
  printf( "testRule, n=%-d\n", n );
#endif

#if 0
  if ( v[0] == 0.0 ) {
    return 31;
  }
  else if ( v[1] < -2 ) {
    return 8;
  }
  else if ( v[1] < -1 ) {
    return 9;
  }
  else if ( v[1] < 0 ) {
    return 10;
  }
  else if ( v[1] < 1 ) {
    return 11;
  }
  else if ( v[1] < 2 ) {
    return 12;
  }
  else if ( v[1] < 3 ) {
    return 13;
  }
#endif

  if ( v->v1 == 0.0 ) {
    return 31;
  }
  else if ( v->v2 < -2 ) {
    return 8;
  }
  else if ( v->v2 < -1 ) {
    return 9;
  }
  else if ( v->v2 < 0 ) {
    return 10;
  }
  else if ( v->v2 < 1 ) {
    return 11;
  }
  else if ( v->v2 < 2 ) {
    return 12;
  }
  else if ( v->v2 < 3 ) {
    return 13;
  }

  return -1;

}

#ifdef __cplusplus
}
#endif




