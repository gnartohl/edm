#include stdio
#include descrip

#include client_iprpc

main (
  int argc,
  int **argv
) {

int stat, i, a, b, result;
IPRPC_HANDLE handle;
float seconds = 0.1;

  stat = client_iprpc_create_handle( &handle, "port name" );
  if ( stat != IPRPC_SUCCESS ) {
    printf( "client_iprpc_create_handle\n" );
    exit(stat);
  }

  a = 100;
  b = 200;
  for ( i=0; i<1000; i++ ) {

    stat = sum ( handle, a, b, &result );
    if ( stat != IPRPC_SUCCESS ) {
      printf( "error from sum\n" );
      goto abort;
    }
    printf ( "%-d + %-d = %-d\n", a, b, result );

    stat = diff ( handle, a, b, &result );
    if ( stat != IPRPC_SUCCESS ) {
      printf( "error from diff\n" );
      goto abort;
    }
    printf ( "%-d - %-d = %-d\n", a, b, result );

    stat = mult ( handle, a, b, &result );
    if ( stat != IPRPC_SUCCESS ) {
      printf( "error from mult\n" );
      goto abort;
    }
    printf ( "%-d * %-d = %-d\n", a, b, result );

    a++;
    b += 2;

    sys_wait_seconds( &seconds );

  }

abort:

  stat = client_iprpc_destroy_handle( &handle );
  if ( stat != IPRPC_SUCCESS ) {
    printf( "client_iprpc_destroy_handle\n" );
    exit(stat);
  }

}
