/***
* includes
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "sys_types.h"
#include "ipncl_priv.h"

#define IPNSV_FD_TABLE_SIZE getdtablesize()

int ipncl_wait_on_port (
  IPRPC_PORT port,
  SYS_TIME_TYPE *sys_timeout,
  int *result
);

int ipncl_receive_msg (
  IPRPC_PORT port,
  int buf_len,
  int *msg_len,
  char *msg
);

/***
* package globals
*/

static int convert_host_string_to_addr(
  char *string,
  int *addr
) {

/* for now, string must be ip addr */
  *addr = inet_addr( string );

  return IPNCL_SUCCESS;

}

static int convert_port_string_to_port_num (
  char *service,
  unsigned short *port_num
) {

int p;

  p = atol( service );
  *port_num = htons( (unsigned short) p );

  return IPNCL_SUCCESS;

}

int ipncl_network_link_fail (
  int stat
) {

  if ( stat == IPNCL_CON_CLOSED ||
       stat == IPNCL_TIMEOUT ||
       stat == IPNCL_CON_RESET ) {

    return 1;

  }
  else {

    return 0;

  }

}

int ipncl_create_port (
  int max_msgs,
  int max_msg_size,
  char *label,
  IPRPC_PORT *port
) {

/***
* allocate and initialize port object
*/

PRIV_IPPORT_P priv_port;
int stat, ret_stat, value, len;

int port_allocated = 0;
int label_allocated = 0;

  priv_port = (PRIV_IPPORT_P) calloc( 1, sizeof(PRIV_IPPORT_T) );
  if ( !priv_port ) {
    ret_stat = IPNCL_NOMEM;
    goto err_ret;
  }
  port_allocated = 1;

  priv_port->port_type = IPPORT_K_UNNAMED;
  priv_port->conn_type = IPPORT_K_CLIENT;

  priv_port->net_obj_name = NULL;

  priv_port->client_data = NULL;

  priv_port->port_label = NULL;
  if ( strlen(label) > 0 ) {
    priv_port->port_label = (char *) calloc( 1, strlen(label)+1 );
    if ( !priv_port->port_label ) {
      ret_stat = IPNCL_NOMEM;
      goto err_ret;
    }
    label_allocated = 1;
    strcpy( priv_port->port_label, label );
  }
  else {
    priv_port->port_label = NULL;
  }

  /* create socket */
  priv_port->sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
  if ( priv_port->sockfd == -1 ) {
    ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  value = 1;
  len = sizeof(value);
  stat = setsockopt( priv_port->sockfd, IPPROTO_TCP, TCP_NODELAY,
   (char *) &value, len );

  value = 1;
  len = sizeof(value);
  stat = setsockopt( priv_port->sockfd, SOL_SOCKET, SO_KEEPALIVE,
   (char *) &value, len );

  /* do socket init */
  bzero( (char *) &priv_port->sin, sizeof(priv_port->sin) );
  priv_port->sin.sin_family = AF_INET;

  *port = (IPRPC_PORT) priv_port;

  return IPNCL_SUCCESS;

err_ret:

  if ( label_allocated ) {
    free( priv_port->port_label );
  }
  if ( port_allocated ) {
    free( priv_port );
  }

  *port = (IPRPC_PORT) 0;

  return ret_stat;

}

int ipncl_disconnect (
  IPRPC_PORT port
) {

int stat, ret_stat;
PRIV_IPPORT_P priv_port;

/***
* check for NULL value
*/
  if ( !port ) {
    ret_stat = IPNCL_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_IPPORT_P) port;

/***
* make sure this is a data port and not a connect port
*/
  if ( priv_port->port_type == IPPORT_K_NAMED ) {
    ret_stat = IPNCL_NOTCON;
    goto err_ret;
  }

  ret_stat = IPNCL_SUCCESS;

/***
* disconnect asychronously
*/
  stat = shutdown( priv_port->sockfd, 2 );
  if ( stat ) {
    ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  stat = shutdown( priv_port->event_sockfd, 2 );
  if ( stat ) {
    ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  return IPNCL_SUCCESS;

err_ret:
  return ret_stat;

}

int ipncl_delete_port (
  IPRPC_PORT *port
) {

int stat, ret_stat;
PRIV_IPPORT_P priv_port;

/***
* check for NULL value
*/
  if ( !port ) {
    ret_stat = IPNCL_BADPARAM;
    goto err_ret;
  }

  ret_stat = IPNCL_SUCCESS;

  priv_port = (PRIV_IPPORT_P) *port;

  stat = close( priv_port->sockfd );

  stat = close( priv_port->event_sockfd );

/***
* free port data
*/
  if ( priv_port->net_obj_name ) {
    free( priv_port->net_obj_name );
  }
  if ( priv_port->client_data ) {
    free( priv_port->client_data );
  }
  if ( priv_port->port_label ) {
    free( priv_port->port_label );
  }

/***
* free port data structure
*/
  free( priv_port );

  *port = NULL;

  return IPNCL_SUCCESS;

err_ret:
  return ret_stat;

}

int ipncl_connect (
  char *host,		/* host name or ip address (asciz) */
  char *service,	/* service name or port num (asciz) */
  char *optional_data,	/* up to 14 bytes */
  IPRPC_PORT port	/* new port through which subsequent
			   messages are exchanged */
) {

int stat, ret_stat, len, ip_addr, result,
 connected, num_fails, value;
float one_sec;
struct sockaddr_in event_addr;
unsigned short port_num;
PRIV_IPPORT_P priv_port;

  if ( !port ) {
    ret_stat = IPNCL_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_IPPORT_P) port;

  stat = convert_host_string_to_addr( host, &ip_addr );
  if ( !( stat & 1 ) ) {
    ret_stat = IPNCL_UNKNOWN_HOST;
    goto err_ret;
  }

  stat = convert_port_string_to_port_num( service, &port_num );
  if ( !( stat & 1 ) ) {
    ret_stat = IPNCL_UNKNOWN_SERVICE;
    goto err_ret;
  }

  bzero( (char *) &priv_port->sin, sizeof(priv_port->sin) );
  priv_port->sin.sin_family = AF_INET;
  priv_port->sin.sin_addr.s_addr = ip_addr;
  priv_port->sin.sin_port = port_num;

  value = 1;
  len = sizeof(value);
  stat = setsockopt( priv_port->sockfd, SOL_SOCKET, SO_KEEPALIVE,
   (char *) &value, len );

  value = 1;
  len = sizeof(value);
  stat = setsockopt( priv_port->sockfd, IPPROTO_TCP, TCP_NODELAY,
   (char *) &value, len );

  stat = connect( priv_port->sockfd, (struct sockaddr *) &priv_port->sin, sizeof(priv_port->sin) );
  if ( stat < 0 ) {
    ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  /* get event port number */
  stat = ipncl_wait_on_port( port, NULL, &result );
  if ( !( stat & 1 ) ) {
    ret_stat = stat;
    goto err_ret;
  }

  stat = ipncl_receive_msg( port, sizeof(short), &len, (char*) &port_num );
  if ( !( stat & 1 ) ) {
    ret_stat = stat;
    goto err_ret;
  }

/*  port_num = ntohs(port_num); */

  priv_port->event_sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
  if ( priv_port->event_sockfd < 0 ) {
    ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  value = 1;
  len = sizeof(value);
  stat = setsockopt( priv_port->event_sockfd, IPPROTO_TCP, TCP_NODELAY,
   (char *) &value, len );

  bzero( (char *) &event_addr, sizeof(event_addr) );
  event_addr.sin_family = AF_INET;
  event_addr.sin_addr.s_addr = ip_addr;
/*  event_addr.sin_port = htons( port_num ); */
  event_addr.sin_port = port_num;

  connected = FALSE;
  num_fails = 0;
  do {

    stat = connect( priv_port->event_sockfd, (struct sockaddr *) &event_addr,
     sizeof(event_addr) );
    if ( stat < 0 ) {
      num_fails++;
      one_sec = 1.0;
      sys_wait_seconds( &one_sec );
    }
    else {
      connected = TRUE;
    }

  } while ( !connected && ( num_fails < 60 ) );

  if ( !connected ) {
    ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  if ( optional_data ) {

    /* ? */

  }

  return IPNCL_SUCCESS;

err_ret:
  return ret_stat;

}

int ipncl_wait_on_port (
  IPRPC_PORT port,
  SYS_TIME_PTR sys_timeout,
  int *result
) {

int ret_stat;
PRIV_IPPORT_P priv_port;
struct fd_set fds;
int fd;

  if ( !port ) {
    ret_stat = IPNCL_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_IPPORT_P) port;

  FD_ZERO( &fds );
  FD_SET( priv_port->sockfd, &fds );

/*
** convert sys_timeout in SYS_TIME_TYPE units to seconds and then to
** type struct timeval
*/
  if ( sys_timeout ) {

/*
** wait for input availability on socket with timeout
*/
    fd = select( IPNSV_FD_TABLE_SIZE, &fds, (struct fd_set *) NULL,
     (struct fd_set *) NULL, &sys_timeout->timeval_time );

  }
  else {

/*
** wait for input availability on socket without timeout
*/
    fd = select( IPNSV_FD_TABLE_SIZE, &fds, (struct fd_set *) NULL,
     (struct fd_set *) NULL, (struct timeval *) NULL );

  }

  /* if fd is zero then must be a timeout */
  if ( !fd ) {
    *result = 0;				/* timeout */
    goto norm_ret;
  }
  else if ( fd == -1 ) {
    *result = 0;
    if ( errno == ECONNRESET )
      ret_stat = IPNCL_CON_RESET;
    else
      ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  *result = 1;			/* input available, no timeout */

norm_ret:
  return IPNCL_SUCCESS;

err_ret:
  return ret_stat;

}

int ipncl_receive_msg (
  IPRPC_PORT port,
  int buf_len,
  int *msg_len,
  char *msg
) {

int ret_stat;
PRIV_IPPORT_P priv_port;

  if ( !port ) {
    ret_stat = IPNCL_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_IPPORT_P) port;

  *msg_len = read( priv_port->sockfd, msg, (unsigned short) buf_len );

  if ( *msg_len == 0 ) {
    ret_stat = IPNCL_CON_CLOSED;
    goto err_ret;
  }
  else if ( *msg_len < 0 ) {
    if ( errno == ECONNRESET )
      ret_stat = IPNCL_CON_RESET;
    else
      ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  return IPNCL_SUCCESS;

err_ret:
  return ret_stat;

}

int ipncl_send_msg (
  IPRPC_PORT port,
  int msg_len,
  char *msg
) {

int n, ret_stat;
PRIV_IPPORT_P priv_port;

  if ( !port ) {
    ret_stat = IPNCL_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_IPPORT_P) port;

  n = write( priv_port->sockfd, msg, (unsigned short) msg_len );

  if ( n < 0 ) {
    if ( errno == ECONNRESET )
      ret_stat = IPNCL_CON_RESET;
    else
      ret_stat = UNIX_ERROR;
    goto err_ret;
  }
  else if ( n < msg_len ) {
    ret_stat = IPNCL_MSG_TRUNC;
    goto err_ret;
  }

  return IPNCL_SUCCESS;

err_ret:
  return ret_stat;

}

int ipncl_wait_on_event (
  IPRPC_PORT port,
  SYS_TIME_PTR sys_timeout,
  int *result
) {

int ret_stat;
PRIV_IPPORT_P priv_port;
struct fd_set fds;
int fd;

  if ( !port ) {
    ret_stat = IPNCL_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_IPPORT_P) port;

  FD_ZERO( &fds );
  FD_SET( priv_port->event_sockfd, &fds );

/*
** convert sys_timeout in SYS_TIME_TYPE units to seconds and then to
** type struct timeval
*/
  if ( sys_timeout ) {

/*
** wait for input availability on socket with timeout
*/
    fd = select( IPNSV_FD_TABLE_SIZE, &fds, (struct fd_set *) NULL,
     (struct fd_set *) NULL, &sys_timeout->timeval_time );

  }
  else {

/*
** wait for input availability on socket without timeout
*/
    fd = select( IPNSV_FD_TABLE_SIZE, &fds, (struct fd_set *) NULL,
     (struct fd_set *) NULL, (struct timeval *) NULL );

  }

  /* if fd is zero then must be a timeout */
  if ( !fd ) {
    *result = 0;				/* timeout */
    goto norm_ret;
  }
  else if ( fd == -1 ) {
    *result = 0;
    if ( errno == ECONNRESET )
      ret_stat = IPNCL_CON_RESET;
    else
      ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  *result = 1;			/* input available, no timeout */

norm_ret:
  return IPNCL_SUCCESS;

err_ret:
  return ret_stat;

}

int ipncl_receive_event_msg (
  IPRPC_PORT port,
  int buf_len,
  int *msg_len,
  char *msg
) {

int ret_stat;
PRIV_IPPORT_P priv_port;

  if ( !port ) {
    ret_stat = IPNCL_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_IPPORT_P) port;

  *msg_len = read( priv_port->event_sockfd, msg, (unsigned short) buf_len );

  if ( *msg_len == 0 ) {
    ret_stat = IPNCL_CON_CLOSED;
    goto err_ret;
  }
  else if ( *msg_len < 0 ) {
    if ( errno == ECONNRESET )
      ret_stat = IPNCL_CON_RESET;
    else
      ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  return IPNCL_SUCCESS;

err_ret:
  return ret_stat;

}
