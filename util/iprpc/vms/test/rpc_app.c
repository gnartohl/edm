#include stdio
#include descrip

#include sys_types
#include client_iprpc
#include iprpc_priv
#include ipnis
#include "rpc_app.h"

#define MAX_CONNECT_FAILURES	10
#define MAX_LINK_FAILURES	10


int iprpc_connect (
  IPRPC_HANDLE handle,
  char *generic_service,
  char *client_data
) {

int mode, stat, ret_stat, num;
PRIV_IPRPC_HANDLE_PTR priv_handle;
char node[31+1], service[31+1];

  priv_handle = (PRIV_IPRPC_HANDLE_PTR) handle;

  num = 0;
  stat = ipnis_get_service( num, generic_service, service, node, &mode );

  stat = ipncl_connect( node, service, client_data, priv_handle->data_port );
  if ( stat != IPNCL_SUCCESS ) {
     ret_stat = stat;
     goto err_return;
  }

  priv_handle->connect_state = IPRPC_K_CONNECTED;
  priv_handle->connect_type = mode;

norm_return:
  return IPRPC_SUCCESS;

err_return:
  return ret_stat;

}

int iprpc_disconnect (
  IPRPC_HANDLE handle
) {

int stat, ret_stat;
PRIV_IPRPC_HANDLE_PTR priv_handle;

  priv_handle = (PRIV_IPRPC_HANDLE_PTR) handle;

  stat = ipncl_disconnect( priv_handle->data_port );
  if ( stat != IPNCL_SUCCESS ) {
     ret_stat = stat;
     goto err_return;
  }

  priv_handle->connect_state = IPRPC_K_UNCONNECTED;
  priv_handle->connect_type = 0;

norm_return:
  return IPRPC_SUCCESS;

err_return:
  return ret_stat;

}

