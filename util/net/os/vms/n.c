/***
/* includes
*/
#include stdio
#include stdlib
#include descrip
#include dvidef
#include iodef
#include msgdef
#include nfbdef
#include ssdef
#include stsdef

#include ncl_priv

#define MBX_DATA_SIZE 64

/***
/* package globals
*/
static $DESCRIPTOR ( g_net_device, "_NET:" );

int ncl_create_port (
  int max_msgs,
  int max_msg_size,
  char *label,
  PORT *port
) {

/***
/* allocate and initialize port object
*/

PRIV_PORT_P priv_port;
int ret_stat, buf_size;

int port_allocated = 0;
int ef_allocated = 0;
int label_allocated = 0;
int sentinel_nodes_allocated = 0;
int io_sentinel_nodes_allocated = 0;

  priv_port = (PRIV_PORT_P) calloc( 1, sizeof(PRIV_PORT_T) );
  if ( !priv_port ) {
    ret_stat = NCL_NOMEM;
    goto err_ret;
  }
  mem_alloc( "NCL", sizeof(PRIV_PORT_T) );
  port_allocated = 1;

  lib$get_ef( &priv_port->port_event_flag );
  if ( priv_port->port_event_flag == 1 ) {	/* no ef's available */
    ret_stat = NCL_NOEF;
    goto err_ret;
  }
  mem_alloc( "NCL", 1 );
  ef_allocated = 1;

  priv_port->port_type = PORT_K_UNNAMED;
  priv_port->conn_type = PORT_K_CLIENT;
  priv_port->net_obj_mbx_chan = 0;
  priv_port->net_dev_chan = 0;
  priv_port->net_obj_mbx_unit = 0;
  priv_port->net_dev_unit = 0;

  priv_port->net_obj_name = NULL;

  priv_port->client_data = NULL;

  priv_port->port_label = NULL;
  if ( strlen(label) > 0 ) {
    priv_port->port_label = (char *) calloc( 1, strlen(label)+1 );
    if ( !priv_port->port_label ) {
      ret_stat = NCL_NOMEM;
      goto err_ret;
    }
    mem_alloc( "NCL", strlen(label)+1 );
    label_allocated = 1;
    strcpy( priv_port->port_label, label );
  }
  else {
    priv_port->port_label = NULL;
  }

  /* create sentinel nodes for the port message buffer */
  priv_port->head = (PORT_MSG_P) calloc( 1, sizeof(PORT_MSG_T) );
  if ( !priv_port->head ) {
    ret_stat = NCL_NOMEM;
    goto err_ret;
  }
  mem_alloc( "NCL", sizeof(PORT_MSG_T) );
  sentinel_nodes_allocated = 1;
  priv_port->tail = priv_port->head;
  priv_port->tail->next = NULL;

  /* create incomplete I/O sentinel nodes for the port message buffer */
  priv_port->incomplete_io_head = (PORT_MSG_P) calloc( 1, sizeof(PORT_MSG_T) );
  if ( !priv_port->incomplete_io_head ) {
    ret_stat = NCL_NOMEM;
    goto err_ret;
  }
  mem_alloc( "NCL", sizeof(PORT_MSG_T) );
  io_sentinel_nodes_allocated = 1;
  priv_port->incomplete_io_tail = priv_port->incomplete_io_head;
  priv_port->incomplete_io_tail->next = NULL;

  priv_port->max_msgs = max_msgs;
  priv_port->max_msg_size = max_msg_size;
  priv_port->msg_count = 0;
  priv_port->incomplete_io_count = 0;

  *port = (PORT) priv_port;

norm_ret:
  return NCL_SUCCESS;

err_ret:

  if ( io_sentinel_nodes_allocated ) {
    mem_free( "NCL", sizeof(PORT_MSG_T) );
    free( priv_port->incomplete_io_head );
  }
  if ( sentinel_nodes_allocated ) {
    mem_free( "NCL", sizeof(PORT_MSG_T) );
    free( priv_port->head );
  }
  if ( label_allocated ) {
    mem_free( "NCL", strlen(priv_port->port_label)+1 );
    free( priv_port->port_label );
  }
  if ( ef_allocated ) {
    mem_free( "NCL", 1 );
    lib$free_ef( &priv_port->port_event_flag );
  }
  if ( port_allocated ) {
    mem_free( "NCL", sizeof(PRIV_PORT_T) );
    free( priv_port );
  }

  *port = (PORT) 0;

  return ret_stat;

}

