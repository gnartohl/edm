#include "pvs.hpp"

#ifdef __linux__

pvsClass::pvsClass ( void ) {

  this->host = new char[1];
  strcpy( this->host, "" );
  this->port = new char[1];
  strcpy( this->port, "" );

  this->needInit = 1;
  this->numPvs = 0;
  this->bufSize = 0;
  this->curGroup = 0;
  this->buf = NULL;
  this->tk = NULL;
  this->ctx = NULL;
  this->buf2 = NULL;
  this->tk2 = NULL;
  this->ctx2 = NULL;

}

pvsClass::pvsClass (
  char *hostWithPort
) {

char *tk, *ctx, *hbuf=NULL;

  if ( !hostWithPort ) {

    this->host = new char[1];
    strcpy( this->host, "" );
    this->port = new char[1];
    strcpy( this->port, "" );

  }
  else {

    hbuf = new char[strlen(hostWithPort)+1];
    strcpy( hbuf, hostWithPort );

    ctx = NULL;
    tk = strtok_r( hbuf, ":", &ctx );
    if ( tk ) {

      this->host = new char[strlen(tk)+1];
      strcpy( this->host, tk );

      tk = strtok_r( NULL, ":", &ctx );
      if ( tk ) {
        this->port = new char[strlen(tk)+1];
        strcpy( this->port, tk );
      }
      else {
        this->port = new char[1];
        strcpy( this->port, "" );
      }

    }
    else {

      this->host = new char[1];
      strcpy( this->host, "" );
      this->port = new char[1];
      strcpy( this->port, "" );

    }

  }

  if ( hbuf ) delete[] hbuf;

  this->needInit = 1;
  this->numPvs = 0;
  this->bufSize = 0;
  this->curGroup = 0;
  this->buf = NULL;
  this->tk = NULL;
  this->ctx = NULL;
  this->buf2 = NULL;
  this->tk2 = NULL;
  this->ctx2 = NULL;

}

pvsClass::~pvsClass ( void ) {

  if ( this->buf ) {
    free( this->buf );
    this->buf = NULL;
  }

  if ( this->buf2 ) {
    free( this->buf2 );
    this->buf2 = NULL;
  }

  if ( this->host ) {
    free( this->host );
    this->host = NULL;
  }

  if ( this->port ) {
    free( this->port );
    this->port = NULL;
  }

}

int pvsClass::getNumPvs (
  int *n
) {

int stat;

  if ( this->needInit ) {
    stat = init();
    if ( !( stat & 1 ) ) return stat;
  }

  *n = this->numPvs;

  return PVS_SUCCESS;

}

int pvsClass::getFirstPvsName (
  char **name
) {

int stat;

  if ( this->needInit ) {
    stat = init();
    if ( !( stat & 1 ) ) return stat;
  }

  this->curGroup = 0;

  stat = readGroup();
  if ( !( stat & 1 ) ) return stat;

  if ( !this->tk ) {
    return PVS_NOMORE;
  }

  *name = this->tk;

  this->tk = strtok_r( NULL, " ,\n", &this->ctx );
  this->tk = strtok_r( NULL, " ,\n", &this->ctx );
  this->tk2 = strtok_r( NULL, " ,\n", &this->ctx2 );
  this->tk2 = strtok_r( NULL, " ,\n", &this->ctx2 );

  return PVS_SUCCESS;

}

int pvsClass::getNextPvsName (
  char **name
) {

int stat;

  if ( this->needInit ) {
    stat = init();
    if ( !( stat & 1 ) ) return stat;
  }

  if ( !this->tk ) {
    this->curGroup++;
    stat = readGroup();
    if ( !( stat & 1 ) ) {
      this->curGroup--;
      return PVS_NOMORE;
    }
    if ( !this->tk ) {
      this->curGroup--;
      return PVS_NOMORE;
    }
  }

  *name = this->tk;

  this->tk = strtok_r( NULL, " ,\n", &this->ctx );
  this->tk = strtok_r( NULL, " ,\n", &this->ctx );
  this->tk2 = strtok_r( NULL, " ,\n", &this->ctx2 );
  this->tk2 = strtok_r( NULL, " ,\n", &this->ctx2 );

  return PVS_SUCCESS;

}

int pvsClass::sendCmd (
  int socketFd,
  char *msg
) {

struct timeval timeout;
int more, fd, i, remain, len;
fd_set fds;

  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  more = 1;
  i = 0;
  remain = strlen(msg);
  while ( more ) {

    FD_ZERO( &fds );
    FD_SET( socketFd, &fds );

    fd = select( getdtablesize(), (fd_set *) NULL, &fds,
     (fd_set *) NULL, &timeout );

    if ( fd == 0 ) { /* timeout */
      /* fprintf( stderr, "timeout\n" ); */
      return 0;
    }

    if ( fd < 0 ) { /* error */
      //perror( "select" );
      return 0;
    }

    len = write( socketFd, &msg[i], remain );
    if ( len < 1 ) return len;

    remain -= len;
    i += len;

    if ( remain < 1 ) more = 0;

  } while ( more );

  return i;

}

