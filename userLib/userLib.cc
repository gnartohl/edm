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

int i;
double *v = (double *) values;

#if 0
  printf( "testRule, n=%-d\n", n );
  for ( i=0; i<n; i++ ) {
    printf( "value[%-d] = %-g\n", i, v[i] );
  }
#endif

  if ( v[0] == 0.0 ) {
    return 31;
  }
  else if ( v[1] < 10.0 ) {
    return 8;
  }
  else if ( v[1] < 20.0 ) {
    return 9;
  }
  else if ( v[1] < 30.0 ) {
    return 10;
  }
  else if ( v[1] < 40.0 ) {
    return 11;
  }
  else if ( v[1] < 50.0 ) {
    return 12;
  }
  else if ( v[1] < 60.0 ) {
    return 13;
  }

  return -1;

}

#ifdef __cplusplus
}
#endif