int ncl_disconnect (
  PORT port
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
/* make sure this is a data port and not a connect port
*/
  if ( priv_port->port_type == PORT_K_NAMED ) {
    ret_stat = NCL_NOTCON;
    goto err_ret;
  }

  ret_stat = NCL_SUCCESS;

/***
/* disconnect asychronously
*/
  stat = sys$qiow( 0, priv_port->net_dev_chan, IO$_DEACCESS|IO$M_ABORT,
   &iosb, 0, 0, 0, 0, 0, 0, 0, 0 );
  if ( stat & STS$M_SUCCESS ) stat = iosb.status;
  if ( ! ( stat & STS$M_SUCCESS ) ) {
    ret_stat = stat;
  }

/***
/* cancel I/O and deassign ports
*/
  if ( priv_port->net_dev_chan ) {
    sys$cancel( priv_port->net_dev_chan );
    stat = sys$dassgn( priv_port->net_dev_chan );
    if ( ! ( stat & STS$M_SUCCESS ) ) {
      ret_stat = stat;
    }
    priv_port->net_dev_chan = 0;
  }

  if ( priv_port->net_obj_mbx_chan ) {
    sys$cancel( priv_port->net_obj_mbx_chan );
    stat = sys$dassgn( priv_port->net_obj_mbx_chan );
    if ( ! ( stat & STS$M_SUCCESS ) ) {
      ret_stat = stat;
    }
    priv_port->net_obj_mbx_chan = 0;
  }

  if ( ret_stat != NCL_SUCCESS ) goto err_ret;

/***
/* if all has succeeded, clean up incomplete I/O list (this list
/* grows as the result of I/O timeouts; the memory should not be freed
/* until I/O is cancelled)
*/
  cur_msg = priv_port->incomplete_io_head->next;
  while ( cur_msg ) {
    next_msg = cur_msg->next;
    if ( cur_msg->msg ) {
      mem_free( "NCL", priv_port->max_msg_size+1 );
      free( cur_msg->msg );
    }
    mem_free( "NCL", sizeof(PORT_MSG_T) );
    free( cur_msg );
    cur_msg = next_msg;
  }
  priv_port->incomplete_io_tail = priv_port->incomplete_io_head;
  priv_port->incomplete_io_tail->next = NULL;

norm_ret:
  return NCL_SUCCESS;

err_ret:
  return ret_stat;

}