int pvsClass::getReply (
  int socketFd,
  char *msg,
  int maxLen
) {

struct timeval timeout;
int more, fd, i, ii, remain, len;
fd_set fds;

  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  more = 1;
  i = 0;
  remain = maxLen;
  while ( more ) {

    FD_ZERO( &fds );
    FD_SET( socketFd, &fds );

    fd = select( getdtablesize(), &fds, (fd_set *) NULL,
     (fd_set *) NULL, &timeout );

    if ( fd == 0 ) { /* timeout */
      return 0;
    }
    if ( fd < 0 ) { /* error */
      //perror( "select" );
      return 0;
    }

    strcpy( msg, "" );

    len = read( socketFd, &msg[i], remain );
    if ( len < 1 ) return len;
    msg[len] = 0;

    for ( ii=0; ii<len; ii++ ) {
      if ( msg[i+ii] == '\n' ) {
        msg[i+ii] = 0;
        len = strlen(msg);
	i += len;
        more = 0;
	break;
      }
    }

    if ( more ) {

      remain -= len;
      i += len;

      if ( remain <= 0 ) return 0;

    }

  } while ( more );

  return i;

}

int pvsClass::cmd (
  char *ipAddrArg,
  char *portArg,
  char *cmd,
  char *reply,
  int maxReplySize
) {

struct sockaddr_in s;
int stat, ip_addr, sockfd;
unsigned short port_num;
int value, len, nIn, nOut;
struct hostent *hostEntry;

  strcpy( reply, "" );

#if 0
  fprintf( stderr, "ipAddrArg = [%s]\n", ipAddrArg );
  fprintf( stderr, "portArg = [%s]\n", portArg );
  fprintf( stderr, "cmd = [%s]\n", cmd );
  return 1;
#endif

  hostEntry = gethostbyname( ipAddrArg );
  if ( !hostEntry ) return 2; // error
  ip_addr = *( (int *) hostEntry->h_addr_list[0] );

  //ip_addr = inet_addr( ipAddrArg );

  sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
  if ( sockfd == -1 ) {
    //perror( "" );
    return 2; // error
  }

  value = 1;
  len = sizeof(value);
  stat = setsockopt( sockfd, IPPROTO_TCP, TCP_NODELAY,
   &value, len );

  value = 1;
  len = sizeof(value);
  stat = setsockopt( sockfd, SOL_SOCKET, SO_KEEPALIVE,
   &value, len );

  port_num = (unsigned short) atol( portArg );

  port_num = htons( port_num );

  memset( (char *) &s, 0, sizeof(s) );
  s.sin_family = AF_INET;
  s.sin_addr.s_addr = ip_addr;
  s.sin_port = port_num;

  stat = connect( sockfd, (struct sockaddr *) &s, sizeof(s) );
  if ( stat ) {
    //perror( "connect" );
    close( sockfd );
    goto abortClose;
  }

  // fprintf( stderr, "connected\n" );

  // primary connection complete

  nOut = sendCmd( sockfd, cmd );
  if ( !nOut ) {
    goto abort;
  }

  nIn = getReply( sockfd, reply, maxReplySize );
  if ( !nIn ) {
    goto abort;
  }

  // fprintf( stderr, "nIn = %-d, reply = [%s]\n", nIn, reply );

  stat = shutdown( sockfd, 2 );
  stat = close( sockfd );

  return 1;

abort:

  stat = shutdown( sockfd, 2 );

abortClose:

  stat = close( sockfd );

  return 2; // error

}

