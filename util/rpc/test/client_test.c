#include stdio
#include descrip

#include client_rpc

main (
  int argc,
  int **argv
) {

int stat, i, a, b, result;
RPC_HANDLE handle;
float seconds = 1.0;

  stat = client_rpc_create_handle( &handle, "port name" );
  if ( stat != RPC_SUCCESS ) {
    printf( "client_rpc_create_handle\n" );
    exit(stat);
  }

  a = 100;
  b = 200;
  for ( i=0; i<1000; i++ ) {

    stat = sum ( handle, a, b, &result );
    if ( stat != RPC_SUCCESS ) {
      printf( "error from sum\n" );
      goto abort;
    }
    printf ( "%-d + %-d = %-d\n", a, b, result );

    stat = diff ( handle, a, b, &result );
    if ( stat != RPC_SUCCESS ) {
      printf( "error from diff\n" );
      goto abort;
    }
    printf ( "%-d - %-d = %-d\n", a, b, result );

    stat = mult ( handle, a, b, &result );
    if ( stat != RPC_SUCCESS ) {
      printf( "error from mult\n" );
      goto abort;
    }
    printf ( "%-d * %-d = %-d\n", a, b, result );

    a++;
    b += 2;

    sys_wait_seconds( &seconds );

  }

abort:

  stat = client_rpc_destroy_handle( &handle );
  if ( stat != RPC_SUCCESS ) {
    printf( "client_rpc_destroy_handle\n" );
    exit(stat);
  }

}