int ncl_delete_port (
  PORT *port
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
/* cancel I/O and deassign ports (in case channels are still assigned)
*/
  if ( priv_port->net_dev_chan ) {
    sys$cancel( priv_port->net_dev_chan );
    stat = sys$dassgn( priv_port->net_dev_chan );
    if ( ! ( stat & STS$M_SUCCESS ) ) {
      ret_stat = stat;
    }
    priv_port->net_dev_chan = 0;
  }

  if ( priv_port->net_obj_mbx_chan ) {
    sys$cancel( priv_port->net_obj_mbx_chan );
    stat = sys$dassgn( priv_port->net_obj_mbx_chan );
    if ( ! ( stat & STS$M_SUCCESS ) ) {
      ret_stat = stat;
    }
    priv_port->net_obj_mbx_chan = 0;
  }

  if ( ret_stat != NCL_SUCCESS ) goto err_ret;

/***
/* if all has succeeded, free memory
*/

/***
/* free port data
*/
  if ( priv_port->net_obj_name ) {
    mem_free( "NCL", strlen(priv_port->net_obj_name)+1 );
    free( priv_port->net_obj_name );
  }
  if ( priv_port->client_data ) {
    mem_free( "NCL", priv_port->client_data_size );
    free( priv_port->client_data );
  }
  if ( priv_port->port_label ) {
    mem_free( "NCL", strlen(priv_port->port_label)+1 );
    free( priv_port->port_label );
  }
  mem_free( "NCL", 1 );
  lib$free_ef( &priv_port->port_event_flag );

/***
/* free message data
*/
  cur_msg = priv_port->head;
  while ( cur_msg ) {
    next_msg = cur_msg->next;
    if ( cur_msg->msg ) {
      mem_free( "NCL", priv_port->max_msg_size+1 );
      free( cur_msg->msg );
    }
    mem_free( "NCL", sizeof(PORT_MSG_T) );
    free( cur_msg );
    cur_msg = next_msg;
  }

/***
/* Clean up incomplete I/O list (this list grows as the result of
/* I/O timeouts; the memory should not be freed until I/O is
/* cancelled)
*/
  cur_msg = priv_port->incomplete_io_head;
  while ( cur_msg ) {
    next_msg = cur_msg->next;
    if ( cur_msg->msg ) {
      mem_free( "NCL", priv_port->max_msg_size+1 );
      free( cur_msg->msg );
    }
    mem_free( "NCL", sizeof(PORT_MSG_T) );
    free( cur_msg );
    cur_msg = next_msg;
  }

/***
/* free port data structure
*/
  mem_free( "NCL", sizeof(PRIV_PORT_T) );
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
  PORT port		/* new port through which subsequent
			   messages are exchanged */
) {

int stat, ret_stat, index, len, i, count;
char c_len;
IO_STAT_BLK_T iosb;
PRIV_PORT_P priv_port;
char ncb[MAX_NCB+128];
struct dsc$descriptor dsc;

float delay = 5.0; /* seconds */

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

/***
/* build net connect block
*/
  strncpy( ncb, node, MAX_NCB );
  strncat( ncb, "::\"task=", MAX_NCB );
  strncat( ncb, task, MAX_NCB );
  index = strlen( ncb );

  if ( optional_data ) {

    memcpy( &ncb[index], "/\000\000", 3 );

    len = strlen( optional_data );
    if ( len > 16 ) len = 16;
    c_len = (char) len;

    index += 3;
    ncb[index] = c_len;

    for ( i=0; i<len; i++ ) {
      index++;
      ncb[index] = optional_data[i];
    }

    index++;
    ncb[index] = '\"';

    index++;
    ncb[index] = 0;

  }
  else {

    memcpy( &ncb[index], "\"\000\000", 3 );

  }

/***
/* setup descriptor
*/
  MAKE_FIXED_DESCRIP( ncb, MAX_NCB, dsc )

/***
/* assign channel to net device and associate with mailbox
*/
  stat = lib$asn_wth_mbx( &g_net_device, &priv_port->mbx_max_msg,
   &priv_port->mbx_buf_quo, &priv_port->net_dev_chan,
   &priv_port->net_obj_mbx_chan );
  if ( ! ( stat & STS$M_SUCCESS ) ) {
    ret_stat = stat;
    goto err_ret;
  }

/***
/* make the connect request
*/
  iosb.status = 0;
  stat = sys$qiow( 0, priv_port->net_dev_chan, IO$_ACCESS, &iosb,
   0, 0, 0, &dsc, 0, 0, 0, 0 );
  if ( stat & STS$M_SUCCESS ) {
    count = 60;
    while ( count && !iosb.status ) {
      delay = 1.0;
      lib$wait( &delay );
      count--;
    }
    stat = iosb.status;
  }
  if ( ! ( stat & STS$M_SUCCESS ) ) {
    ret_stat = stat;
    sys$cancel( priv_port->net_dev_chan );
    sys$cancel( priv_port->net_obj_mbx_chan );
    sys$dassgn( priv_port->net_dev_chan );
    sys$dassgn( priv_port->net_obj_mbx_chan );
    delay = 5.0;
    lib$wait( &delay );
    goto err_ret;
  }

/***
/* read empty message out of net mailbox
*/
  iosb.status = 0;
  stat = sys$qiow( 0, priv_port->net_obj_mbx_chan, IO$_READVBLK,
   &iosb, 0, 0, &mbx_block, sizeof(mbx_block), 0, 0, 0, 0 );
  if ( stat & STS$M_SUCCESS ) {
    count = 60;
    while ( count && !iosb.status ) {
      delay = 1.0;
      lib$wait( &delay );
      count--;
    }
    stat = iosb.status;
  }
  if ( ! ( stat & STS$M_SUCCESS ) ) {
    ret_stat = stat;
    sys$cancel( priv_port->net_dev_chan );
    sys$cancel( priv_port->net_obj_mbx_chan );
    sys$dassgn( priv_port->net_dev_chan );
    sys$dassgn( priv_port->net_obj_mbx_chan );
    delay = 5.0;
    lib$wait( &delay );
    goto err_ret;
  }


norm_ret:
  return NCL_SUCCESS;

err_ret:
  return ret_stat;

}

