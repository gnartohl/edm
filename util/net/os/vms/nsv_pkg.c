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

#include <cma.h>
#include nsv_priv
#include sys_types

/***
/* package globals
*/
static $DESCRIPTOR ( g_net_device, "_NET:" );
static int g_num_signals = 0;

static void ast_routine (
 cma_t_cond *cv
) {

  g_num_signals++;
  cma_cond_signal_int( cv );

}

static void post_log_msg (
  char *msg
) {

  printf( "%s\n", msg );

}

static int establish_link (
  char *name_info,
  PRIV_PORT_P connect_port,
  PRIV_PORT_P data_port
) {

int stat, ret_stat, i, count, start;
struct dsc$descriptor  obj_name_d, mbx_name_d;
IO_STAT_BLK_T iosb;

static struct {			/* item list to get net device unit */
  short int buff_len;
  short int code;
  int *ret_info;
  int ret_len;
  int terminator;
} getdvi_item_list = { 4, DVI$_UNIT, 0, 0, 0 };

  if ( !connect_port ) {
    ret_stat = NSV_BADPARAM;
    goto err_ret;
  }

  if ( !data_port ) {
    ret_stat = NSV_BADPARAM;
    goto err_ret;
  }

/***
/* Copy the information portion of the NCB into client data for port
*/
  i = (int) name_info[0] + 1;
  count = (int) name_info[i];
  start = i + 1;

  data_port->client_data = (char *) calloc( 1, count+1 );
  if ( !data_port->client_data ) {
    ret_stat = NSV_NOMEM;
    goto err_ret;
  }
  data_port->client_data_size = count+1;

  memcpy( data_port->client_data, &name_info[start], count );
  data_port->client_data[count] = 0;

/***
/* Assign a channel to the _NET device and associate the network
/* command mailbox with the channel.
*/
  MAKE_FIXED_DESCRIP( connect_port->net_obj_name,
   strlen(connect_port->net_obj_name), mbx_name_d )

  stat = sys$assign( &g_net_device, &data_port->net_dev_chan, 0,
   &mbx_name_d );
  if ( ! ( stat & STS$M_SUCCESS ) ) {
    ret_stat = stat;
    goto err_ret;
  }

/***
/* Get the unit number of the _NET channel to identify the network
/* command messages.
*/
  getdvi_item_list.ret_info = &data_port->net_dev_unit;

  stat = sys$getdviw( 0, data_port->net_dev_chan, 0, &getdvi_item_list,
   &iosb, 0, 0, 0 );
  if ( stat & STS$M_SUCCESS ) stat = iosb.status;
  if ( !( stat & STS$M_SUCCESS ) ) {
    ret_stat = stat;
    goto err_ret;
  }

/***
/* Accept connection:
/* Issue to the _NET channel a QIO with the IO$_ACCESS function code
/* and the P2 parameter set to the address of the copied NCB
/* information.
*/
  MAKE_FIXED_DESCRIP( data_port->client_data, count, obj_name_d )

  stat = sys$qiow( 0, data_port->net_dev_chan, IO$_ACCESS, &iosb, 0,
   0, 0, &obj_name_d, 0, 0, 0, 0 );

  if ( stat & STS$M_SUCCESS ) stat = iosb.status;
  if ( !( stat & STS$M_SUCCESS ) ) {
    ret_stat = stat;
    goto err_ret;
  }

norm_ret:
  return NSV_SUCCESS;

err_ret:

/***
/* reject connection
*/
  MAKE_FIXED_DESCRIP( data_port->client_data, count, obj_name_d )

  stat = sys$qiow ( 0, connect_port->net_dev_chan, IO$_ACCESS | IO$M_ABORT,
   &iosb, 0, 0, 0, &obj_name_d, 0, 0, 0, 0 );

  return ret_stat;

}

