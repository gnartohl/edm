#include stdio
#include descrip

#include sys_types
#include client_rpc
#include rpc_priv
#include "rpc_app.h"

#define MAX_CONNECT_FAILURES	10
#define MAX_LINK_FAILURES	10


int rpc_connect (
  RPC_HANDLE handle,
  char *generic_service,
  char *client_data
) {

int mode, stat, ret_stat, num;
PRIV_RPC_HANDLE_PTR priv_handle;
char node[31+1], service[31+1];

  priv_handle = (PRIV_RPC_HANDLE_PTR) handle;

  num = 0;
  stat = nis_get_service( num, generic_service, service, node, &mode );

  stat = ncl_connect( node, service, client_data, priv_handle->data_port );
  if ( stat != NCL_SUCCESS ) {
     ret_stat = stat;
     goto err_return;
  }

  priv_handle->connect_state = RPC_K_CONNECTED;
  priv_handle->connect_type = mode;

norm_return:
  return RPC_SUCCESS;

err_return:
  return ret_stat;

}

int rpc_disconnect (
  RPC_HANDLE handle
) {

int stat, ret_stat;
PRIV_RPC_HANDLE_PTR priv_handle;

  priv_handle = (PRIV_RPC_HANDLE_PTR) handle;

  stat = ncl_disconnect( priv_handle->data_port );
  if ( stat != NCL_SUCCESS ) {
     ret_stat = stat;
     goto err_return;
  }

  priv_handle->connect_state = RPC_K_UNCONNECTED;
  priv_handle->connect_type = 0;

norm_return:
  return RPC_SUCCESS;

err_return:
  return ret_stat;

}

