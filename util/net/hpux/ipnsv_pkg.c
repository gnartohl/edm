/***
* includes
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#ifdef __osf__
#include <time.h>
#include <sys/time.h>
#endif

#ifdef __vxworks__
#include <time.h>
#include <sys/times.h>
#endif

#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "ipnsv_priv.h"
#include "sys_types.h"

#ifdef __osf__
#define IPNSV_FD_TABLE_SIZE getdtablesize()
#endif

#ifdef HP_UX
#define IPNSV_FD_TABLE_SIZE getdtablesize()
#endif

#ifdef __vxworks__
#define IPNSV_FD_TABLE_SIZE FD_SETSIZE
#endif


static int convert_port_string_to_port_num (
  char *service,
  unsigned short *port_num
) {

int p;

  p = atol( service );
  *port_num = htons( (unsigned short) p );

  return IPNSV_SUCCESS;

}

#ifdef FUTURE_USE
static void post_log_msg (
  char *msg
) {

  printf( "%s\n", msg );

}
#endif

int ipnsv_create_named_port (
  char *service,
  int max_msgs,
  int max_msg_size,
  char *label,
  IPRPC_PORT *port
) {

/***
* create communication port and network service name
*/

PRIV_IPPORT_P priv_port;
int stat, ret_stat;
unsigned short port_num;
int value, len;