int ncl_wait_on_port (
  PORT port,
  int *timeout,
  int *result
) {

int stat, ret_stat;
PORT_MSG_P cur_msg, next_msg;
PRIV_PORT_P priv_port;

  if ( !port ) {
    ret_stat = NCL_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_PORT_P) port;

/***
/* Note that if an I/O timeout occurs, then cur_msg and cur_msg->msg
/* get put on the incomplete I/O list and may be forever lost. Thus,
/* whenever priv_port->incomplete_io_count is non-zero the list will
/* be traversed and memory freed if the iosb field is non-zero (which
/* indicates late I/O completion).
*/
  if ( priv_port->incomplete_io_count ) {
    cur_msg = priv_port->incomplete_io_head;
    while ( cur_msg ) {
      next_msg = cur_msg->next;
      if ( cur_msg->iosb.status ) {
        mem_free( "NCL", priv_port->max_msg_size+1 );
        free( cur_msg->msg );
        mem_free( "NCL", sizeof(PORT_MSG_T) );
        free( cur_msg );
        priv_port->incomplete_io_count--;
      }
      cur_msg = next_msg;
    }
  }

  if ( priv_port->msg_count == priv_port->max_msgs ) {
    ret_stat = NCL_PORTFULL;
    goto err_ret;
  }

  cur_msg = (PORT_MSG_P) calloc( 1, sizeof(PORT_MSG_T) );
  if ( !cur_msg ) {
    ret_stat = NCL_NOMEM;
    goto err_ret;
  }
  mem_alloc( "NCL", sizeof(PORT_MSG_T) );

  cur_msg->msg = (char *) calloc( 1, priv_port->max_msg_size+1 );
  if ( !cur_msg->msg ) {
    ret_stat = NCL_NOMEM;
    mem_free( "NCL", sizeof(PORT_MSG_T) );
    free( cur_msg );
    goto err_ret;
  }
  mem_alloc( "NCL", priv_port->max_msg_size+1 );

  cur_msg->msg_len = 0;

  cur_msg->iosb.status = 0;

  /* queue asynchronous I/O request, set event flag on completion */
  stat = sys$qio( priv_port->port_event_flag, priv_port->net_dev_chan,
   IO$_READVBLK, &cur_msg->iosb, 0, 0, cur_msg->msg, priv_port->max_msg_size,
   0, 0, 0, 0 );
  if ( ! ( stat & STS$M_SUCCESS ) ) {
    mem_free( "NCL", priv_port->max_msg_size+1 );
    free( cur_msg->msg );
    mem_free( "NCL", sizeof(PORT_MSG_T) );
    free( cur_msg );
    ret_stat = stat;
    goto err_ret;
  }

  if ( timeout ) {

    /* set time-out timer, set event flag if time-out occurs */
    stat = sys$setimr( priv_port->port_event_flag, timeout, 0,
     priv_port->port_event_flag, 0 );
    if ( ! ( stat & STS$M_SUCCESS ) ) {
      priv_port->incomplete_io_tail->next = cur_msg;
      priv_port->incomplete_io_tail = cur_msg;
      priv_port->incomplete_io_tail->next = NULL;
      priv_port->incomplete_io_count++;
      ret_stat = stat;
      goto err_ret;
    }

  }

  if ( !cur_msg->iosb.status ) {
    /* wait for event flag to set (I/O completion or time-out) */
    sys$waitfr( priv_port->port_event_flag );
  }

  /* if cur_msg->iosb.status is zero then must be a timeout */
  stat = cur_msg->iosb.status;
  if ( !stat ) {
    ret_stat = NCL_TIMEOUT;
    *result = 0;				/* timeout */
    priv_port->incomplete_io_tail->next = cur_msg;
    priv_port->incomplete_io_tail = cur_msg;
    priv_port->incomplete_io_tail->next = NULL;
    priv_port->incomplete_io_count++;
    goto err_ret;
  }

  *result = 1;			/* I/O completed, no timeout */

  /* cancel timer */
  sys$cantim( priv_port->port_event_flag, 0 );

  /* check status of completed I/O */
  stat = cur_msg->iosb.status;
  if ( ! ( stat & STS$M_SUCCESS ) ) {
    mem_free( "NCL", priv_port->max_msg_size+1 );
    free( cur_msg->msg );
    mem_free( "NCL", sizeof(PORT_MSG_T) );
    free( cur_msg );
    ret_stat = stat;
    goto err_ret;
  }

  priv_port->tail->next = cur_msg;
  priv_port->tail = cur_msg;
  priv_port->tail->next = NULL;
  priv_port->msg_count++;

  cur_msg->msg_len = cur_msg->iosb.msg_len;

  *result = 1;

norm_ret:
  return NCL_SUCCESS;

err_ret:
  return ret_stat;

}