int pvsClass::init ( void ) {

int stat, i, error;
char msg[31+1], *tk, *ctx;
int n;

 if ( !this->needInit ) return PVS_SUCCESS;

  // check server version

  stat = cmd( this->host, this->port, "version\n", msg, 31 );
  if ( !( stat & 1 ) ) return PVS_SERVER_FAIL;

  n = -1;
  error = 1;  
  ctx = NULL;
  tk = strtok_r( msg, " \n", &ctx );
  if ( tk ) {
    if ( strcmp( tk, "ok" ) == 0 ) {
      error = 0;
    }
  }
  tk = strtok_r( NULL, " \n", &ctx );
  if ( tk ) {
    if ( strcmp( tk, "R1-2" ) == 0 ) {
      error = 0;
    }
  }

  if ( error ) {
    return PVS_INCOMPATIBLE_VERION;
  }

  // get my buffer size -----------------------------------------------

  stat = cmd( this->host, this->port, "bufsize\n", msg, 31 );
  if ( !( stat & 1 ) ) return PVS_SERVER_FAIL;

  n = -1;
  error = 1;  
  ctx = NULL;
  tk = strtok_r( msg, " \n", &ctx );
  for ( i=0; i<2; i++ ) {

    if ( tk ) {

      switch ( i ) {

      case 0:
        if ( strcmp( tk, "ok" ) == 0 ) {
          error = 0;
	}
	break;

      case 1:
        n = atol( tk );
	break;

      }

    }
    else {

      return PVS_SERVER_FAIL;

    }

    tk = strtok_r( NULL, " \n", &ctx );

  }

  if ( error ) {
    return PVS_SERVER_FAIL;
  }

  if ( n == -1 ) {
    return PVS_SERVER_FAIL;
  }

  this->bufSize = n;
  this->buf = (char *) calloc( sizeof(char), n+1 );
  this->buf2 = (char *) calloc( sizeof(char), n+1 );

  // ------------------------------------------------------------------

  // get num pvs ------------------------------------------------------

  stat = cmd( this->host, this->port, "numpvs\n", msg, 31 );
  if ( !( stat & 1 ) ) return PVS_SERVER_FAIL;

  n = -1;
  error = 1;  
  ctx = NULL;
  tk = strtok_r( msg, " \n", &ctx );
  for ( i=0; i<2; i++ ) {

    if ( tk ) {

      switch ( i ) {

      case 0:
        if ( strcmp( tk, "ok" ) == 0 ) {
          error = 0;
	}
	break;

      case 1:
        n = atol( tk );
	break;

      }

    }
    else {

      return PVS_SERVER_FAIL;

    }

    tk = strtok_r( NULL, " \n", &ctx );

  }

  if ( error ) {
    return PVS_SERVER_FAIL;
  }

  if ( n == -1 ) {
    return PVS_SERVER_FAIL;
  }
  else {
    this->numPvs = n;
  }

  // ------------------------------------------------------------------

  //fprintf( stderr, "numPvs = %-d\n", this->numPvs );
  //fprintf( stderr, "bufSize = %-d\n", this->bufSize );

  this->needInit = 0;

  return PVS_SUCCESS;

}

int pvsClass::readGroup ( void ) {

int stat, i, error;
char msg[31+1];
int n;

  this->curNumNames = 0;
  this->curNameIndex = 0;

  if ( this->needInit ) {
    stat = init();
    if ( !( stat & 1 ) ) return stat;
  }

  snprintf( msg, 31, "getpvs %-d\n", this->curGroup );

  stat = cmd( this->host, this->port, msg, this->buf, this->bufSize );
  if ( !( stat & 1 ) ) return PVS_SERVER_FAIL;

  strcpy( this->buf2, this->buf );

  // get status and number of pvs in this message --------------------------
  // and leave buf and ctx in a state ready to read
  // pv names

  n = -1;
  error = 1;  
  this->ctx = NULL;
  this->tk = strtok_r( this->buf, " ,\n", &this->ctx );
  this->ctx2 = NULL;
  this->tk2 = strtok_r( this->buf2, " ,\n", &this->ctx2 );
  for ( i=0; i<2; i++ ) {

    if ( this->tk ) {

      switch ( i ) {

      case 0:
        if ( strcmp( this->tk, "ok" ) == 0 ) {
          error = 0;
	}
	break;

      case 1:
        n = atol( this->tk );
	break;

      }

    }
    else {

      return PVS_SERVER_FAIL;

    }

    this->tk = strtok_r( NULL, " ,\n", &this->ctx );
    this->tk2 = strtok_r( NULL, " ,\n", &this->ctx2 );

  }

  this->tk2 = strtok_r( NULL, " ,\n", &this->ctx2 );

  if ( error ) {
    return PVS_SERVER_FAIL;
  }

  if ( n == -1 ) {
    return PVS_SERVER_FAIL;
  }

  this->curNumNames = n;

  return PVS_SUCCESS;

}

#else

pvsClass::pvsClass ( void ) {
}

pvsClass::pvsClass (
  char *hostWithPort
) {
}

pvsClass::~pvsClass ( void ) {
}

int pvsClass::getNumPvs (
  int *n
) {
  *n = 0;
  return PVS_FAILURE;
}

int pvsClass::getFirstPvsName (
  char **name
) {
  return PVS_FAILURE;
}

int pvsClass::getNextPvsName (
  char **name
) {
  return PVS_FAILURE;
}

int pvsClass::sendCmd (
  int socketFd,
  char *msg
) {
  return 0;
}

int pvsClass::getReply (
  int socketFd,
  char *msg,
  int maxLen
) {
  return 0;
}

int pvsClass::init ( void ) {
  return 0;
}

int pvsClass::readGroup ( void ) {
  return 0;
}

#endif