/***
* allocate and populate port object
*/
  priv_port = (PRIV_IPPORT_P) calloc( 1, sizeof(PRIV_IPPORT_T) );
  if ( !priv_port ) {
    ret_stat = IPNSV_NOMEM;
    goto err_ret;
  }

  priv_port->port_type = IPPORT_K_NAMED;
  priv_port->conn_type = IPPORT_K_SERVER;

  priv_port->net_obj_name = (char *) calloc( 1, strlen(service)+1 );
  if ( !priv_port->net_obj_name ) {
    ret_stat = IPNSV_NOMEM;
    goto err_ret;
  }
  strcpy( priv_port->net_obj_name, service );

  priv_port->client_data = NULL;

  priv_port->port_label = NULL;
  if ( strlen(label) > 0 ) {
    priv_port->port_label = (char *) calloc( 1, strlen(label)+1 );
    if ( !priv_port->port_label ) {
      ret_stat = IPNSV_NOMEM;
      goto err_ret;
    }
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
  stat = setsockopt( priv_port->sockfd, SOL_SOCKET, SO_REUSEADDR,
   &value, len );

  /* do socket init */
  stat = convert_port_string_to_port_num( service, &port_num );
  if ( !( stat & 1 ) ) {
    ret_stat = IPNSV_UNKNOWN_SERVICE;
    goto err_ret;
  }

  bzero( (char *) &priv_port->sin, sizeof(priv_port->sin) );
  priv_port->sin.sin_family = AF_INET;
  priv_port->sin.sin_addr.s_addr = htonl(INADDR_ANY);
  priv_port->sin.sin_port = port_num;

  /* do bind and listen */
  stat = bind( priv_port->sockfd, (struct sockaddr*) &priv_port->sin,
   sizeof(priv_port->sin) );
  if ( stat < 0 ) {
    ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  stat = listen( priv_port->sockfd, 5 );
  if ( stat < 0 ) {
    ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  /* retur object */
  *port = (IPRPC_PORT) priv_port;

  return IPNSV_SUCCESS;

err_ret:
  *port = (IPRPC_PORT) 0;
  return ret_stat;

}

int ipnsv_create_port (
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

  priv_port = (PRIV_IPPORT_P) calloc( 1, sizeof(PRIV_IPPORT_T) );
  if ( !priv_port ) {
    ret_stat = IPNSV_NOMEM;
    goto err_ret;
  }

  priv_port->port_type = IPPORT_K_UNNAMED;

  priv_port->net_obj_name = NULL;

  priv_port->client_data = NULL;

  priv_port->port_label = NULL;
  if ( strlen(label) > 0 ) {
    priv_port->port_label = (char *) calloc( 1, strlen(label)+1 );
    if ( !priv_port->port_label ) {
      ret_stat = IPNSV_NOMEM;
      goto err_ret;
    }
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
  stat = setsockopt( priv_port->sockfd, SOL_SOCKET, SO_KEEPALIVE,
   &value, len );

  value = 1;
  len = sizeof(value);
  stat = setsockopt( priv_port->sockfd, IPPROTO_TCP, TCP_NODELAY,
   &value, len );

  /* do socket init */
  bzero( (char *) &priv_port->sin, sizeof(priv_port->sin) );
  priv_port->sin.sin_family = AF_INET;

  *port = (IPRPC_PORT) priv_port;

  return IPNSV_SUCCESS;

err_ret:
  *port = (IPRPC_PORT) 0;
  return ret_stat;

}

int ipnsv_disconnect (
  IPRPC_PORT port
) {

int stat, ret_stat;
PRIV_IPPORT_P priv_port;

  if ( !port ) {
    ret_stat = IPNSV_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_IPPORT_P) port;

  if ( priv_port->port_type == IPPORT_K_NAMED ) {
    ret_stat = IPNSV_NOTCON;
    goto err_ret;
  }

  if ( priv_port->client_data ) {
    free( priv_port->client_data );
    priv_port->client_data = NULL;
  }

  ret_stat = IPNSV_SUCCESS;

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

  return IPNSV_SUCCESS;

err_ret:
  return ret_stat;

}

int ipnsv_delete_port (
  IPRPC_PORT *port
) {

int stat, ret_stat;
PRIV_IPPORT_P priv_port;

  priv_port = (PRIV_IPPORT_P) *port;

  if ( !priv_port ) {
    ret_stat = IPNSV_BADPARAM;
    goto err_ret;
  }

  stat = close( priv_port->sockfd );

  if ( priv_port->port_type != IPPORT_K_NAMED ) {
    stat = close( priv_port->event_sockfd );
  }

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

/***
* free port data structure
*/
  free( priv_port );

  *port = NULL;

  return IPNSV_SUCCESS;

err_ret:
  return ret_stat;

}

int ipnsv_accept_connection (
  IPRPC_PORT connect_port, /* port through which connect requests enter */
  IPRPC_PORT data_port,	 /* new port through which subsequent
			    messages are exchanged */
  char *optional_data	 /* up to 16 chars + zero */
) {

int stat, ret_stat, clilen, newsockfd, n, msg_len, event_len,
 con_event_sockfd, len;
PRIV_IPPORT_P priv_connect_port, priv_data_port;
struct sockaddr_in cli_addr, event_addr;
int value;
unsigned short port_num;

  if ( !connect_port ) {
    ret_stat = IPNSV_BADPARAM;
    goto err_ret;
  }

  if ( !data_port ) {
    ret_stat = IPNSV_BADPARAM;
    goto err_ret;
  }

  priv_connect_port = (PRIV_IPPORT_P) connect_port;
  priv_data_port = (PRIV_IPPORT_P) data_port;

  /* accept connection */
  newsockfd = accept( priv_connect_port->sockfd, (struct sockaddr *) &cli_addr,
   &clilen );

  if ( newsockfd < 0 ) {
    ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  priv_data_port->sockfd = newsockfd;

  value = 1;
  len = sizeof(value);
  stat = setsockopt( priv_data_port->sockfd, IPPROTO_TCP, TCP_NODELAY,
   &value, len );

  /* event connection socket */
  con_event_sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
  if ( con_event_sockfd < 0 ) {
    ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  event_len = sizeof(event_addr);
  bzero( (char *) &event_addr, event_len );
  stat = getsockname( newsockfd, (struct sockaddr *) &event_addr, &event_len );
  if ( stat < 0 ) {
    ret_stat = UNIX_ERROR;
    goto err_ret;
  }
  event_addr.sin_port = 0;

  stat = bind( con_event_sockfd, (struct sockaddr *) &event_addr, event_len );
  if ( stat < 0 ) {
    ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  stat = getsockname( con_event_sockfd, (struct sockaddr *) &event_addr,
   &event_len );
  if ( stat < 0 ) {
    ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  port_num = event_addr.sin_port;
/*  net_port_num = htons( port_num ); */
  msg_len = sizeof(unsigned short);
  n = write( priv_data_port->sockfd, (char *) &port_num,
  (unsigned short) msg_len );
  if ( n < 0 ) {
    ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  stat = listen( con_event_sockfd, 1 );
  if ( stat < 0 ) {
    ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  newsockfd = accept( con_event_sockfd, (struct sockaddr*) &event_addr,
   &event_len );

  if ( newsockfd < 0 ) {
    ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  priv_data_port->event_sockfd = newsockfd;

  value = 1;
  len = sizeof(value);
  stat = setsockopt( priv_data_port->event_sockfd, IPPROTO_TCP, TCP_NODELAY,
   &value, len );

  stat = close( con_event_sockfd );
  if ( stat < 0 ) {
    ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  return IPNSV_SUCCESS;

err_ret:
  return ret_stat;

}

int ipnsv_wait_on_port (
  IPRPC_PORT port,
  SYS_TIME_TYPE *sys_timeout,
  int *result
) {

int stat, ret_stat;
PRIV_IPPORT_P priv_port;
fd_set fds;
float seconds;
struct timeval timeout;
int fd;

  if ( !port ) {
    ret_stat = IPNSV_BADPARAM;
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

    stat = sys_cvt_timeout_to_seconds( sys_timeout, &seconds );

    timeout.tv_sec = (int) seconds;
    timeout.tv_usec = (int) ( seconds - (float) timeout.tv_sec ) * 1.0e6;

/*
** wait for input availability on socket with timeout
*/
    fd = select( IPNSV_FD_TABLE_SIZE, &fds, (fd_set *) NULL,
     (fd_set *) NULL, &timeout );

  }
  else {

/*
** wait for input availability on socket without timeout
*/
    fd = select( IPNSV_FD_TABLE_SIZE, &fds, (fd_set *) NULL,
     (fd_set *) NULL, (struct timeval *) NULL );

  }

  /* if fd is zero then must be a timeout */
  if ( !fd ) {
    *result = 0;                                /* timeout */
    goto norm_ret;
  }
  else if ( fd == -1 ) {
    *result = 0;
    if ( errno == ECONNRESET )
      ret_stat = IPNSV_CON_RESET;
    else
      ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  *result = 1;                  /* input available, no timeout */

norm_ret:
  return IPNSV_SUCCESS;

err_ret:
  return ret_stat;

}

int ipnsv_receive_msg (
  IPRPC_PORT port,
  int buf_len,
  int *msg_len,
  char *msg
) {

int ret_stat;
PRIV_IPPORT_P priv_port;

  if ( !port ) {
    ret_stat = IPNSV_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_IPPORT_P) port;

  *msg_len = read( priv_port->sockfd, msg, (unsigned short) buf_len );

  if ( *msg_len == 0 ) {
    ret_stat = IPNSV_CON_CLOSED;
    goto err_ret;
  }
  else if ( *msg_len < 0 ) {
    if ( errno == ECONNRESET )
      ret_stat = IPNSV_CON_RESET;
    else
      ret_stat = UNIX_ERROR;
    goto err_ret;
  }

  return IPNSV_SUCCESS;

err_ret:
  return ret_stat;

}

int ipnsv_send_msg (
  IPRPC_PORT port,
  int msg_len,
  char *msg
) {

int n, ret_stat;
PRIV_IPPORT_P priv_port;

  if ( !port ) {
    ret_stat = IPNSV_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_IPPORT_P) port;

  n = write( priv_port->sockfd, msg, (unsigned short) msg_len );

  if ( n < 0 ) {
    if ( errno == ECONNRESET )
      ret_stat = IPNSV_CON_RESET;
    else
      ret_stat = UNIX_ERROR;
    goto err_ret;
  }
  else if ( n < msg_len ) {
    ret_stat = IPNSV_MSG_TRUNC;
    goto err_ret;
  }

  return IPNSV_SUCCESS;

err_ret:
  return ret_stat;

}

int ipnsv_send_event_msg (
  IPRPC_PORT port,
  int msg_len,
  char *msg
) {

int n, ret_stat;
PRIV_IPPORT_P priv_port;

  if ( !port ) {
    ret_stat = IPNSV_BADPARAM;
    goto err_ret;
  }

  priv_port = (PRIV_IPPORT_P) port;

  n = write( priv_port->event_sockfd, msg, (unsigned short) msg_len );

  if ( n < 0 ) {
    if ( errno == ECONNRESET )
      ret_stat = IPNSV_CON_RESET;
    else
      ret_stat = UNIX_ERROR;
    goto err_ret;
  }
  else if ( n < msg_len ) {
    ret_stat = IPNSV_MSG_TRUNC;
    goto err_ret;
  }

  return IPNSV_SUCCESS;

err_ret:
  return ret_stat;

}