static void get_optional_data (
  PRIV_PORT_P port,
  char *optional_data
) {

int i, start, count;

  strcpy( optional_data, "" );

  i = 0;
  for ( i=0; i<128; i++ ) {

    if ( port->client_data[i] == '/' ) {

      i += 3;				    /* skip over word */
      count = port->client_data[i];
      if ( count > 16 ) count = 16;
      i++;
      start = i;

      strncpy( optional_data, &port->client_data[start], count );
      optional_data[count] = 0;
      return;

    }

  }

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
struct dsc$descriptor name_d, nfb_d;
int stat, ret_stat, buf_size;
short mbx_chan, netdev_chan;
IO_STAT_BLK_T iosb;
THREAD_SIGNAL_PTR thread_signal_rec;
cma_t_attr attr;

struct {
  char func;
  int terminator;
} nfb = { NFB$C_DECLNAME, 0 };		/* network function block */

  MAKE_FIXED_DESCRIP( &nfb, sizeof(nfb), nfb_d )

  if ( strlen( obj_name ) > 0 ) {
    MAKE_FIXED_DESCRIP( obj_name, strlen( obj_name ), name_d )
  }
  else {
    ret_stat = NSV_BADPARAM;
    goto err_ret;
  }

/***
/* create temp mailbox with same name as object
*/
  stat = sys$crembx( TEMP_MBX, &mbx_chan, MAX_MBX_MSG, BUF_QUO, 0, 0,
   &name_d );
  if ( ! ( stat & STS$M_SUCCESS ) ) {
    ret_stat = stat;
    goto err_ret;
  }

/***
/* assign channel to network device and associate with above mailbox
*/
  stat = sys$assign( &g_net_device, &netdev_chan, 0, &name_d );
  if ( ! ( stat & STS$M_SUCCESS ) ) {
    ret_stat = stat;
    goto err_ret;
  }

/***
/* declare network object name
*/
  stat = sys$qiow( 0, netdev_chan, IO$_ACPCONTROL, &iosb, 0, 0,
   &nfb_d, &name_d, 0, 0, 0, 0 );
  if ( stat & STS$M_SUCCESS ) stat = iosb.status;
  if ( ! ( stat & STS$M_SUCCESS ) ) {
    ret_stat = stat;
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

  thread_signal_rec = (THREAD_SIGNAL_PTR) calloc( 1,
   sizeof(THREAD_SIGNAL_TYPE) );
  if ( !thread_signal_rec ) {
    ret_stat = NSV_NOMEM;
    goto err_ret;
  }

  attr = cma_c_null;
  cma_attr_create( &thread_signal_rec->cv_attr, &attr );
  cma_cond_create( &thread_signal_rec->cv, &thread_signal_rec->cv_attr );
  cma_attr_create( &thread_signal_rec->mu_attr, &attr );
  cma_mutex_create( &thread_signal_rec->mu, &thread_signal_rec->mu_attr );

  thread_signal_rec->break_connection = FALSE;

  priv_port->app_data = (void *) thread_signal_rec;

  /* read priv_port->net_obj_mbx_chan to detect connect requests, disconnects,
     link failures, shutdown requests, etc. */

  /* write priv_port->net_dev_chan to reject a connection (and to declare
     network object name as shown in the above write to netdev_chan) */

  priv_port->port_type = PORT_K_NAMED;
  priv_port->conn_type = PORT_K_SERVER;
  priv_port->net_obj_mbx_chan = mbx_chan;
  priv_port->net_dev_chan = netdev_chan;
  priv_port->net_obj_mbx_unit = 0;
  priv_port->net_dev_unit = 0;
  priv_port->mbx_max_msg = MAX_MBX_MSG;
  priv_port->mbx_buf_quo = BUF_QUO;

  priv_port->net_obj_name = NULL;
  priv_port->net_obj_name = (char *) calloc( 1, strlen(obj_name)+1 );
  if ( !priv_port->net_obj_name ) {
    ret_stat = NSV_NOMEM;
    goto err_ret;
  }
  strcpy( priv_port->net_obj_name, obj_name );

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

  /* create sentinel nodes for the port message buffer */
  priv_port->head = (PORT_MSG_P) calloc( 1, sizeof(PORT_MSG_T) );
  if ( !priv_port->head ) {
    ret_stat = NSV_NOMEM;
    goto err_ret;
  }
  priv_port->tail = priv_port->head;
  priv_port->tail->next = NULL;
  priv_port->max_msgs = max_msgs;
  priv_port->max_msg_size = max_msg_size;
  priv_port->msg_count = 0;

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
/* allocate and initialize port object
*/

PRIV_PORT_P priv_port;
int ret_stat, buf_size;
THREAD_SIGNAL_PTR thread_signal_rec;
cma_t_attr attr;

  priv_port = (PRIV_PORT_P) calloc( 1, sizeof(PRIV_PORT_T) );
  if ( !priv_port ) {
    ret_stat = NSV_NOMEM;
    goto err_ret;
  }

  thread_signal_rec = (THREAD_SIGNAL_PTR) calloc( 1,
   sizeof(THREAD_SIGNAL_TYPE) );
  if ( !thread_signal_rec ) {
    ret_stat = NSV_NOMEM;
    goto err_ret;
  }

  attr = cma_c_null;
  cma_attr_create( &thread_signal_rec->cv_attr, &attr );
  cma_cond_create( &thread_signal_rec->cv, &thread_signal_rec->cv_attr );
  cma_attr_create( &thread_signal_rec->mu_attr, &attr );
  cma_mutex_create( &thread_signal_rec->mu, &thread_signal_rec->mu_attr );

  thread_signal_rec->break_connection = FALSE;

  priv_port->app_data = (void *) thread_signal_rec;

  priv_port->port_type = PORT_K_UNNAMED;
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
      ret_stat = NSV_NOMEM;
      goto err_ret;
    }
    strcpy( priv_port->port_label, label );
  }
  else {
    priv_port->port_label = NULL;
  }

  /* create sentinel nodes for the port message buffer */
  priv_port->head = (PORT_MSG_P) calloc( 1, sizeof(PORT_MSG_T) );
  if ( !priv_port->head ) {
    ret_stat = NSV_NOMEM;
    goto err_ret;
  }
  priv_port->tail = priv_port->head;
  priv_port->tail->next = NULL;
  priv_port->max_msgs = max_msgs;
  priv_port->max_msg_size = max_msg_size;
  priv_port->msg_count = 0;

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

  if ( priv_port->client_data ) {
    free( priv_port->client_data );
    priv_port->client_data = NULL;
  }

  if ( priv_port->port_type == PORT_K_NAMED ) {
    ret_stat = NSV_NOTCON;
    goto err_ret;
  }

  sys$cancel( priv_port->net_dev_chan );

  stat = sys$qiow( 0, priv_port->net_dev_chan, IO$_DEACCESS|IO$M_ABORT,
   &iosb, 0, 0, 0, 0, 0, 0, 0, 0 );
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