int sum (
  RPC_HANDLE handle,
  int a,
  int b,
  int *result
) {

int link_fail, num_link_fails, connect_fail, num_connect_fails, stat, ret_stat;
int link_fail_stat;
PRIV_RPC_HANDLE_PTR priv_handle;

  priv_handle = (PRIV_RPC_HANDLE_PTR) handle;

  num_link_fails = 0;
  do {

    if ( priv_handle->connect_state == RPC_K_UNCONNECTED ) {

      num_connect_fails = 0;
      do {

          connect_fail = 0;
          stat = rpc_connect ( handle, "TEST", "client data" );
          if ( stat != RPC_SUCCESS ) {
            connect_fail = 1;
            num_connect_fails++;
          }

      } while ( connect_fail && ( num_connect_fails < MAX_CONNECT_FAILURES ) );

      if ( connect_fail ) {
        ret_stat = stat;
        goto err_return;
      }

    }

    if ( num_link_fails ) {
      printf( "Network reconnected\n" );
    }

    client_rpc_buf_init( priv_handle, SUM, 3 );

    stat = client_rpc_buf_add_arg( priv_handle, ARG_TYPE_INT,
     ARG_ACCESS_READ, sizeof(int), &a );
    if ( stat != RPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

    stat = client_rpc_buf_add_arg( priv_handle, ARG_TYPE_INT,
     ARG_ACCESS_READ, sizeof(int), &b );
    if ( stat != RPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

    stat = client_rpc_buf_add_arg( priv_handle, ARG_TYPE_INT,
     ARG_ACCESS_WRITE, sizeof(int), result );
    if ( stat != RPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

    link_fail = 0;

    stat = client_rpc_remote_call( priv_handle );

    if ( ncl_network_link_fail( stat ) ) {
      link_fail_stat = stat;
      link_fail = 1;
      num_link_fails++;
      stat = rpc_disconnect( priv_handle );
      printf( "Network connection failed\n" );
    }
    else if ( stat != RPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

  } while ( link_fail && ( num_link_fails < MAX_LINK_FAILURES ) );

  if ( link_fail ) {
    ret_stat = link_fail_stat;
    goto err_return;
  }

norm_return:
  return RPC_SUCCESS;

err_return:
  return ret_stat;

}

int diff (
  RPC_HANDLE handle,
  int a,
  int b,
  int *result
) {

int link_fail, num_link_fails, connect_fail, num_connect_fails, stat, ret_stat;
int link_fail_stat;
PRIV_RPC_HANDLE_PTR priv_handle;

  priv_handle = (PRIV_RPC_HANDLE_PTR) handle;

  num_link_fails = 0;
  do {

    if ( priv_handle->connect_state == RPC_K_UNCONNECTED ) {

      num_connect_fails = 0;
      do {

          connect_fail = 0;
          stat = rpc_connect ( handle, "TEST", "client data" );
          if ( stat != RPC_SUCCESS ) {
            connect_fail = 1;
            num_connect_fails++;
          }

      } while ( connect_fail && ( num_connect_fails < MAX_CONNECT_FAILURES ) );

      if ( connect_fail ) {
        ret_stat = stat;
        goto err_return;
      }

    }

    if ( num_link_fails ) {
      printf( "Network reconnected\n" );
    }

    client_rpc_buf_init( priv_handle, DIFF, 3 );

    stat = client_rpc_buf_add_arg( priv_handle, ARG_TYPE_INT,
     ARG_ACCESS_READ, sizeof(int), &a );
    if ( stat != RPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

    stat = client_rpc_buf_add_arg( priv_handle, ARG_TYPE_INT,
     ARG_ACCESS_READ, sizeof(int), &b );
    if ( stat != RPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

    stat = client_rpc_buf_add_arg( priv_handle, ARG_TYPE_INT,
     ARG_ACCESS_WRITE, sizeof(int), result );
    if ( stat != RPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

    link_fail = 0;

    stat = client_rpc_remote_call( priv_handle );

    if ( ncl_network_link_fail( stat ) ) {
      link_fail_stat = stat;
      link_fail = 1;
      num_link_fails++;
      stat = rpc_disconnect( priv_handle );
      printf( "Network connection failed\n" );
    }
    else if ( stat != RPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

  } while ( link_fail && ( num_link_fails < MAX_LINK_FAILURES ) );

  if ( link_fail ) {
    ret_stat = link_fail_stat;
    goto err_return;
  }

norm_return:
  return RPC_SUCCESS;

err_return:
  return ret_stat;

}

int mult (
  RPC_HANDLE handle,
  int a,
  int b,
  int *result
) {

int link_fail, num_link_fails, connect_fail, num_connect_fails, stat, ret_stat;
int link_fail_stat;
PRIV_RPC_HANDLE_PTR priv_handle;

  priv_handle = (PRIV_RPC_HANDLE_PTR) handle;

  num_link_fails = 0;
  do {

    if ( priv_handle->connect_state == RPC_K_UNCONNECTED ) {

      num_connect_fails = 0;
      do {

          connect_fail = 0;
          stat = rpc_connect ( handle, "TEST", "client data" );
          if ( stat != RPC_SUCCESS ) {
            connect_fail = 1;
            num_connect_fails++;
          }

      } while ( connect_fail && ( num_connect_fails < MAX_CONNECT_FAILURES ) );

      if ( connect_fail ) {
        ret_stat = stat;
        goto err_return;
      }

    }

    if ( num_link_fails ) {
      printf( "Network reconnected\n" );
    }

    client_rpc_buf_init( priv_handle, MULT, 3 );

    stat = client_rpc_buf_add_arg( priv_handle, ARG_TYPE_INT,
     ARG_ACCESS_READ, sizeof(int), &a );
    if ( stat != RPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

    stat = client_rpc_buf_add_arg( priv_handle, ARG_TYPE_INT,
     ARG_ACCESS_READ, sizeof(int), &b );
    if ( stat != RPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

    stat = client_rpc_buf_add_arg( priv_handle, ARG_TYPE_INT,
     ARG_ACCESS_WRITE, sizeof(int), result );
    if ( stat != RPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

    link_fail = 0;

    stat = client_rpc_remote_call( priv_handle );

    if ( ncl_network_link_fail( stat ) ) {
      link_fail_stat = stat;
      link_fail = 1;
      num_link_fails++;
      stat = rpc_disconnect( priv_handle );
      printf( "Network connection failed\n" );
    }
    else if ( stat != RPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

  } while ( link_fail && ( num_link_fails < MAX_LINK_FAILURES ) );

  if ( link_fail ) {
    ret_stat = link_fail_stat;
    goto err_return;
  }

norm_return:
  return RPC_SUCCESS;

err_return:
  return ret_stat;

}
