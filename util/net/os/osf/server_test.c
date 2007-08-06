#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#ifdef __osf__
#include <time.h>
#include <sys/time.h>
#endif

#ifdef __vxworks__
#include <time.h>
#include <sys/times.h>
#endif

#include <errno.h>

#include "sys_types.h"
#include "ipnsv.h"
#include "thread.h"

void srv_thread (
  THREAD_HANDLE thread_h
) {

IPRPC_PORT port;
int i, ii, stat, result, len, send_len;
char msg[100+1];
SYS_TIME_TYPE timeout;

  stat = sys_cvt_seconds_to_timeout( 10.0, &timeout );
  if ( !( stat & 1 ) ) {
    printf( "error from sys_cvt_seconds_to_timeout\n" );
    msg_show_error_message( stat );
    exit(stat);
  }

  port = (IPRPC_PORT) thread_get_app_data( thread_h );

  while ( 1 ) {

    stat = ipnsv_wait_on_port( port, &timeout, &result );
    if ( !( stat & 1 ) ) {
      printf( "error from ipnsv_wait_on_port\n" );
      exit(stat);
    }

    if ( !result ) {

      printf( "Timeout\n" );
      continue;

    }
    else {

      strcpy( msg, "              " );
      stat = ipnsv_receive_msg( port, 20, &len, msg );
      if ( !( stat & 1 ) ) {
        printf( "error from ipnsv_receive_msg\n" );
        exit(stat);
      }

      if ( strcmp( msg, "EXIT" ) == 0 ) goto done;

      printf( "%s", msg );

      send_len = strlen( msg );

/*
      stat = ipnsv_send_msg( port, len+1, msg );
      if ( !( stat & 1 ) ) {
        printf( "error from ipnsv_send_msg\n" );
        exit(stat);
      }
*/

      stat = ipnsv_send_event_msg( port, len+1, msg );
      if ( !( stat & 1 ) ) {
        printf( "error from ipnsv_send_msg\n" );
        exit(stat);
      }

    }

  }

done:

  stat = ipnsv_disconnect( port );
  if ( !( stat & 1 ) ) {
    printf( "error from ipnsv_disconnect\n" );
    msg_show_error_message( stat );
  }

  stat = ipnsv_delete_port( &port );
  if ( !( stat & 1 ) ) {
    printf( "error from ipnsv_delete_port\n" );
    msg_show_error_message( stat );
  }

  stat = thread_destroy_handle( thread_h );
  if ( ! ( stat & 1 ) ) {
    msg_show_error_message( stat );
  }

}

#ifdef __osf__
main (
#endif
#ifdef __vxworks__
srv_main (
#endif
  int argc,
  int **argv
) {

typedef void (*VFUNC)();

int i, ii, stat, result, len, send_len;
THREAD_HANDLE thread_h;
IPRPC_PORT con_port, port;
char msg[100+1];

  stat = ipnsv_create_named_port( "15123", 1, 4096, "test (s)", &con_port );
  if ( !( stat & 1 ) ) {
    printf( "error from ipnsv_create_named_port\n" );
    msg_show_error_message( stat );
    exit(stat);
  }

  while ( 1 ) {

    stat = ipnsv_create_port( 1, 4096, "test", &port );
    if ( !( stat & 1 ) ) {
      printf( "error from ipnsv_create_port\n" );
      msg_show_error_message( stat );
      exit(stat);
    }

    stat = ipnsv_accept_connection( con_port, port, " " );
    if ( !( stat & 1 ) ) {
      printf( "error from ipnsv_accept_connection\n" );
      msg_show_error_message( stat );
      exit(stat);
    }

    stat = thread_create_handle( &thread_h, (void *) port );
    if ( ! ( stat & 1 ) ) {
      goto err_label;
    }

    stat = thread_create_proc( thread_h, (VFUNC) srv_thread );
    if ( ! ( stat & 1 ) ) {
      goto err_label;
    }

    goto next;
err_label:
    printf( "error\n" );
next:
    ;

  }

}



