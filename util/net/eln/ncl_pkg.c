/* includes
*/
#include stdio
#include stdlib
#include descrip
#include $vaxelnc
#include $kernelmsg

#include sys_types
#include ncl_priv

#define MBX_DATA_SIZE 64

/***
/* package globals
*/

int ncl_network_link_fail (
  int stat
) {

  if ( stat == KER$_CONNECT_PENDING ||
       stat == KER$_DISCONNECT ||
       stat == KER$_NO_MESSAGE ||
       stat == KER$_NO_SUCH_PORT ||
       stat == KER$_BAD_VALUE ||
       stat == NCL_TIMEOUT ) {

    return 1;

  }
  else {

    return 0;

  }

}

int ncl_create_port (
  int max_msgs,
  int max_msg_size,
  char *label,
  RPC_PORT *port
) {

/***
/* allocate and initialize port object
*/

PRIV_PORT_P priv_port;
int stat, ret_stat, buf_size;

int port_allocated = 0;
int label_allocated = 0;

  priv_port = (PRIV_PORT_P) calloc( 1, sizeof(PRIV_PORT_T) );
  if ( !priv_port ) {
    ret_stat = NCL_NOMEM;
    goto err_ret;
  }
  port_allocated = 1;

  priv_port->port_type = PORT_K_UNNAMED;
  priv_port->conn_type = PORT_K_CLIENT;

  priv_port->client_data = NULL;

  priv_port->port_label = NULL;
  if ( strlen(label) > 0 ) {
    priv_port->port_label = (char *) calloc( 1, strlen(label)+1 );
    if ( !priv_port->port_label ) {
      ret_stat = NCL_NOMEM;
      goto err_ret;
    }
    label_allocated = 1;
    strcpy( priv_port->port_label, label );
  }
  else {
    priv_port->port_label = NULL;
  }

  priv_port->max_msgs = max_msgs;
  priv_port->max_msg_size = max_msg_size;
  priv_port->msg_count = 0;
  priv_port->incomplete_io_count = 0;

  ker$create_port( &stat, &priv_port->eln_port, max_msgs );
  if ( stat != KER$_SUCCESS ) {
    ret_stat = stat;
    goto err_ret;
  }

  *port = (RPC_PORT) priv_port;

norm_ret:
  return NCL_SUCCESS;

err_ret:

  if ( label_allocated ) {
    free( priv_port->port_label );
  }
  if ( port_allocated ) {
    free( priv_port );
  }

  *port = (RPC_PORT) 0;

  return ret_stat;

}

int ncl_disconnect (
  RPC_PORT port
) {

int stat, ret_stat;
PRIV_PORT_P priv_port;
IO_STAT_BLK_T iosb;
PORT_MSG_P cur_msg, next_msg;

/***
/* check for NULL value
*/
  if ( !port ) {
    ret_stat = NCL_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_PORT_P) port;

/***
/* make sure this is a data port and not a connection port
*/
  if ( priv_port->port_type == PORT_K_NAMED ) {
    ret_stat = NCL_NOTCON;
    goto err_ret;
  }

  ret_stat = NCL_SUCCESS;

/***
/* disconnect
*/
  ker$disconnect_circuit( &stat, &priv_port->eln_port );
  if ( stat != KER$_SUCCESS ) {
    ret_stat = stat;
    goto err_ret;
  }

norm_ret:
  return NCL_SUCCESS;

err_ret:
  return ret_stat;

}

int ncl_delete_port (
  RPC_PORT *port
) {

int stat, ret_stat;
PRIV_PORT_P priv_port;
PORT_MSG_P cur_msg, next_msg;

/***
/* check for NULL value
*/
  if ( !port ) {
    ret_stat = NCL_BADPARAM;
    goto err_ret;
  }

  ret_stat = NCL_SUCCESS;

  priv_port = (PRIV_PORT_P) *port;

/***
/* if all has succeeded, free memory
*/

/***
/* free port data
*/
  if ( priv_port->client_data ) {
    free( priv_port->client_data );
    priv_port->client_data = NULL;
  }
  if ( priv_port->port_label ) {
    free( priv_port->port_label );
    priv_port->port_label = NULL;
  }

/***
/* delete eln port
*/
  ker$delete( &stat, &priv_port->eln_port );
  if ( stat != KER$_SUCCESS ) {
    ret_stat = stat;
    goto err_ret;
  }

/***
/* free port data structure
*/
  free( priv_port );

  *port = NULL;

norm_ret:
  return NCL_SUCCESS;

err_ret:
  return ret_stat;

}

