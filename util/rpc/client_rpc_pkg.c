#include stdio
#include descrip

#include sys_types
#include port
#include nis
#include client_rpc_priv


int client_rpc_max_buf_size ( void ) {

  return RPC_MAX_MSG_SIZE;

}

int client_rpc_create_buffer (
  RPC_BUF *buf,
  RPC_PORT port
) {

PRIV_RPC_BUF_PTR priv_buf;
int ret_stat;

  priv_buf = (PRIV_RPC_BUF_PTR) calloc( 1, sizeof(PRIV_RPC_BUF_TYPE) );
  if ( !priv_buf ) {
    ret_stat = RPC_NOMEM;
    goto err_return;
  }

  priv_buf->rpc_port = port;
  priv_buf->msg_buf.send_seq_num = 0;
  priv_buf->msg_buf.rcv_seq_num = 0;

  *buf = (RPC_BUF *) priv_buf;

norm_return:
  return RPC_SUCCESS;

err_return:
  return ret_stat;

}

int client_rpc_destroy_buffer (
  RPC_BUF buf
) {

int ret_stat;

  if ( buf ) {
    free( buf );
  }
  else {
    ret_stat = RPC_BADPARAM;
    goto err_return;
  }

norm_return:
  return RPC_SUCCESS;

err_return:
  return ret_stat;

}

int client_rpc_create_handle (
  RPC_HANDLE *handle,
  char *port_name
) {

PRIV_RPC_HANDLE_PTR priv_handle;
int stat, ret_stat;
float seconds = 5.0 * 60.0;	/* five minutes */
SYS_TIME_TYPE timeout;

  stat = sys_cvt_seconds_to_timeout( seconds, &timeout );
  if ( !( stat % 2 ) ) {
     ret_stat = stat;
     goto err_return;
  }

  priv_handle = (PRIV_RPC_HANDLE_PTR) calloc( 1, sizeof(PRIV_RPC_HANDLE_TYPE) );
  if ( !priv_handle ) {
    ret_stat = RPC_NOMEM;
    goto err_return;
  }

  priv_handle->connect_state = RPC_K_UNCONNECTED;
  priv_handle->connect_type = 0;
  priv_handle->timeout = timeout;

  stat = ncl_create_port( 1, RPC_MAX_MSG_SIZE, port_name,
   &priv_handle->data_port );
  if ( stat != NCL_SUCCESS ) {
     ret_stat = stat;
     goto err_return;
  }

  stat = client_rpc_create_buffer( &priv_handle->buf, priv_handle->data_port );
  if ( stat != RPC_SUCCESS ) {
     ret_stat = stat;
     goto err_return;
  }

  *handle = (RPC_HANDLE) priv_handle;

norm_return:
  return RPC_SUCCESS;

err_return:
  *handle = NULL;
  return ret_stat;

}

int client_rpc_destroy_handle (
  RPC_HANDLE *handle
) {

PRIV_RPC_HANDLE_PTR priv_handle;
int stat, ret_stat;

  priv_handle = (PRIV_RPC_HANDLE_PTR) *handle;

  if ( priv_handle->connect_state == RPC_K_CONNECTED ) {

    stat = ncl_disconnect( priv_handle->data_port );
    if ( stat != NCL_SUCCESS ) {
       ret_stat = stat;
       goto err_return;
    }

  }

  stat = ncl_delete_port( &priv_handle->data_port );
  if ( stat != NCL_SUCCESS ) {
     ret_stat = stat;
     goto err_return;
  }

  stat = client_rpc_destroy_buffer( priv_handle->buf );
  if ( stat != RPC_SUCCESS ) {
     ret_stat = stat;
     goto err_return;
  }

  free ( priv_handle );

  *handle = NULL;

norm_return:
  return RPC_SUCCESS;

err_return:
  return ret_stat;

}