int nsv_delete_port (
  RPC_PORT *port
) {

int stat, ret_stat;
PRIV_PORT_P priv_port;
THREAD_SIGNAL_PTR thread_signal_rec;

  priv_port = (PRIV_PORT_P) *port;

  if ( !priv_port ) {
    ret_stat = NSV_BADPARAM;
    goto err_ret;
  }

  if ( priv_port->net_dev_chan ) {
    stat = sys$dassgn( priv_port->net_dev_chan );
    if ( ! ( stat & STS$M_SUCCESS ) ) {
      ret_stat = stat;
      goto err_ret;
    }
    priv_port->net_dev_chan = 0;
  }

  if ( priv_port->port_type == PORT_K_NAMED ) {

    if ( priv_port->net_obj_mbx_chan ) {
      stat = sys$dassgn( priv_port->net_obj_mbx_chan );
      if ( ! ( stat & STS$M_SUCCESS ) ) {
        ret_stat = stat;
        goto err_ret;
      }
      priv_port->net_obj_mbx_chan = 0;
    }

  }

  thread_signal_rec = (THREAD_SIGNAL_PTR) priv_port->app_data;
  cma_attr_delete( &thread_signal_rec->cv_attr );
  cma_cond_delete( &thread_signal_rec->cv );
  cma_attr_delete( &thread_signal_rec->mu_attr );
  cma_mutex_delete( &thread_signal_rec->mu );
  free( thread_signal_rec );

  if ( priv_port->net_obj_name ) {
    free( priv_port->net_obj_name );
    priv_port->net_obj_name = NULL;
  }
  if ( priv_port->port_label ) {
    free( priv_port->port_label );
    priv_port->port_label = NULL;
  }
  if ( priv_port->client_data ) {
    free( priv_port->client_data );
    priv_port->client_data = NULL;
  }
  free( priv_port->head );
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
IO_STAT_BLK_T iosb;
char logbuf[128+1];
PRIV_PORT_P priv_connect_port, priv_data_port;
int connected;
THREAD_SIGNAL_PTR thread_signal_rec;

struct mbx_blk {
  short msg_type;
  short unit;
  char name_info[MAX_NCB];
} net_cmd_msg;

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

  thread_signal_rec = (THREAD_SIGNAL_PTR) priv_connect_port->app_data;

  connected = FALSE;
  do {

/***
/* Monitor network mailbox; we will get connect reqeust messages, link
/* disconnect messages, etc. Ignore all but connect messages
*/
    stat = sys$qio( 0, priv_connect_port->net_obj_mbx_chan, IO$_READVBLK,
     &iosb, ast_routine, &thread_signal_rec->cv, &net_cmd_msg,
     sizeof(net_cmd_msg), 0, 0, 0, 0 );

    if ( ! ( stat & STS$M_SUCCESS ) ) {
      ret_stat = stat;
      goto err_ret;
    }

    do {

      cma_mutex_lock( &thread_signal_rec->mu );
      cma_cond_wait( &thread_signal_rec->cv, &thread_signal_rec->mu );
      cma_mutex_unlock( &thread_signal_rec->mu );

      stat = iosb.status;

    } while ( stat == 0 );

    if ( ! ( stat & STS$M_SUCCESS ) ) {
      ret_stat = stat;
      goto err_ret;
    }

    switch ( net_cmd_msg.msg_type ) {

      case MSG$_CONNECT:
        stat = establish_link( net_cmd_msg.name_info, priv_connect_port,
         priv_data_port );
        if ( ! ( stat & STS$M_SUCCESS ) ) {
          ret_stat = stat;
          goto err_ret;
        }
        connected = TRUE;
        get_optional_data( priv_data_port, optional_data );
        break;

      case MSG$_NETSHUT:
        strcpy( logbuf,
         "[Network shutdown message received]" );
        post_log_msg( logbuf );
        ret_stat = NSV_SHUTDOWN;
        goto err_ret;
        break;

      case MSG$_DISCON:
        break;

      case MSG$_ABORT:
      case MSG$_EXIT:
      case MSG$_PATHLOST:
      case MSG$_PROTOCOL:
      case MSG$_THIRDPARTY:
      case MSG$_TIMEOUT:
        break;

      default:
        sprintf( logbuf, "[Network event on unit %-d, msg type: %-d]",
         net_cmd_msg.unit, net_cmd_msg.msg_type );
        post_log_msg( logbuf );
        break;

    }

  } while ( !connected );

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

IO_STAT_BLK_T iosb;
int stat, ret_stat;
unsigned int break_connection;
char logbuf[128+1];
PORT_MSG_P cur_msg;
PRIV_PORT_P priv_port;
THREAD_SIGNAL_PTR thread_signal_rec;

  if ( !port ) {
    ret_stat = NSV_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_PORT_P) port;

  if ( priv_port->msg_count == priv_port->max_msgs ) {
    ret_stat = NSV_PORTFULL;
    goto err_ret;
  }

  cur_msg = (PORT_MSG_P) calloc( 1, sizeof(PORT_MSG_T) );
  if ( !cur_msg ) {
    ret_stat = NSV_NOMEM;
    goto err_ret;
  }

  cur_msg->msg = (char *) calloc( 1, priv_port->max_msg_size+1 );
  if ( !cur_msg->msg ) {
    ret_stat = NSV_NOMEM;
    goto err_ret;
  }

  cur_msg->msg_len = 0;

  thread_signal_rec = (THREAD_SIGNAL_PTR) priv_port->app_data;

  stat = sys$qio( 0, priv_port->net_dev_chan, IO$_READVBLK,
   &iosb, ast_routine, &thread_signal_rec->cv, cur_msg->msg,
   priv_port->max_msg_size, 0, 0, 0, 0 );

  if ( ! ( stat & STS$M_SUCCESS ) ) {
    free( cur_msg->msg );
    free( cur_msg );
    ret_stat = stat;
    goto err_ret;
  }

  do {

    cma_mutex_lock( &thread_signal_rec->mu );
    cma_cond_wait( &thread_signal_rec->cv, &thread_signal_rec->mu );
    cma_mutex_unlock( &thread_signal_rec->mu );

    stat = iosb.status;
    break_connection = thread_signal_rec->break_connection;

  } while ( ( stat == 0 ) && ( break_connection == 0 ) );

  if ( break_connection ) {
    sys$cancel( priv_port->net_dev_chan );
    free( cur_msg->msg );
    free( cur_msg );
    ret_stat = NSV_ABORT;
    goto err_ret;
  }

  stat = iosb.status;
  if ( ! ( stat & STS$M_SUCCESS ) ) {
    free( cur_msg->msg );
    free( cur_msg );
    ret_stat = stat;
    goto err_ret;
  }

  priv_port->tail->next = cur_msg;
  priv_port->tail = cur_msg;
  priv_port->tail->next = NULL;

  (priv_port->msg_count)++;

  cur_msg->msg_len = iosb.msg_len;

  *result = 1;

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
char logbuf[128+1];
PORT_MSG_P cur_msg;
PRIV_PORT_P priv_port;

  if ( !port ) {
    ret_stat = NSV_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_PORT_P) port;

  if ( priv_port->msg_count == 0 ) {
    ret_stat = NSV_PORTEMPTY;
    goto err_ret;
  }

  /* unlink */
  cur_msg = priv_port->head->next;
  if ( !cur_msg ) {
    ret_stat = NSV_PORTEMPTY;
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
  free( cur_msg->msg );
  free( cur_msg );

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

IO_STAT_BLK_T iosb;
int stat, ret_stat;
char logbuf[128+1];
PRIV_PORT_P priv_port;

  if ( !port ) {
    ret_stat = NSV_BADPARAM;
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
  return NSV_SUCCESS;

err_ret:
  return ret_stat;

}

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

int nsv_abort_connection (
  RPC_PORT port
) {

int stat, ret_stat;
PRIV_PORT_P priv_port;
THREAD_SIGNAL_PTR thread_signal_rec;

  if ( !port ) {
    ret_stat = NSV_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_PORT_P) port;

  thread_signal_rec = (THREAD_SIGNAL_PTR) priv_port->app_data;

  thread_signal_rec->break_connection = TRUE;
  cma_cond_signal( &thread_signal_rec->cv );

norm_ret:
  return NSV_SUCCESS;

err_ret:
  return ret_stat;

}