int sum (
  IPRPC_HANDLE handle,
  int a,
  int b,
  int *result
) {

int link_fail, num_link_fails, connect_fail, num_connect_fails, stat, ret_stat;
int link_fail_stat;
PRIV_IPRPC_HANDLE_PTR priv_handle;

  priv_handle = (PRIV_IPRPC_HANDLE_PTR) handle;

  num_link_fails = 0;
  do {

    if ( priv_handle->connect_state == IPRPC_K_UNCONNECTED ) {

      num_connect_fails = 0;
      do {

          connect_fail = 0;
          stat = iprpc_connect ( handle, "TEST", "client data" );
          if ( stat != IPRPC_SUCCESS ) {
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

    client_iprpc_buf_init( priv_handle, SUM, 3 );

    stat = client_iprpc_buf_add_arg( priv_handle, ARG_TYPE_INT,
     ARG_ACCESS_READ, sizeof(int), &a );
    if ( stat != IPRPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

    stat = client_iprpc_buf_add_arg( priv_handle, ARG_TYPE_INT,
     ARG_ACCESS_READ, sizeof(int), &b );
    if ( stat != IPRPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

    stat = client_iprpc_buf_add_arg( priv_handle, ARG_TYPE_INT,
     ARG_ACCESS_WRITE, sizeof(int), result );
    if ( stat != IPRPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

    link_fail = 0;

    stat = client_iprpc_remote_call( priv_handle );

    if ( ipncl_network_link_fail( stat ) ) {
      link_fail_stat = stat;
      link_fail = 1;
      num_link_fails++;
      stat = iprpc_disconnect( priv_handle );
      printf( "Network connection failed\n" );
    }
    else if ( stat != IPRPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

  } while ( link_fail && ( num_link_fails < MAX_LINK_FAILURES ) );

  if ( link_fail ) {
    ret_stat = link_fail_stat;
    goto err_return;
  }

norm_return:
  return IPRPC_SUCCESS;

err_return:
  return ret_stat;

}

int diff (
  IPRPC_HANDLE handle,
  int a,
  int b,
  int *result
) {

int link_fail, num_link_fails, connect_fail, num_connect_fails, stat, ret_stat;
int link_fail_stat;
PRIV_IPRPC_HANDLE_PTR priv_handle;

  priv_handle = (PRIV_IPRPC_HANDLE_PTR) handle;

  num_link_fails = 0;
  do {

    if ( priv_handle->connect_state == IPRPC_K_UNCONNECTED ) {

      num_connect_fails = 0;
      do {

          connect_fail = 0;
          stat = iprpc_connect ( handle, "TEST", "client data" );
          if ( stat != IPRPC_SUCCESS ) {
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

    client_iprpc_buf_init( priv_handle, DIFF, 3 );

    stat = client_iprpc_buf_add_arg( priv_handle, ARG_TYPE_INT,
     ARG_ACCESS_READ, sizeof(int), &a );
    if ( stat != IPRPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

    stat = client_iprpc_buf_add_arg( priv_handle, ARG_TYPE_INT,
     ARG_ACCESS_READ, sizeof(int), &b );
    if ( stat != IPRPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

    stat = client_iprpc_buf_add_arg( priv_handle, ARG_TYPE_INT,
     ARG_ACCESS_WRITE, sizeof(int), result );
    if ( stat != IPRPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

    link_fail = 0;

    stat = client_iprpc_remote_call( priv_handle );

    if ( ipncl_network_link_fail( stat ) ) {
      link_fail_stat = stat;
      link_fail = 1;
      num_link_fails++;
      stat = iprpc_disconnect( priv_handle );
      printf( "Network connection failed\n" );
    }
    else if ( stat != IPRPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

  } while ( link_fail && ( num_link_fails < MAX_LINK_FAILURES ) );

  if ( link_fail ) {
    ret_stat = link_fail_stat;
    goto err_return;
  }

norm_return:
  return IPRPC_SUCCESS;

err_return:
  return ret_stat;

}

int mult (
  IPRPC_HANDLE handle,
  int a,
  int b,
  int *result
) {

int link_fail, num_link_fails, connect_fail, num_connect_fails, stat, ret_stat;
int link_fail_stat;
PRIV_IPRPC_HANDLE_PTR priv_handle;

  priv_handle = (PRIV_IPRPC_HANDLE_PTR) handle;

  num_link_fails = 0;
  do {

    if ( priv_handle->connect_state == IPRPC_K_UNCONNECTED ) {

      num_connect_fails = 0;
      do {

          connect_fail = 0;
          stat = iprpc_connect ( handle, "TEST", "client data" );
          if ( stat != IPRPC_SUCCESS ) {
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

    client_iprpc_buf_init( priv_handle, MULT, 3 );

    stat = client_iprpc_buf_add_arg( priv_handle, ARG_TYPE_INT,
     ARG_ACCESS_READ, sizeof(int), &a );
    if ( stat != IPRPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

    stat = client_iprpc_buf_add_arg( priv_handle, ARG_TYPE_INT,
     ARG_ACCESS_READ, sizeof(int), &b );
    if ( stat != IPRPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

    stat = client_iprpc_buf_add_arg( priv_handle, ARG_TYPE_INT,
     ARG_ACCESS_WRITE, sizeof(int), result );
    if ( stat != IPRPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

    link_fail = 0;

    stat = client_iprpc_remote_call( priv_handle );

    if ( ipncl_network_link_fail( stat ) ) {
      link_fail_stat = stat;
      link_fail = 1;
      num_link_fails++;
      stat = iprpc_disconnect( priv_handle );
      printf( "Network connection failed\n" );
    }
    else if ( stat != IPRPC_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

  } while ( link_fail && ( num_link_fails < MAX_LINK_FAILURES ) );

  if ( link_fail ) {
    ret_stat = link_fail_stat;
    goto err_return;
  }

norm_return:
  return IPRPC_SUCCESS;

err_return:
  return ret_stat;

}
