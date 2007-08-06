/***
/* includes
*/
#include stdio
#include stdlib
#include descrip
#include $vaxelnc
#include $mutex
#include $kernelmsg

#include sys_types
#include nsv_priv

/***
/* package globals
*/

static void post_log_msg (
  char *msg
) {

  printf( "%s\n", msg );

}

int nsv_create_named_port (
  char *obj_name,
  int max_msgs,
  int max_msg_size,
  char *label,
  RPC_PORT *port
) {

/***
/* create communication port and declared network object name
*/

PRIV_PORT_P priv_port;
struct dsc$descriptor name_d;
int stat, ret_stat, buf_size;

  if ( !obj_name ) {
    ret_stat = NSV_BADPARAM;
    goto err_ret;
  }

  if ( strlen( obj_name ) > 0 ) {
    MAKE_FIXED_DESCRIP( obj_name, strlen( obj_name ), name_d )
  }
  else {
    ret_stat = NSV_BADPARAM;
    goto err_ret;
  }

/***
/* allocate and populate port object
*/
  priv_port = (PRIV_PORT_P) calloc( 1, sizeof(PRIV_PORT_T) );
  if ( !priv_port ) {
    ret_stat = NSV_NOMEM;
    goto err_ret;
  }

  priv_port->app_data = NULL;

  priv_port->port_type = PORT_K_NAMED;
  priv_port->conn_type = PORT_K_SERVER;

  priv_port->eln_port_name = NULL;
  priv_port->eln_port_name = (char *) calloc( 1, strlen(obj_name)+1 );
  if ( !priv_port->eln_port_name ) {
    ret_stat = NSV_NOMEM;
    goto err_ret;
  }
  strcpy( priv_port->eln_port_name, obj_name );

  priv_port->client_data = NULL;

  priv_port->port_label = NULL;
  if ( strlen(label) > 0 ) {
    priv_port->port_label = (char *) calloc( 1, strlen(label)+1 );
    if ( !priv_port->port_label ) {
      ret_stat = NSV_NOMEM;
      goto err_ret;
    }
    strcpy( priv_port->port_label, label );
  }
  else {
    priv_port->port_label = NULL;
  }

  ker$create_port (
   &stat,
   &priv_port->eln_port,
   max_msgs );
  if ( stat != KER$_SUCCESS ) {
    ret_stat = stat;
    goto err_ret;
  }

  ker$create_name (
   &stat,
   &priv_port->eln_port_name,
   &name_d,
   &priv_port->eln_port,
   NAME$LOCAL );
  if ( stat != KER$_SUCCESS ) {
    ret_stat = stat;
    goto err_ret;
  }

  *port = (RPC_PORT) priv_port;

norm_ret:
  return NSV_SUCCESS;

err_ret:
  *port = (RPC_PORT) 0;
  return ret_stat;

}

int nsv_create_port (
  int max_msgs,
  int max_msg_size,
  char *label,
  RPC_PORT *port
) {

/***
/* create communication port
*/

PRIV_PORT_P priv_port;
int stat, ret_stat, buf_size;

/***
/* allocate and populate port object
*/
  priv_port = (PRIV_PORT_P) calloc( 1, sizeof(PRIV_PORT_T) );
  if ( !priv_port ) {
    ret_stat = NSV_NOMEM;
    goto err_ret;
  }

  priv_port->port_type = PORT_K_UNNAMED;
  priv_port->eln_port_name = NULL;
  priv_port->app_data = NULL;
  priv_port->client_data = NULL;

  priv_port->port_label = NULL;
  if ( strlen(label) > 0 ) {
    priv_port->port_label = (char *) calloc( 1, strlen(label)+1 );
    if ( !priv_port->port_label ) {
      ret_stat = NSV_NOMEM;
      goto err_ret;
    }
    strcpy( priv_port->port_label, label );
  }
  else {
    priv_port->port_label = NULL;
  }

  ker$create_port (
   &stat,
   &priv_port->eln_port,
   max_msgs );
  if ( stat != KER$_SUCCESS ) {
    ret_stat = stat;
    goto err_ret;
  }

  *port = (RPC_PORT) priv_port;

norm_ret:
  return NSV_SUCCESS;

err_ret:
  *port = (RPC_PORT) 0;
  return ret_stat;

}

int nsv_disconnect (
  RPC_PORT port
) {

int stat, ret_stat;
PRIV_PORT_P priv_port;
IO_STAT_BLK_T iosb;

  if ( !port ) {
    ret_stat = NSV_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_PORT_P) port;

  if ( priv_port->port_type == PORT_K_NAMED ) {
    ret_stat = NSV_NOTCON;
    goto err_ret;
  }

/***
/* disconnect
*/
  ker$disconnect_circuit( &stat, &priv_port->eln_port );
  if ( stat != KER$_SUCCESS ) {
    ret_stat = stat;
    goto err_ret;
  }

norm_ret:
  return NSV_SUCCESS;

err_ret:
  return ret_stat;

}