int ncl_receive_msg (
  PORT port,
  int buf_len,
  int *msg_len,
  char *msg
) {

int stat, ret_stat;
PORT_MSG_P cur_msg;
PRIV_PORT_P priv_port;

  if ( !port ) {
    ret_stat = NCL_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_PORT_P) port;

  if ( priv_port->msg_count == 0 ) {
    ret_stat = NCL_PORTEMPTY;
    goto err_ret;
  }

  /* unlink */
  cur_msg = priv_port->head->next;
  if ( !cur_msg ) {
    ret_stat = NCL_PORTEMPTY;
    goto err_ret;
  }
  priv_port->head->next = cur_msg->next;
  if ( priv_port->tail == cur_msg )
    priv_port->tail = priv_port->head;

  /* decrement count */
  (priv_port->msg_count)--;

  /* copy message to application buffer */
  if ( cur_msg->msg_len > buf_len )
    *msg_len = buf_len;
  else
    *msg_len = cur_msg->msg_len;
  memcpy( msg, cur_msg->msg, *msg_len );

  /* deallocate mem */
  mem_free( "NCL", priv_port->max_msg_size+1 );
  free( cur_msg->msg );
  mem_free( "NCL", sizeof(PORT_MSG_T) );
  free( cur_msg );

norm_ret:
  return NCL_SUCCESS;

err_ret:
  return ret_stat;

}

int ncl_send_msg (
  PORT port,
  int msg_len,
  char *msg
) {

IO_STAT_BLK_T iosb;
int stat, ret_stat;
PRIV_PORT_P priv_port;

  if ( !port ) {
    ret_stat = NCL_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_PORT_P) port;

  stat = sys$qiow( 0, priv_port->net_dev_chan, IO$_WRITEVBLK,
   &iosb, 0, 0, msg, msg_len, 0, 0, 0, 0 );
  if ( stat & STS$M_SUCCESS ) stat = iosb.status;
  if ( ! ( stat & STS$M_SUCCESS ) ) {
    ret_stat = stat;
    goto err_ret;
  }

norm_ret:
  return NCL_SUCCESS;

err_ret:
  return ret_stat;

}

int ncl_wait_on_event (
  PORT port,
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
  PORT port,
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
