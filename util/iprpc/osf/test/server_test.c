#include <stdio.h>
#include <stdlib.h>

#include "ipport.h"
#include "server_rpc.h"
#include "thread.h"
#include "rpc_app.h"

#define MAX_ERRORS	100

static void sum (
  void *app_data,
  int *a,
  int *b,
  int *sum
) {

  *sum = *a + *b;

}

static void diff (
  void *app_data,
  int *a,
  int *b,
  int *diff
) {

  *diff = *a - *b;

}

static void mult (
  void *app_data,
  int *a,
  int *b,
  int *mult
) {

  *mult = *a * *b;

}

static int rpc_thread (
  THREAD_HANDLE thread_h
) {

RPC_IPPORT data_port;
RPC_BUF buf;
float sec = 2.0;
int ret_stat, stat;

  ret_stat = 1;

  data_port = (RPC_IPPORT) thread_get_app_data( thread_h );
  if ( !data_port ) {
    ret_stat = RPC_BADPARAM;
    goto err_return;
  }

  stat = server_rpc_create_buffer( MAX_FUNCS, data_port, NULL, &buf );
  if ( ! ( stat & 1 ) ) {
    ret_stat = stat;
    goto err_return;
  }

  stat = server_rpc_register_function( buf, sum, SUM );
  if ( ! ( stat & 1 ) ) {
    ret_stat = stat;
    goto err_return;
  }

  stat = server_rpc_register_function( buf, diff, DIFF );
  if ( ! ( stat & 1 ) ) {
    ret_stat = stat;
    goto err_return;
  }

  stat = server_rpc_register_function( buf, mult, MULT );
  if ( ! ( stat & 1 ) ) {
    ret_stat = stat;
    goto err_return;
  }

  stat = server_rpc_dispatch_requests( buf );
  if ( ! ( stat & 1 ) ) {
    ret_stat = stat;
  }

  sys_wait_seconds( &sec );

  stat = server_rpc_destroy_buffer( buf );
  if ( ! ( stat & 1 ) ) {
    ret_stat = stat;
  }

  stat = ipnsv_disconnect( data_port );
  if ( ! ( stat & 1 ) ) {
    ret_stat = stat;
  }

  stat = ipnsv_delete_port( &data_port );
  if ( ! ( stat & 1 ) ) {
    ret_stat = stat;
  }

  stat = thread_destroy_handle( thread_h );
  if ( ! ( stat & 1 ) ) {
    ret_stat = stat;
  }

  if ( ! ( ret_stat & 1 ) ) goto err_return;

norm_ret:
  THREAD_SUCCESS( 1 )

err_return:
  THREAD_ERROR( ret_stat )

}

main (
  int argc,
  int **argv
) {

typedef void (*VFUNC)();

RPC_IPPORT connect_port, data_port;
THREAD_HANDLE thread_h;
int stat, num_errors;
char optional_data[16+1];

  stat = ipnsv_create_named_port( "5200", 1, 128, "my connect port",
   &connect_port );
  if ( ! ( stat & 1 ) ) {
    exit( stat );
  }

  num_errors = 0;

  do {

    stat = ipnsv_create_port( 10, 512, "my data port", &data_port );
    if ( ! ( stat & 1 ) ) {
      goto err_label;
    }

    stat = ipnsv_accept_connection( connect_port, data_port, optional_data );
    if ( ! ( stat & 1 ) ) {
      goto err_label;
    }

    if ( optional_data[0] )
      printf( "connect data = [%s]\n", optional_data );
    else
      printf( "no connect data\n" );

    stat = thread_create_handle( &thread_h, (void *) data_port );
    if ( ! ( stat & 1 ) ) {
      goto err_label;
    }

    thread_create_proc( thread_h, (VFUNC) rpc_thread );
    if ( ! ( stat & 1 ) ) {
      goto err_label;
    }

/*
    rpc_thread( thread_h );
    stat = 1;
*/

err_label:
    if ( ! ( stat & 1 ) ) {
      /* log error message */
      printf( "stat = %d\n", stat );
      num_errors++;
    }

  } while ( num_errors < MAX_ERRORS );

  stat = ipnsv_delete_port( &connect_port );
  if ( ! ( stat & 1 ) ) {
    printf( "stat = %d\n", stat );
  }

}
