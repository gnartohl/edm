#include stdio
#include descrip

#include sys_types
#include ipncl

main (
  int argc,
  int **argv
) {

int i, stat, result, len;
IPRPC_PORT port;
char msg[100+1];
SYS_TIME_TYPE timeout;

  stat = sys_cvt_seconds_to_timeout( 10.0, &timeout );
  if ( !( stat & 1 ) ) {
    printf( "error from sys_cvt_seconds_to_timeout\n" );
    exit(stat);
  }

  stat = ipncl_create_port( 1, 4096, "test", &port );
  if ( !( stat & 1 ) ) {
    printf( "error from ipncl_create_port\n" );
    exit(stat);
  }

  stat = ipncl_connect( "134.167.20.89", "15123", "", port );
  if ( !( stat & 1 ) ) {
    printf( "error from ipncl_connect\n" );
    exit(stat);
  }

  for ( i=0; i<100; i++ ) {

    sprintf( msg, "this is msg %-d\n", i );
    len = strlen(msg);

    stat = ipncl_send_msg( port, len+1, msg );
    if ( !( stat & 1 ) ) {
      printf( "error from ipncl_send_msg\n" );
      exit(stat);
    }

    stat = ipncl_wait_on_port( port, &timeout, &result );
    if ( !( stat & 1 ) ) {
      printf( "error from ipncl_wait_on_port\n" );
      exit(stat);
    }

/*
    stat = ipncl_wait_on_event( port, NULL, &result );
    if ( !( stat & 1 ) ) {
      printf( "error from ipncl_wait_on_port\n" );
      exit(stat);
    }
    result = 1;
*/

    if ( !result ) {

      printf( "Timeout\n" );

    }
    else {

      strcpy( msg, "              " );

      stat = ipncl_receive_msg( port, 100, &len, msg );
      if ( !( stat & 1 ) ) {
        printf( "error from ipncl_receive_msg\n" );
        exit(stat);
      }

/*
      stat = ipncl_receive_event_msg( port, 100, &len, msg );
      if ( !( stat & 1 ) ) {
        printf( "error from ipncl_receive_msg\n" );
        exit(stat);
      }
*/

    }

  }

  strcpy( msg, "EXIT" );
  len = strlen(msg);

  stat = ipncl_send_msg( port, len+1, msg );
  if ( !( stat & 1 ) ) {
    printf( "error from ipncl_send_msg\n" );
    exit(stat);
  }

  stat = ipncl_disconnect( port );
  if ( !( stat & 1 ) ) {
    printf( "error from ipncl_disconnect\n" );
    exit(stat);
  }

  stat = ipncl_delete_port( &port );
  if ( !( stat & 1 ) ) {
    printf( "error from ipncl_delete_port\n" );
    exit(stat);
  }

}