int nsv_delete_port (
  RPC_PORT *port
) {

int stat, ret_stat;
PRIV_PORT_P priv_port;

  priv_port = (PRIV_PORT_P) *port;

  if ( !priv_port ) {
    ret_stat = NSV_BADPARAM;
    goto err_ret;
  }

  if ( priv_port->port_type == PORT_K_NAMED ) {

    ker$delete( &stat, priv_port->eln_port_name );
    if ( stat != KER$_SUCCESS ) {
      ret_stat = stat;
      goto err_ret;
    }

    ker$delete( &stat, &priv_port->eln_port );
    if ( stat != KER$_SUCCESS ) {
      ret_stat = stat;
      goto err_ret;
    }

  }

  if ( priv_port->eln_port_name ) {
    free( priv_port->eln_port_name );
    priv_port->eln_port_name = NULL;
  }
  if ( priv_port->port_label ) {
    free( priv_port->port_label );
    priv_port->port_label = NULL;
  }
  if ( priv_port->client_data ) {
    free( priv_port->client_data );
    priv_port->client_data = NULL;
  }

  free( priv_port );

  *port = NULL;

norm_ret:
  return NSV_SUCCESS;

err_ret:
  return ret_stat;

}

int nsv_accept_connection (
  RPC_PORT connect_port, /* port through which connect requests enter */
  RPC_PORT data_port,	 /* new port through which subsequent
			    messages are exchanged */
  char *optional_data	 /* up to 16 chars + zero */
) {

int stat, ret_stat;
char logbuf[128+1];
PRIV_PORT_P priv_connect_port, priv_data_port;
int connected;
VARYING_STRING(16) vs_con_data;

  if ( !connect_port ) {
    ret_stat = NSV_BADPARAM;
    goto err_ret;
  }

  if ( !data_port ) {
    ret_stat = NSV_BADPARAM;
    goto err_ret;
  }

  priv_connect_port = (PRIV_PORT_P) connect_port;
  priv_data_port = (PRIV_PORT_P) data_port;

  if ( optional_data ) {

    strncpy( vs_con_data.data, optional_data, 15 );
    vs_con_data.data[15] = 0;
    vs_con_data.count = strlen(vs_con_data.data);

  }
  else {

    vs_con_data.count = 0;

  }

  ker$accept_circuit( &stat, &priv_connect_port->eln_port,
   &priv_data_port->eln_port, NULL, NULL, NULL );
  if ( stat != KER$_SUCCESS ) {
    ret_stat = stat;
    goto err_ret;
  }

norm_ret:
  return NSV_SUCCESS;

err_ret:
  return ret_stat;

}

int nsv_wait_on_port (
  RPC_PORT port,
  SYS_TIME_TYPE *timeout,
  int *result
) {

int stat, ret_stat;
PRIV_PORT_P priv_port;

  if ( !port ) {
    ret_stat = NSV_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_PORT_P) port;

  ker$wait_any( &stat, result, timeout, &priv_port->eln_port );
  if ( stat != KER$_SUCCESS ) {
    ret_stat = stat;
    goto err_ret;
  }

norm_ret:
  return NSV_SUCCESS;

err_ret:
  return ret_stat;

}

int nsv_receive_msg (
  RPC_PORT port,
  int buf_len,
  int *msg_len,
  char *msg
) {

int stat, ret_stat;
PORT_MSG_P cur_msg;
PRIV_PORT_P priv_port;
char *data;
int size;
MESSAGE message;

  if ( !port ) {
    ret_stat = NSV_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_PORT_P) port;

  ker$receive( &stat, &message, &data, &size,
   &priv_port->eln_port, NULL, NULL );
  if ( stat != KER$_SUCCESS ) {
    ret_stat = stat;
    goto err_ret;
  }

  if ( size > buf_len ) size = buf_len;

  memcpy( msg, data, size );
  *msg_len = size;

  ker$delete( &stat, message );
  if ( stat != KER$_SUCCESS ) {
    ret_stat = stat;
    goto err_ret;
  }

norm_ret:
  return NSV_SUCCESS;

err_ret:
  return ret_stat;

}

int nsv_send_msg (
  RPC_PORT port,
  int msg_len,
  char *msg
) {

int stat, ret_stat;
PRIV_PORT_P priv_port;
char *data;
MESSAGE message;
int message_created = FALSE;

  if ( !port ) {
    ret_stat = NSV_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_PORT_P) port;

  ker$create_message( &stat, &message, &data, msg_len );
  if ( stat != KER$_SUCCESS ) {
    ret_stat = stat;
    goto err_ret;
  }

  message_created = TRUE;

  memcpy( data, msg, msg_len );

  ker$send( &stat, message, msg_len, &priv_port->eln_port, NULL, FALSE );
  if ( stat != KER$_SUCCESS ) {
    ret_stat = stat;
    goto err_ret;
  }

norm_ret:
  return NSV_SUCCESS;

err_ret:
  return ret_stat;

}
#ifdef xxxxxxx

int nsv_send_event_msg (
  RPC_PORT port,
  int msg_len,
  char *msg
) {

IO_STAT_BLK_T iosb;
int stat, ret_stat;
char logbuf[128+1];
PRIV_PORT_P priv_port;

  if ( !port ) {
    ret_stat = NSV_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_PORT_P) port;

  stat = sys$qiow( 0, priv_port->net_dev_chan, IO$_WRITEVBLK|IO$M_INTERRUPT,
   &iosb, 0, 0, msg, msg_len, 0, 0, 0, 0 );
  if ( stat & STS$M_SUCCESS ) stat = iosb.status;
  if ( ! ( stat & STS$M_SUCCESS ) ) {
    ret_stat = stat;
    goto err_ret;
  }

norm_ret:
  return NSV_SUCCESS;

err_ret:
  return ret_stat;

}
#endif