int ncl_connect (
  char *node,
  char *task,
  char *optional_data,	/* up to 14 bytes */
  RPC_PORT port		/* new port through which subsequent
			   messages are exchanged */
) {

int stat, ret_stat, index, len, i, count;
char c_len;
PRIV_PORT_P priv_port;
char port_name[127+1];
struct dsc$descriptor dsc;
VARYING_STRING(16) vs_con_data;
VARYING_STRING(16) vs_accept_data;

  if ( !port ) {
    ret_stat = NCL_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_PORT_P) port;

  if ( optional_data ) {

    strncpy( vs_con_data.data, optional_data, 15 );
    vs_con_data.data[15] = 0;
    vs_con_data.count = strlen(vs_con_data.data);

  }
  else {

    vs_con_data.count = 0;

  }

/***
/* build port name
*/
  strcpy( port_name, node );
  strcat( port_name, "::" );
  strcat( port_name, task );

/***
/* setup descriptor
*/
  MAKE_FIXED_DESCRIP( port_name, strlen(port_name), dsc )

/***
/* connect circuit
*/
  ker$connect_circuit( &stat, &priv_port->eln_port, NULL, &dsc,
   FALSE, &vs_con_data, &vs_accept_data );
  if ( stat != KER$_SUCCESS ) {
    ret_stat = stat;
    goto err_ret;
  }

norm_ret:
  return NCL_SUCCESS;

err_ret:
  return ret_stat;

}

int ncl_wait_on_port (
  RPC_PORT port,
  SYS_TIME_TYPE *timeout,
  int *result
) {

int stat, ret_stat;
PRIV_PORT_P priv_port;

  if ( !port ) {
    ret_stat = NCL_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_PORT_P) port;

  ker$wait_any( &stat, result, timeout, &priv_port->eln_port );
  if ( stat != KER$_SUCCESS ) {
    ret_stat = stat;
    goto err_ret;
  }

norm_ret:
  return NCL_SUCCESS;

err_ret:
  return ret_stat;

}

int ncl_receive_msg (
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
    ret_stat = NCL_BADPARAM;
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
  return NCL_SUCCESS;

err_ret:
  return ret_stat;

}

int ncl_send_msg (
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
    ret_stat = NCL_BADPARAM;
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
  return NCL_SUCCESS;

err_ret:
  if ( message_created ) ker$delete( &stat, message );
  return ret_stat;

}
#ifdef xxxxxx

int ncl_wait_on_event (
  RPC_PORT port,
  int *timeout,
  int *result
) {

IO_STAT_BLK_T iosb;
int stat, ret_stat, i, count;
PORT_MSG_P cur_msg;
PRIV_PORT_P priv_port;

struct mbx_blk {
  short msg_type;
  short unit;
  char name_info[MBX_DATA_SIZE];
} mbx_block;

  if ( !port ) {
    ret_stat = NCL_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_PORT_P) port;

  if ( priv_port->msg_in_mbox ) {
    ret_stat = NCL_MBXFULL;
    goto err_ret;
  }

  stat = sys$qiow( 0, priv_port->net_obj_mbx_chan, IO$_READVBLK,
   &iosb, 0, 0, &mbx_block, sizeof(mbx_block), 0, 0, 0, 0 );
  if ( stat & STS$M_SUCCESS ) stat = iosb.status;
  if ( ! ( stat & STS$M_SUCCESS ) ) {
    ret_stat = stat;
    goto err_ret;
  }

  i = mbx_block.name_info[0] + 1;
  count = mbx_block.name_info[i];
  i++;

  if ( count > 0 ) {
    memcpy( priv_port->mbx_data, &mbx_block.name_info[i], count );
  }
  priv_port->mbx_data[count] = 0;

  priv_port->msg_in_mbox = TRUE;
  priv_port->mbox_msg_len = count;

  *result = 1;

norm_ret:
  return NCL_SUCCESS;

err_ret:
  return ret_stat;

}

int ncl_receive_event_msg (
  RPC_PORT port,
  int buf_len,
  int *msg_len,
  char *msg
) {

int stat, ret_stat;
PRIV_PORT_P priv_port;

  if ( !port ) {
    ret_stat = NCL_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_PORT_P) port;

  if ( !priv_port->msg_in_mbox ) {
    ret_stat = NCL_PORTEMPTY;
    goto err_ret;
  }

  /* copy message to application buffer */
  if ( priv_port->mbox_msg_len > buf_len )
    *msg_len = buf_len;
  else
    *msg_len = priv_port->mbox_msg_len;
  memcpy( msg, priv_port->mbx_data, *msg_len );

  priv_port->msg_in_mbox = FALSE;

norm_ret:
  return NCL_SUCCESS;

err_ret:
  return ret_stat;

}
#endif