int client_rpc_get_connect_type(
  RPC_HANDLE handle,
  int *mode
) {

PRIV_RPC_HANDLE_PTR priv_handle;
int stat, ret_stat;

  priv_handle = (PRIV_RPC_HANDLE_PTR) handle;
  if ( !priv_handle ) {
    ret_stat = RPC_BADPARAM;
    goto err_return;
  }

  *mode = priv_handle->connect_type;

norm_return:
  return RPC_SUCCESS;

err_return:
  return ret_stat;

}

int client_rpc_set_timeout (
  RPC_HANDLE handle,
  float seconds
) {

PRIV_RPC_HANDLE_PTR priv_handle;
int stat, ret_stat;
SYS_TIME_TYPE timeout;

  priv_handle = (PRIV_RPC_HANDLE_PTR) handle;
  if ( !priv_handle ) {
    ret_stat = RPC_BADPARAM;
    goto err_return;
  }

  stat = sys_cvt_seconds_to_timeout( seconds, &timeout );
  if ( !( stat % 2 ) ) {
     ret_stat = stat;
     goto err_return;
  }

  priv_handle->timeout = timeout;

norm_return:
  return RPC_SUCCESS;

err_return:
  return ret_stat;

}

int client_rpc_connect (
  RPC_HANDLE handle,
  char *generic_service_name,
  char *client_data
) {

int mode, stat, ret_stat;
PRIV_RPC_HANDLE_PTR priv_handle;
char node[31+1];
char service_name[NIS_MAX_SERVICE_NAME+1];
int num_connect_fails = 0;

  priv_handle = (PRIV_RPC_HANDLE_PTR) handle;

  do {

    stat = nis_get_service( num_connect_fails, generic_service_name,
     service_name, node, &mode );
    if ( stat != NIS_SUCCESS ) {
      ret_stat = stat;
      goto err_return;
    }

    stat = ncl_connect( node, service_name, client_data,
     priv_handle->data_port );
    if ( stat != NCL_SUCCESS ) num_connect_fails++;

  } while ( ( stat != NCL_SUCCESS ) && ( num_connect_fails < 2 ) );

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

int client_rpc_disconnect (
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

void client_rpc_buf_init (
  RPC_HANDLE handle,
  int func_id,
  int num_args
) {

PRIV_RPC_HANDLE_PTR priv_handle;
PRIV_RPC_BUF_PTR priv_buf;

  priv_handle = (PRIV_RPC_HANDLE_PTR) handle;
  priv_buf = (PRIV_RPC_BUF_PTR) priv_handle->buf;

  priv_buf->cur_index = 0;
  priv_buf->cur_ofs = num_args * sizeof(ARG_DESC_TYPE);
  priv_buf->msg_buf.func_id = func_id;
  priv_buf->msg_buf.num_args = num_args;

}

int client_rpc_buf_add_arg (
  RPC_HANDLE handle,
  int arg_type,
  int access_type,
  int buf_size,
  void *addr
) {

int align_size, new_ofs, adjustment, ret_stat;
ARG_DESC_TYPE *arg_desc;
char *data;
PRIV_RPC_HANDLE_PTR priv_handle;
PRIV_RPC_BUF_PTR priv_buf;

  priv_handle = (PRIV_RPC_HANDLE_PTR) handle;
  priv_buf = (PRIV_RPC_BUF_PTR) priv_handle->buf;

  if ( priv_buf->cur_index >= RPC_MAX_ARGS ) {
    ret_stat = RPC_ARGBUFOVFLO;
    goto err_return;
  }

  switch ( arg_type ) {
    case ARG_TYPE_INT:
      align_size = 4;
      break;
    case ARG_TYPE_SHORT:
      align_size = 2;
      break;
    case ARG_TYPE_QUAD:
      align_size = 8;
      break;
    case ARG_TYPE_CHAR:
      align_size = 1;
      break;
    case ARG_TYPE_FLOAT:
      align_size = 4;
      break;
    case ARG_TYPE_DOUBLE:
      align_size = 8;
      break;
    case ARG_TYPE_STRUCT:
      align_size = 8;
      break;
    default:
      ret_stat = RPC_INVTYPE;
      goto err_return;
  }

  arg_desc = (ARG_DESC_TYPE *) priv_buf->msg_buf.data;
  data = (char *) priv_buf->msg_buf.data;

  arg_desc[priv_buf->cur_index].type_code = arg_type;
  arg_desc[priv_buf->cur_index].access_code = access_type;
  arg_desc[priv_buf->cur_index].buf_size = buf_size;

/***
/* perform natural alignment of data item
*/
  new_ofs = priv_buf->cur_ofs;
  adjustment = align_size - ( new_ofs % align_size );
  if ( adjustment != align_size ) new_ofs += adjustment;

  if ( ( new_ofs + buf_size ) >= RPC_MAX_MSG_SIZE ) {
    ret_stat = RPC_DATAOVFLO;
    goto err_return;
  }

  arg_desc[priv_buf->cur_index].ofs = new_ofs;

  arg_desc[priv_buf->cur_index].client_addr = addr;

  if ( access_type != ARG_ACCESS_WRITE )
    memcpy( &data[new_ofs], (char *) addr, buf_size );

  priv_buf->cur_index++;
  priv_buf->cur_ofs = new_ofs + buf_size;

norm_return:
  return RPC_SUCCESS;

err_return:
  return ret_stat;

}

int client_rpc_remote_call (
  RPC_HANDLE handle
) {

int stat, ret_stat, result, len, ofs, buf_size, i;
unsigned int cur_seq_num;
PRIV_RPC_HANDLE_PTR priv_handle;
PRIV_RPC_BUF_PTR priv_buf;
ARG_DESC_TYPE *arg_desc;
char *data;

  priv_handle = (PRIV_RPC_HANDLE_PTR) handle;
  priv_buf = (PRIV_RPC_BUF_PTR) priv_handle->buf;

  priv_buf->msg_size = priv_buf->cur_ofs +
   sizeof(priv_buf->msg_buf.send_seq_num) +
   sizeof(priv_buf->msg_buf.rcv_seq_num) +
   sizeof(priv_buf->msg_buf.func_id) +
   sizeof(priv_buf->msg_buf.num_args);

  if ( priv_buf->msg_buf.send_seq_num == 0xffffffff )
    priv_buf->msg_buf.send_seq_num = 0;
  priv_buf->msg_buf.send_seq_num++;
  cur_seq_num = priv_buf->msg_buf.send_seq_num;
  priv_buf->msg_buf.rcv_seq_num = 0;

  stat = ncl_send_msg( priv_buf->rpc_port, priv_buf->msg_size,
   &priv_buf->msg_buf );
  if ( stat != NCL_SUCCESS ) {
    ret_stat = stat;
    goto err_return;
  }

  stat = ncl_wait_on_port( priv_buf->rpc_port, &priv_handle->timeout, &result );
  if ( stat != NCL_SUCCESS ) {
    ret_stat = stat;
    goto err_return;
  }

  stat = ncl_receive_msg( priv_buf->rpc_port, sizeof(MSG_BUF_TYPE),
   &len, &priv_buf->msg_buf );
  if ( stat != NCL_SUCCESS ) {
    ret_stat = stat;
    goto err_return;
  }

  if ( cur_seq_num != priv_buf->msg_buf.rcv_seq_num ) {
    ret_stat = RPC_BADSEQ;
    goto err_return;
  }

  arg_desc = (ARG_DESC_TYPE *) priv_buf->msg_buf.data;
  data = (char *) priv_buf->msg_buf.data;

  for ( i=0; i<priv_buf->msg_buf.num_args; i++ ) {
    if ( arg_desc[i].access_code != ARG_ACCESS_READ ) {
      ofs = arg_desc[i].ofs;
      buf_size = arg_desc[i].buf_size;
      memcpy( (char *) arg_desc[i].client_addr, &data[ofs], buf_size );
    }
  }

norm_return:
  return RPC_SUCCESS;

err_return:
  return ret_stat;

}
