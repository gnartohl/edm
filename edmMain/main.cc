//  edm - extensible display manager

//  Copyright (C) 1999 John W. Sinclair

//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#define FD_TABLE_SIZE getdtablesize()

#include <X11/Xlib.h>
#include <X11/Xlocale.h>
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/BulletinB.h>
#include <Xm/DrawingA.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>

#include "pv_factory.h"
#include "color_pkg.h"
#include "color_button.h"
#include "font_pkg.h"
#include "font_menu.h"
#include "gc_pkg.h"
#include "entry_form.h"

#include "process.h"
#include "app_pkg.h"
#include "act_win.h"
#include "act_grf.h"
#include "bindings.h"
#include "utility.h"
#include "crawler.h"
#include "lookup.h"

#include "sys_types.h"
#include "thread.h"

#include "main.str"

void doCmdPut (
  char *cmd
) {

char *ctx, *buf, *name, *typ, *val, *other;
int l = strlen( cmd );

  //fprintf( stderr, "l = %-d\n", l );
  //fprintf( stderr, "cmd = %s\n", cmd );

  if ( l > 1000 ) return;

  buf = new char[l+1];
  strcpy( buf, cmd );

  //fprintf( stderr, "l = %-d, buf = %s\n", l, buf );

  ctx = NULL;
  name = strtok_r( buf, "=:", &ctx );
  if ( !name ) {
    delete[] buf;
    return;
  }
  //fprintf( stderr, "name = [%s]\n", name );

  typ = strtok_r( NULL, "=:", &ctx );
  if ( !typ ) {
    delete[] buf;
    return;
  }
  //fprintf( stderr, "typ = [%s]\n", typ );

  val = strtok_r( NULL, "=:", &ctx );
  if ( !val ) {
    delete[] buf;
    return;
  }
  //fprintf( stderr, "val = [%s]\n", val );

  ProcessVariable *pv = the_PV_Factory->create( name );
  if ( pv->is_valid() ) {
    //fprintf( stderr, "valid\n" );
    if ( strcmp( typ, "s" ) == 0 ) {
      pv->putText( val );
    }
    else  if ( strcmp( typ, "i" ) == 0 ) {
      int iv = strtol( val, &other, 0 );
      pv->put( iv );
    }
    else {
      double dv = strtod( val, &other );
      pv->put( dv );
    }
    pv->release();
  }

  delete[] buf;

}

void setServerSocketFd (
  int fd
);

typedef struct main_node_tag { /* locked queue node */
  void *flink;
  void *blink;
  char msg[255+1];
} MAIN_NODE_TYPE, *MAIN_NODE_PTR;

typedef struct main_que_tag { /* locked queue header */
  void *flink;
  void *blink;
  void *lock;
  char msg[255+1];
} MAIN_QUE_TYPE, *MAIN_QUE_PTR;

#define REMQHI( queue, buf, flag )\
  sys_remqh( (void *) (queue), (void **) (buf), (int) (flag) )

#define INSQTI( buf, queue, flag )\
  sys_insqt( (void *) (buf), (void *) (queue), (int) (flag) )

#define QUEWASEMP SYS_QUEWASEMP
#define ONEENTQUE SYS_ONEENTQUE

#define MAIN_QUEUE_SIZE 200

#define DONE -1
#define SWITCHES 1
#define FILES 2

// remote one-byte command type
// (no future command type value may be 10 - '\n' is control char)
#define QUERY_LOAD 1
#define CONNECT 2
#define OPEN_INITIAL 3
#define OPEN 4
#define CONTROL 5
#define QUERY_DISPLAY 6
#define PUT 7

typedef struct argsTag {
  int argc;
  char **argv;
  appContextClass *appCtxPtr;
} argsType, *argsPtr;

typedef struct appListTag {
  struct appListTag *flink;
  struct appListTag *blink;
  argsPtr appArgs;
} appListType, *appListPtr;

static MAIN_QUE_TYPE g_mainFreeQueue, g_mainActiveQueue;
static MAIN_NODE_TYPE g_mainNodes[MAIN_QUEUE_SIZE];
static int g_numClients = 0;
static int g_pidNum = 0;
static char g_restartId[31+1];
static int g_displayIndex = 0;

#define MAXDSPNAMES 255
static char g_displayNames[MAXDSPNAMES+1][63+1];
static char g_defaultDisplay[63+1];

//#include "alloc.h"
#ifdef DIAGNOSTIC_ALLOC
int z=2, zz=0;
#endif


#define MAX_X_ERRORS 100

static int xIoErrorHandler (
  Display *d )
{

  fprintf( stderr, "xIoErrorHandler\n" );

  return 0;

}

static int xErrorHandler (
  Display *d,
  XErrorEvent *err )
{

char msg[80];
static int num = 0;

  if ( num > MAX_X_ERRORS ) {
    return 0;
  }
  else if ( num == MAX_X_ERRORS ) {
    num++;
    fprintf( stderr, "Too many X errors\n" );
    return 0;
  }

  if ( badWindowErrorsDisabled() && ( err->error_code == BadWindow ) ) {
    return 0;
  }

  if ( err->error_code == BadAccess ) {
    return 0;
  }

  XGetErrorText( d, err->error_code, msg, 80 );
  fprintf( stderr, main_str1, err->error_code, msg );

  num++;

  return 0;

}

void xtErrorHandler (
  char *msg )
{

static int num = 0;

  if ( num > MAX_X_ERRORS ) {
    return;
  }
  else if ( num == MAX_X_ERRORS ) {
    num++;
    fprintf( stderr, "Too many Xt errors\n" );
    return;
  }

  num++;

  if ( !strcmp( msg,
   "XtPopdown requires a subclass of shellWidgetClass" ) ) {
    return;
  }

  fprintf( stderr, "xtErrorHandler - %s\n", msg );

}

static void addDisplayToList (
  int argc,
  char **argv
) {

int i, n, found;
char dspName[63+1];

  strncpy( dspName, g_defaultDisplay, 63 );
  dspName[63] = 0;

  for ( i=0; i<argc; i++ ) {
    if ( strcmp( argv[i], "-display" ) == 0 ) {
      n = i+1;
      if ( n < argc ) {
        strncpy( dspName, argv[n], 63 );
        dspName[63] = 0;
      }
    }
  }

  //fprintf( stderr, "dspName = [%s]\n", dspName );

  found = 0;
  for ( i=0; i<g_displayIndex; i++ ) {
    //fprintf( stderr, "check [%s]\n", g_displayNames[i] );
    if ( strcmp( g_displayNames[i], dspName ) == 0 ) {
      found = 1;
      break;
    }
  }

  if ( !found ) {
    if ( g_displayIndex < MAXDSPNAMES ) {
      strncpy( g_displayNames[g_displayIndex], dspName, 63 );
      g_displayNames[g_displayIndex][63] = 0;
      g_displayIndex++;
    }
  }
  //else {
  //  fprintf( stderr, "found it\n" );
  //}

}

static void addDisplayToListByName (
  char *displayName
) {

int i, found;
char dspName[63+1];

  strncpy( dspName, displayName, 63 );
  dspName[63] = 0;

  found = 0;
  for ( i=0; i<g_displayIndex; i++ ) {
    //fprintf( stderr, "check [%s]\n", g_displayNames[i] );
    if ( strcmp( g_displayNames[i], dspName ) == 0 ) {
      found = 1;
      break;
    }
  }

  if ( !found ) {
    if ( g_displayIndex < MAXDSPNAMES ) {
      strncpy( g_displayNames[g_displayIndex], dspName, 63 );
      g_displayNames[g_displayIndex][63] = 0;
      g_displayIndex++;
    }
  }
  //else {
  //  fprintf( stderr, "found it\n" );
  //}

}

static void getCheckPointFileNamefromPID (
  char *checkPointFileName
) {

char procIdName[31+1], *envPtr;
SYS_PROC_ID_TYPE procId;

  envPtr = getenv( environment_str8 );
  if ( envPtr ) {
    strncpy( checkPointFileName, envPtr, 255 );
    if ( envPtr[strlen(envPtr)] != '/' ) {
      Strncat( checkPointFileName, "/", 255 );
    }
  }
  else {
    strncpy( checkPointFileName, "/tmp/", 255 );
  }
  Strncat( checkPointFileName, "edmCheckPointFile_", 255 );
  sys_get_proc_id( &procId );
  sprintf( procIdName, "%-d", (int) procId.id );
  Strncat( checkPointFileName, procIdName, 255 );
  //fprintf( stderr, "[%s]\n", checkPointFileName );

}

static void getCheckPointFileName (
  char *checkPointFileName,
  char *procIdName
) {

char *envPtr;

  envPtr = getenv( environment_str8 );
  if ( envPtr ) {
    strncpy( checkPointFileName, envPtr, 255 );
    if ( envPtr[strlen(envPtr)] != '/' ) {
      Strncat( checkPointFileName, "/", 255 );
    }
  }
  else {
    strncpy( checkPointFileName, "/tmp/", 255 );
  }
  Strncat( checkPointFileName, "edmCheckPointFile_", 255 );
  Strncat( checkPointFileName, procIdName, 255 );
  //fprintf( stderr, "%-d, [%s]\n", g_pidNum, checkPointFileName );

}

static int getServerCheckPointParams (
  FILE *f,
  int *server,
  int *oneInstance,
  char *displayName,
  int *noEdit,
  int *numCheckPointMacros,
  char *checkPointMacros
) {

char *cptr, *tk, *buf1;
char text[1023+1];
int i, n, ii, nn, tmp, sanity;

  *server = 0;
  *oneInstance = 0;
  strcpy( displayName, "" );
  *numCheckPointMacros = 0;
  strcpy( checkPointMacros, "" );

  sanity = 99999;
  do {

    cptr = fgets( text, 1023, f );
    text[1023] = 0;
    if ( !cptr ) return 2; // fail
    if ( strcmp( text, "<<<EOD>>>\n" ) == 0 ) return 3; // no more data
    tmp = atol( text );
    *server = tmp & 0xf;
    *oneInstance = ( tmp >> 8 );

    if ( !(*server) ) {

      cptr = fgets( text, 1023, f );
      text[1023] = 0;
      if ( !cptr ) return 2; // fail

      cptr = fgets( text, 1023, f );
      text[1023] = 0;
      if ( !cptr ) return 2; // fail

      cptr = fgets( text, 1023, f );
      text[1023] = 0;
      if ( !cptr ) return 2; // fail
      n = atol( text );

      // skip all global macros
      for ( i=0; i<n; i++ ) {

        cptr = fgets( text, 1023, f );
        text[1023] = 0;
        if ( !cptr ) return 2; // fail

      }

      cptr = fgets( text, 1023, f );
      text[1023] = 0;
      if ( !cptr ) return 2; // fail
      n = atol( text );

      // skip all screens
      for ( i=0; i<n; i++ ) {

        cptr = fgets( text, 1023, f );
        text[1023] = 0;
        if ( !cptr ) return 2; // fail

        cptr = fgets( text, 1023, f );
        text[1023] = 0;
        if ( !cptr ) return 2; // fail

        cptr = fgets( text, 1023, f );
        text[1023] = 0;
        if ( !cptr ) return 2; // fail

        cptr = fgets( text, 1023, f );
        text[1023] = 0;
        if ( !cptr ) return 2; // fail

        cptr = fgets( text, 1023, f );
        text[1023] = 0;
        if ( !cptr ) return 2; // fail

        cptr = fgets( text, 1023, f );
        text[1023] = 0;
        if ( !cptr ) return 2; // fail
        nn = atol( text );

        // skip all screen macros
        for ( ii=0; ii<nn; ii++ ) {

          cptr = fgets( text, 1023, f );
          text[1023] = 0;
          if ( !cptr ) return 2; // fail

        }

      }

    }
    else {

      readStringFromFile( displayName, 63, f );

      cptr = fgets( text, 1023, f );
      text[1023] = 0;
      if ( !cptr ) return 2; // fail
      *noEdit = atol( text );

      cptr = fgets( text, 1023, f );
      text[1023] = 0;
      if ( !cptr ) return 2; // fail
      *numCheckPointMacros = atol( text );

      for ( i=0; i<*numCheckPointMacros; i++ ) {
        cptr = fgets( text, 1023, f );
        text[1023] = 0;
        if ( !cptr ) return 2; // fail
        buf1 = NULL;
        tk = strtok_r( text, "\n \t", &buf1 );
        if ( i > 0 ) {
          Strncat( checkPointMacros, ",", 1023 );
          checkPointMacros[1023] = 0;
        }
        Strncat( checkPointMacros, tk, 1023 );
        checkPointMacros[1023] = 0;
      }

      return 1;

    }

  } while ( --sanity );

  return 2;

}

static int getMainCheckPointParams (
  FILE *f,
  int *server,
  int *oneInstance,
  char *displayName,
  int *noEdit,
  int *numCheckPointMacros,
  char *checkPointMacros
) {

char *cptr, *tk, *buf1;
char text[1023+1];
int i, n, ii, nn, tmp, sanity;

  // get params for non-server

  *server = 0;
  *oneInstance = 0;
  strcpy( displayName, "" );
  *numCheckPointMacros = 0;
  strcpy( checkPointMacros, "" );

  sanity = 99999;
  do {

    cptr = fgets( text, 1023, f );
    text[1023] = 0;
    if ( !cptr ) return 2; // fail
    if ( strcmp( text, "<<<EOD>>>\n" ) == 0 ) return 3; // no more data
    tmp = atol( text );
    *server = tmp & 0xf;
    *oneInstance = ( tmp >> 8 );

    if ( *server ) {

      cptr = fgets( text, 1023, f );
      text[1023] = 0;
      if ( !cptr ) return 2; // fail

      cptr = fgets( text, 1023, f );
      text[1023] = 0;
      if ( !cptr ) return 2; // fail

      cptr = fgets( text, 1023, f );
      text[1023] = 0;
      if ( !cptr ) return 2; // fail
      n = atol( text );

      // skip all global macros
      for ( i=0; i<n; i++ ) {

        cptr = fgets( text, 1023, f );
        text[1023] = 0;
        if ( !cptr ) return 2; // fail

      }

      cptr = fgets( text, 1023, f );
      text[1023] = 0;
      if ( !cptr ) return 2; // fail
      n = atol( text );

      // skip all screens
      for ( i=0; i<n; i++ ) {

        cptr = fgets( text, 1023, f );
        text[1023] = 0;
        if ( !cptr ) return 2; // fail

        cptr = fgets( text, 1023, f );
        text[1023] = 0;
        if ( !cptr ) return 2; // fail

        cptr = fgets( text, 1023, f );
        text[1023] = 0;
        if ( !cptr ) return 2; // fail

        cptr = fgets( text, 1023, f );
        text[1023] = 0;
        if ( !cptr ) return 2; // fail

        cptr = fgets( text, 1023, f );
        text[1023] = 0;
        if ( !cptr ) return 2; // fail

        cptr = fgets( text, 1023, f );
        text[1023] = 0;
        if ( !cptr ) return 2; // fail
        nn = atol( text );

        // skip all screen macros
        for ( ii=0; ii<nn; ii++ ) {

          cptr = fgets( text, 1023, f );
          text[1023] = 0;
          if ( !cptr ) return 2; // fail

        }

      }

    }
    else {

      readStringFromFile( displayName, 63, f );

      cptr = fgets( text, 1023, f );
      text[1023] = 0;
      if ( !cptr ) return 2; // fail
      *noEdit = atol( text );

      cptr = fgets( text, 1023, f );
      text[1023] = 0;
      if ( !cptr ) return 2; // fail
      *numCheckPointMacros = atol( text );

      for ( i=0; i<*numCheckPointMacros; i++ ) {
        cptr = fgets( text, 1023, f );
        text[1023] = 0;
        if ( !cptr ) return 2; // fail
        buf1 = NULL;
        tk = strtok_r( text, "\n \t", &buf1 );
        if ( i > 0 ) {
          Strncat( checkPointMacros, ",", 1023 );
          checkPointMacros[1023] = 0;
        }
        Strncat( checkPointMacros, tk, 1023 );
        checkPointMacros[1023] = 0;
      }

      return 1;

    }

  } while ( --sanity );

  return 2;

}

static int getNumCheckPointScreens (
  FILE *f
) {

char text[31+1], *cptr;
int n;

  cptr = fgets( text, 31, f );
  if ( !cptr ) return 0; // fail
  text[31] = 0;
  n = atol( text );
  return n;

}

static int getScreenCheckPointParams (
  FILE *f,
  char *screenName,
  int *x,
  int *y,
  int *icon,
  int *noEdit,
  int *numCheckPointMacros,
  char *checkPointMacros
) {

char *cptr, *tk, *buf1;
char text[1023+1];
int i;

  strcpy( screenName, "" );
  *x = 0;
  *y = 0;
  *icon = 0;
  *numCheckPointMacros = 0;
  strcpy( checkPointMacros, "" );

  readStringFromFile( screenName, 255, f );

  cptr = fgets( text, 1023, f );
  text[1023] = 0;
  if ( !cptr ) return 2; // fail
  *x = atol( text );

  cptr = fgets( text, 1023, f );
  text[1023] = 0;
  if ( !cptr ) return 2; // fail
  *y = atol( text );

  cptr = fgets( text, 1023, f );
  text[1023] = 0;
  if ( !cptr ) return 2; // fail
  *icon = atol( text );

  cptr = fgets( text, 1023, f );
  text[1023] = 0;
  if ( !cptr ) return 2; // fail
  *noEdit = atol( text );

  cptr = fgets( text, 1023, f );
  text[1023] = 0;
  if ( !cptr ) return 2; // fail
  *numCheckPointMacros = atol( text );

  for ( i=0; i<*numCheckPointMacros; i++ ) {
    cptr = fgets( text, 1023, f );
    text[1023] = 0;
    if ( !cptr ) return 2; // fail
    buf1 = NULL;
    tk = strtok_r( text, "\n \t", &buf1 );
    if ( i > 0 ) {
      Strncat( checkPointMacros, ",", 1023 );
      checkPointMacros[1023] = 0;
    }
    Strncat( checkPointMacros, tk, 1023 );
    checkPointMacros[1023] = 0;
  }

  return 1;

}

static int sendCmd (
  int socketFd,
  char *msg,
  int len
) {

struct timeval timeout;
int more, fd, i, remain;
fd_set fds;

  timeout.tv_sec = 5;
  timeout.tv_usec = 0;

  more = 1;
  i = 0;
  remain = len;
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

static int getReply (
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
    if ( len < 0 ) return len; // error
    if ( len == 0 ) return i; // return total chars read
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

int getHostAddr (
  char *name,
  int *addr )
{

struct hostent *entry;
char *tk, *buf;

  *addr = 0;

  tk = strtok_r( name, ":,", &buf );
  if ( !tk ) tk = name;

  entry = gethostbyname( tk );
  if ( entry->h_length != 4 ) return -1;

  *addr = *( (int *) (entry->h_addr_list)[0] );

  return 0;

}

void checkForServer (
  int argc,
  char **argv,
  int portNum,
  int appendDisplay,
  char *displayName,
  int oneInstance,
  int openCmd
) {

char chkHost[31+1], host[31+1], buf[511+1];
int i, len, pos, max, argCount, stat, item, useItem;
const int MAX_MSG_LEN = 256;
char msg[MAX_MSG_LEN+1];
SYS_TIME_TYPE timeout;
char *envPtr, *tk1, *tk2, *buf1, *buf2;
double merit, min, num;
struct sockaddr_in s;
int ip_addr, sockfd;
unsigned short port_num;
int value, n, nIn, nOut;

  //fprintf( stderr, "displayName = [%s]\n", displayName );

  envPtr = getenv( "EDMSERVERS" );

  if ( !envPtr ) {
    stat = gethostname( host, 31 );
    if ( stat ) return;
    envPtr = host;
  }

  if ( appendDisplay ) {
    argCount = argc + 2; // we will add two parameters below
  }
  else {
    argCount = argc;
  }

  stat = sys_cvt_seconds_to_timeout( 10.0, &timeout );
  if ( !( stat & 1 ) ) {
    fprintf( stderr, main_str3 );
    return;
  }

  // if not doing a -open operation, do simple load balancing
  if ( !openCmd ) {

    strncpy( buf, envPtr, 511 );
    buf[511] = 0;

    tk1 = strtok_r( buf, ",", &buf1 );
    if ( tk1 ) {
      strncpy( host, tk1, 31 );
      host[31] = 0;
      min = -1;
      useItem = 1;
    }

    item = 1;
    while ( tk1 ) {

      strncpy( chkHost, tk1, 31 );
      chkHost[31] = 0;
      merit = 1.0;

      // use msg as a tmp buffer
      strncpy( msg, tk1, 254 );
      msg[254] = 0;
      tk2 = strtok_r( msg, ":", &buf2 );

      if ( tk2 ) {

        strncpy( chkHost, tk2, 31 );

        tk2 = strtok_r( NULL, ":", &buf2 );
        if ( tk2 ) {
          merit = atof( tk2 );
          if ( merit <= 0.0 ) merit = 1.0;
        }

      }

      //fprintf( stderr, "Checking host [%s], merit = %-f\n", chkHost, merit );

      stat = getHostAddr( chkHost, &ip_addr );
      if ( stat ) return;

      sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
      if ( sockfd == -1 ) {
        //perror( "" );
        return;
      }

      {
        int flags, status;
        flags = fcntl( sockfd, F_GETFD );
        if ( flags >= 0 ) {
          status = fcntl( sockfd, F_SETFD, flags | FD_CLOEXEC );
        }
      }

      value = 1;
      len = sizeof(value);
      stat = setsockopt( sockfd, SOL_SOCKET, SO_KEEPALIVE,
       &value, len );

      port_num = (unsigned short) portNum;

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

      //fprintf( stderr, "connected\n" );

      msg[0] = (char) QUERY_LOAD;
      msg[1] = '\n';
      msg[2] = 0;

      nOut = sendCmd( sockfd, msg, 2 );
      if ( !nOut ) {
        goto nextHost;
      }

      strcpy( msg, "" );

      nIn = getReply( sockfd, msg, 255 );

      sscanf( msg, "%d", &n );

      //fprintf( stderr, "nIn = %-d, reply = %-d\n", nIn, n );

      if ( !nIn ) {
        goto nextHost;
      }

      num = (double) n / merit;

      if ( ( num < min ) || ( min == -1 ) ) {
        min = num;
        strncpy( host, chkHost, 31 );
        host[31] = 0;
        useItem = item;
      }

      //fprintf( stderr, "min = %-f, adj num = %-f\n", min, num );

nextHost:

      // don't check status, we're probably already disconnected
      stat = shutdown( sockfd, 2 );
      stat = close( sockfd );

      item++;
      tk1 = strtok_r( NULL, ",", &buf1 );

    }

  }

  // if we are doing a -open operation, find first server that is managing our display
  else {

    strncpy( buf, envPtr, 511 );
    buf[511] = 0;

    tk1 = strtok_r( buf, ",", &buf1 );
    if ( tk1 ) {
      strncpy( host, tk1, 31 );
      host[31] = 0;
    }

    item = 0;
    while ( tk1 ) {

      strncpy( chkHost, tk1, 31 );
      chkHost[31] = 0;

      //fprintf( stderr, "Checking host [%s]\n", chkHost );

      stat = getHostAddr( chkHost, &ip_addr );
      if ( stat ) return;

      sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
      if ( sockfd == -1 ) {
        //perror( "" );
        return;
      }

      {
        int flags, status;
        flags = fcntl( sockfd, F_GETFD );
        if ( flags >= 0 ) {
          status = fcntl( sockfd, F_SETFD, flags | FD_CLOEXEC );
        }
      }

      value = 1;
      len = sizeof(value);
      stat = setsockopt( sockfd, SOL_SOCKET, SO_KEEPALIVE,
       &value, len );

      port_num = (unsigned short) portNum;

      port_num = htons( port_num );

      memset( (char *) &s, 0, sizeof(s) );
      s.sin_family = AF_INET;
      s.sin_addr.s_addr = ip_addr;
      s.sin_port = port_num;

      stat = connect( sockfd, (struct sockaddr *) &s, sizeof(s) );
      if ( stat ) {
        if ( debugMode() ) perror( "connect" );
        close( sockfd );
        goto abortClose;
      }

      //fprintf( stderr, "connected\n" );

      msg[0] = (char) QUERY_DISPLAY;
      strcpy( &msg[1], displayName );
      Strncat( msg, "\n", MAX_MSG_LEN );
      msg[MAX_MSG_LEN] = 0;

      nOut = sendCmd( sockfd, msg, strlen(msg) );
      if ( !nOut ) {
        goto nextHost1;
      }

      strcpy( msg, "" );

      nIn = getReply( sockfd, msg, 255 );

      //fprintf( stderr, "nIn = %-d, reply = %s\n", nIn, msg );

      if ( !nIn ) {
        goto nextHost1;
      }

      if ( strcmp( msg, "1" ) == 0 ) {
        //fprintf( stderr, "got yes response from %s\n", chkHost );
        strncpy( host, chkHost, 31 );
        host[31] = 0;
        item = 1;
        useItem = item;
	break;
      }

nextHost1:

      // don't check status, we're probably already disconnected
      stat = shutdown( sockfd, 2 );
      stat = close( sockfd );

      if ( item ) break;

      tk1 = strtok_r( NULL, ",", &buf1 );

    }

  }

  if ( openCmd && !item ) {
    return;
  }

  //fprintf( stderr, "Using host [%s], item %-d\n", host, useItem );

  stat = getHostAddr( host, &ip_addr );
  if ( stat ) return;

  sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
  if ( sockfd == -1 ) {
    //perror( "" );
    return;
  }

  {
    int flags, status;
    flags = fcntl( sockfd, F_GETFD );
    if ( flags >= 0 ) {
      status = fcntl( sockfd, F_SETFD, flags | FD_CLOEXEC );
    }
  }

  value = 1;
  len = sizeof(value);
  stat = setsockopt( sockfd, SOL_SOCKET, SO_KEEPALIVE,
   &value, len );

  port_num = (unsigned short) portNum;

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

  //fprintf( stderr, "connected\n" );

  msg[MAX_MSG_LEN] = 0;

  if ( oneInstance ) {

    if ( openCmd == 1 ) {

      msg[0] = (char) OPEN;
      pos = 1;
      max = MAX_MSG_LEN - pos;

      strncpy( &msg[pos], "*OPN*|", max );
      pos = strlen(msg);
      max = MAX_MSG_LEN - pos;

    }
    else if ( openCmd == 3 ) {

      msg[0] = (char) PUT;
      pos = 1;
      max = MAX_MSG_LEN - pos;

      strncpy( &msg[pos], "*PUT*|", max );
      pos = strlen(msg);
      max = MAX_MSG_LEN - pos;

    }
    else if ( openCmd == 2 ) {

      msg[0] = (char) CONTROL;
      pos = 1;
      max = MAX_MSG_LEN - pos;

      strncpy( &msg[pos], "*CTL*|", max );
      pos = strlen(msg);
      max = MAX_MSG_LEN - pos;

    }
    else {

      msg[0] = (char) OPEN_INITIAL;
      pos = 1;
      max = MAX_MSG_LEN - pos;

      strncpy( &msg[pos], "*OIS*|", max );
      pos = strlen(msg);
      max = MAX_MSG_LEN - pos;

    }

    strncpy( &msg[pos], displayName, max );
    pos = strlen(msg);
    max = MAX_MSG_LEN - pos;

    strncpy( &msg[pos], "|", max );
    pos = strlen(msg);
    max = MAX_MSG_LEN - pos;

    snprintf( &msg[pos], max, "%-d|", argCount );
    pos = strlen(msg);
    max = MAX_MSG_LEN - pos;

    strncpy( &msg[pos], "edm|", max );
    pos = strlen(msg);
    max = MAX_MSG_LEN - pos;

    if ( appendDisplay ) {
      strncpy( &msg[pos], global_str56, max );
      pos = strlen(msg);
      max = MAX_MSG_LEN - pos;
      strncpy( &msg[pos], displayName, max );
      pos = strlen(msg);
      max = MAX_MSG_LEN - pos;
      Strncat( &msg[pos], "|", max );
      pos = strlen(msg);
      max = MAX_MSG_LEN - pos;
    }

    for ( i=1; i<argc; i++ ) {
      strncpy( &msg[pos], argv[i], max );
      pos = strlen(msg);
      max = MAX_MSG_LEN - pos;
      Strncat( &msg[pos], "|", max );
      pos = strlen(msg);
      max = MAX_MSG_LEN - pos;
    }

  }
  else {

    msg[0] = (char) CONNECT;
    pos = 1;
    max = MAX_MSG_LEN - pos;

    snprintf( &msg[pos], max, "%-d|", argCount );
    pos = strlen(msg);
    max = MAX_MSG_LEN - pos;

    strncpy( &msg[pos], "edm|", max );
    pos = strlen(msg);
    max = MAX_MSG_LEN - pos;

    if ( appendDisplay ) {
      strncpy( &msg[pos], global_str56, max );
      pos = strlen(msg);
      max = MAX_MSG_LEN - pos;
      strncpy( &msg[pos], displayName, max );
      pos = strlen(msg);
      max = MAX_MSG_LEN - pos;
      Strncat( &msg[pos], "|", max );
      pos = strlen(msg);
      max = MAX_MSG_LEN - pos;
    }

    for ( i=1; i<argc; i++ ) {
      strncpy( &msg[pos], argv[i], max );
      pos = strlen(msg);
      max = MAX_MSG_LEN - pos;
      Strncat( &msg[pos], "|", max );
      pos = strlen(msg);
      max = MAX_MSG_LEN - pos;
    }

  }

  Strncat( msg, "\n", MAX_MSG_LEN );

  if ( strlen(msg) == (unsigned int) MAX_MSG_LEN ) {
    fprintf( stderr, "Message length exceeded - abort\n" );
    stat = shutdown( sockfd, 2 );
    stat = close( sockfd );
    exit(1);
  }

  nOut = sendCmd( sockfd, msg, strlen(msg) );
  if ( !nOut ) {
    goto abort;
  }

  // don't check status, we're probably already disconnected
  stat = shutdown( sockfd, 2 );
  stat = close( sockfd );

  exit(0);

abort:
  stat = shutdown( sockfd, 2 );

abortClose:
  stat = close( sockfd );

  return;

}

static int reply (
  int socketFd,
  char *msg,
  int len
) {

struct timeval timeout;
int more, fd, i, remain;
fd_set fds;

  timeout.tv_sec = 5;
  timeout.tv_usec = 0;

  more = 1;
  i = 0;
  remain = len;
  while ( more ) {

    FD_ZERO( &fds );
    FD_SET( socketFd, &fds );

    fd = select( FD_TABLE_SIZE, (fd_set *) NULL, &fds,
     (fd_set *) NULL, &timeout );

    if ( fd == 0 ) { /* timeout */
      /* fprintf( stderr, "timeout\n" ); */
      return 0;
    }

    if ( fd < 0 ) { /* error */
      perror( "select" );
      return 0;
    }

    len = write( socketFd, &msg[i], remain );
    if ( len < 1 ) return len; /* error */

    remain -= len;
    i += len;

    if ( remain < 1 ) more = 0;

  } while ( more );

  return i;

}

static int getCommand (
  int socketFd,
  char *msg,
  int maxLen
) {

struct timeval timeout;
int more, i, ii, remain, len, count, n;
fd_set fds;

  timeout.tv_sec = 5;
  timeout.tv_usec = 0;

  more = 1;
  i = count = 0;
  remain = maxLen;
  while ( more ) {

    /* fprintf( stderr, "socketFd = %-d\n", socketFd ); */

    FD_ZERO( &fds );
    FD_SET( socketFd, &fds );

    n = select( FD_TABLE_SIZE, &fds, (fd_set *) NULL,
     (fd_set *) NULL, &timeout );

    if ( n == 0 ) { /* timeout */
      return 0;
    }
    if ( n < 0 ) { /* error */
      perror( "select" );
      return n;
    }

    strcpy( msg, "" );

    len = read( socketFd, &msg[i], remain );
    if ( len < 0 ) return len; // error
    if ( len == 0 ) return i; // return total chars read

    for ( ii=0; ii<len; ii++ ) {
      if ( msg[i+ii] == '\n' ) {
        msg[i+ii] = 0;
        len = strlen(msg);
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

  return len;

}

#ifdef __linux__
void *caPendThread (
  THREAD_HANDLE h )
{
#endif

#ifdef darwin
void *caPendThread (
  THREAD_HANDLE h )
{
#endif

#ifdef __solaris__
void *caPendThread (
  THREAD_HANDLE h )
{
#endif

#ifdef __osf__
void caPendThread (
  THREAD_HANDLE h )
{
#endif

#ifdef HP_UX
void *caPendThread (
  THREAD_HANDLE h )
{
#endif

int stat;

  do {

    stat = pend_io( 5.0 );
    stat = pend_event( 0.05 );

  } while ( 1 );
  return 0;
}

#ifdef __linux__
void *serverThread (
  THREAD_HANDLE h )
{
#endif

#ifdef darwin
void *serverThread (
  THREAD_HANDLE h )
{
#endif

#ifdef __solaris__
void *serverThread (
  THREAD_HANDLE h )
{
#endif

#ifdef __osf__
void serverThread (
  THREAD_HANDLE h )
{
#endif

#ifdef HP_UX
void *serverThread (
  THREAD_HANDLE h )
{
#endif

int value, stat, i, n, n_in, q_stat_r, q_stat_i;
THREAD_HANDLE delayH;
MAIN_NODE_PTR node;
SYS_TIME_TYPE timeout;
int len, num, cmd=0;
char msg[255+1], msg1[255+1];
struct sockaddr_in s, cli_s;
int sockfd, newsockfd, more;
unsigned short port_num;

#ifdef HPUX
  int cliLen;
#else
  socklen_t cliLen;
#endif

int *portNumPtr = (int *) thread_get_app_data( h );

  stat = thread_create_handle( &delayH, NULL );

  stat = sys_cvt_seconds_to_timeout( 10.0, &timeout );
  if ( !( stat & 1 ) ) {
    fprintf( stderr, main_str3 );
    goto err_return;
  }

  n = 0;
  while ( 1 ) {

    sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if ( sockfd == -1 ) {
      perror( "socket" );
      goto err_return;
    }

    {
      int flags, status;
      flags = fcntl( sockfd, F_GETFD );
      if ( flags >= 0 ) {
        status = fcntl( sockfd, F_SETFD, flags | FD_CLOEXEC );
      }
    }

    value = 1;
    len = sizeof(value);

    stat = setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR,
     &value, len );

    if ( sockfd == -1 ) {
      perror( "setsockopt" );
      goto err_return;
    }

    port_num = (unsigned short) *portNumPtr;

    port_num = htons( port_num );

    memset( (char *) &s, 0, sizeof(s) );
    s.sin_family = AF_INET;
    s.sin_addr.s_addr = htonl(INADDR_ANY);
    s.sin_port = port_num;

    /* do bind and listen */
    stat = bind( sockfd, (struct sockaddr*) &s, sizeof(s) );
    if ( stat < 0 ) {
      perror( "bind" );
      goto err_return;
    }

    stat = listen( sockfd, 5 );
    if ( stat < 0 ) {
      perror( "listen" );
      goto err_return;
    }

    setServerSocketFd( sockfd );

    more = 1;
    while ( more ) {

      /*fprintf( stderr, "sockfd = %-d\n", sockfd ); */

      /* accept connection */
      cliLen = sizeof(cli_s);
      newsockfd = accept( sockfd, (struct sockaddr *) &cli_s,
       &cliLen );

      if ( newsockfd < 0 ) {
        perror( "accept" );
        goto err_branch;
      }

      n = getCommand( newsockfd, msg, 255 );
      if ( n ) {

        cmd = (int) msg[0];

	//fprintf( stderr, "cmd = %-d\n", cmd );
	//fprintf( stderr, "msg = %s\n", &msg[1] );

        switch ( cmd ) {

        case OPEN_INITIAL:
        case OPEN:

          stat = thread_lock_master( h );

          q_stat_r = REMQHI( (void *) &g_mainFreeQueue, (void **) &node, 0 );
          if ( q_stat_r & 1 ) {
            strncpy( node->msg, &msg[1], 254 );
            q_stat_i = INSQTI( (void *) node, (void *) &g_mainActiveQueue, 0 );
            if ( !( q_stat_i & 1 ) ) {
              fprintf( stderr, main_str17 );
            }
          }
          else {
            fprintf( stderr, main_str18 );
          }

          stat = thread_unlock_master( h );

          break;

        case CONTROL:
	case PUT:

          stat = thread_lock_master( h );

          q_stat_r = REMQHI( (void *) &g_mainFreeQueue, (void **) &node, 0 );
          if ( q_stat_r & 1 ) {
            strncpy( node->msg, &msg[1], 254 );
            q_stat_i = INSQTI( (void *) node, (void *) &g_mainActiveQueue, 0 );
            if ( !( q_stat_i & 1 ) ) {
              fprintf( stderr, main_str17 );
            }
          }
          else {
            fprintf( stderr, main_str18 );
          }

          stat = thread_unlock_master( h );

          break;

        case QUERY_LOAD:

          stat = thread_lock_master( h );
          num = g_numClients;
          stat = thread_unlock_master( h );

          snprintf( msg1, 255, "%-d\n", num );

          n_in = reply( newsockfd, msg1, strlen(msg1) );
          if ( n_in < 1 ) {
            fprintf( stderr, main_str44 );
          }

          break;

        case QUERY_DISPLAY:

          stat = thread_lock_master( h );

	  num = 0;
          if ( debugMode() ) {
            fprintf( stderr, "searching for display name:\n" );
	  }
	  for ( i=0; i<g_displayIndex; i++ ) {
            if ( debugMode() ) {
	      fprintf( stderr, "  client request: [%s]   found: [%s]\n", &msg[1], g_displayNames[i] );
	    }
	    if ( strcmp( &msg[1], g_displayNames[i] ) == 0 ) {
              num = 1;
	    }
	  }

          stat = thread_unlock_master( h );

          snprintf( msg1, 255, "%-d\n", num );

          n_in = reply( newsockfd, msg1, strlen(msg1) );
          if ( n_in < 1 ) {
            fprintf( stderr, main_str44 );
          }

          break;

        case CONNECT:

          stat = thread_lock_master( h );

          q_stat_r = REMQHI( (void *) &g_mainFreeQueue, (void **) &node, 0 );
          if ( q_stat_r & 1 ) {
            n++;
            strncpy( node->msg, &msg[1], 254 );
            q_stat_i = INSQTI( (void *) node, (void *) &g_mainActiveQueue, 0 );
            if ( !( q_stat_i & 1 ) ) {
              fprintf( stderr, main_str17 );
            }
          }
          else {
            fprintf( stderr, main_str18 );
          }

          stat = thread_unlock_master( h );

          break;

        }

      }

      //disconnect asychronously

      stat = shutdown( newsockfd, 2 );

      stat = close( newsockfd );

    }

err_branch:

    stat = shutdown( sockfd, 2 );
    stat = close( sockfd );

    if ( cmd == CONNECT ) {
      thread_delay( delayH, 0.5 );
    }

  }

err_return:

  stat = 0;

#ifdef __linux__
  return NULL;
#endif

#ifdef darwin
  return NULL;
#endif

#ifdef __solaris__
  return NULL;
#endif

#ifdef HP_UX
  return NULL;
#endif

}

static void checkParams (
  int argc,
  char **argv,
  int *local,
  int *server,
  int *appendDisplay,
  char *displayName,
  int *portNum,
  int *restart,
  int *oneInstance,
  int *openCmd,
  int *convertOnly,
  int *crawl,
  int *verbose
) {

char buf[1023+1], mac[1023+1], exp[1023+1];
int state = SWITCHES;
int l, nm = 0, n = 1;
char *tk, *buf1;

  strcpy( displayName, "" );
  *local = 0;
  *server = 0;
  *oneInstance = 0;
  *openCmd = 0;
  *appendDisplay = 1;
  *portNum = 19000;
  *restart = 0;
  *convertOnly = 0;
  *crawl = 0;
  *verbose = 0;

  // check first for component management commands
  if ( argc > 1 ) {

    if ( ( strcmp( argv[1], global_str6 ) == 0 ) ||
         ( strcmp( argv[1], global_str7 ) == 0 ) ||
         ( strcmp( argv[1], global_str8 ) == 0 ) ) {
      *local = 1;
      return;
    }

  }

  while ( n < argc ) {

    switch ( state ) {

    case SWITCHES:

      if ( argv[n][0] == '-' ) {

        //fprintf( stderr, "argv[%-d] = %s\n", n, argv[n] );

        if ( strcmp( argv[n], global_str9 ) == 0 ) {
          *local = 1;
        }
	else if ( strcmp( argv[n], global_str91 ) == 0 ) {
          *convertOnly = 1;
	  *oneInstance = 0;
          *server = 0;
          *local = 1;
	}
	else if ( strcmp( argv[n], global_str102 ) == 0 ) {
          *convertOnly = 0;
	  *oneInstance = 0;
          *server = 0;
          *local = 1;
	  *crawl = 1;
	}
	else if ( strcmp( argv[n], global_str104 ) == 0 ) {
	  *verbose = 1;
	}
        else if ( strcmp( argv[n], global_str10 ) == 0 ) {
          *server = 1;
          *local = 0;
        }
	else if ( strcmp( argv[n], global_str89 ) == 0 ) {
	  *oneInstance = 1;
          *server = 1;
          *local = 0;
        }
	else if ( strcmp( argv[n], global_str93 ) == 0 ) {
	  *oneInstance = 1;
          *server = 1;
          *local = 0;
          *openCmd = 1;
        }
	else if ( strcmp( argv[n], global_str128 ) == 0 ) { // close
	  *oneInstance = 1;
          *server = 1;
          *local = 0;
          *openCmd = 2; // control named window
	}
	else if ( strcmp( argv[n], global_str130 ) == 0 ) { // move
	  *oneInstance = 1;
          *server = 1;
          *local = 0;
          *openCmd = 2; // control named window
	}
	else if ( strcmp( argv[n], global_str132 ) == 0 ) { // raise
	  *oneInstance = 1;
          *server = 1;
          *local = 0;
          *openCmd = 2; // control named window
	}
	else if ( strcmp( argv[n], global_str134 ) == 0 ) { // lower
	  *oneInstance = 1;
          *server = 1;
          *local = 0;
          *openCmd = 2; // control named window
	}
	else if ( strcmp( argv[n], global_str136 ) == 0 ) { // iconify
	  *oneInstance = 1;
          *server = 1;
          *local = 0;
          *openCmd = 2; // control named window
	}
	else if ( strcmp( argv[n], global_str138 ) == 0 ) { // deiconify
	  *oneInstance = 1;
          *server = 1;
          *local = 0;
          *openCmd = 2; // control named window
	}
	else if ( strcmp( argv[n], global_str146 ) == 0 ) { // put
	  *oneInstance = 1;
          *server = 1;
          *local = 0;
          *openCmd = 3; // control named window
	}
	else if ( strcmp( argv[n], global_str140 ) == 0 ) { // xwdsnap
	  *oneInstance = 1;
          *server = 1;
          *local = 0;
          *openCmd = 2; // control named window
	}
        else if ( strcmp( argv[n], global_str86 ) == 0 ) {
          n++;
          if ( n >= argc ) { // missing pid num
            return;
          }
          *restart = 1;
          g_pidNum = atol( argv[n] );
	  l = strlen( argv[n] );
	  if ( l > 31 ) l = 31;
	  strncpy( g_restartId, argv[n], l );
	}
        else if ( strcmp( argv[n], global_str11 ) == 0 ) {
          *local = 1;
          return;
        }
        else if ( strcmp( argv[n], global_str12 ) == 0 ) {
          *local = 1;
          return;
        }
        else if ( strcmp( argv[n], global_str13 ) == 0 ) {
          *local = 1;
          return;
        }
        else if ( strcmp( argv[n], global_str14 ) == 0 ) {
          *local = 1;
          return;
        }
        else if ( strcmp( argv[n], global_str15 ) == 0 ) {
          *local = 1;
          return;
        }
        else if ( strcmp( argv[n], global_str16 ) == 0 ) {
          *local = 1;
          return; 
       }
        else if ( strcmp( argv[n], global_str17 ) == 0 ) {
        }
        else if ( strcmp( argv[n], global_str18 ) == 0 ) {
        }
        else if ( strcmp( argv[n], global_str19 ) == 0 ) {
          n++;
          if ( n >= argc ) { // missing macro arg
            *local = 1;
            return;
          }
          strncpy( buf, argv[n], 1023 );
	  buf1 = NULL;
          tk = strtok_r( buf, "=,", &buf1 );
          while ( tk ) {
            strncpy( mac, tk, 1023 );
            tk = strtok_r( NULL, "=,", &buf1 );
            if ( tk ) {
              strncpy( exp, tk, 1023 );
            }
            else {
              *local = 1;
              return;
            }
            nm++;
            tk = strtok_r( NULL, "=,", &buf1 );
          }

          if ( nm == 0 ) {
            *local = 1;
            return;
          }

        }
        else if ( strcmp( argv[n], global_str20 ) == 0 ) {
          n++;
          if ( n >= argc ) {
            *local = 1;
            return;
          }
        }
        else if ( strcmp( argv[n], global_str21 ) == 0 ) {
          *appendDisplay = 0;
          n++;
          if ( n >= argc ) {
            *local = 1;
            return;
          }
          strncpy( displayName, argv[n], 63 );
        }
        else if ( strcmp( argv[n], global_str22 ) == 0 ) {
          n++;
          if ( n >= argc ) { // missing user library name
            *local = 1;
            return;
          }
        }
        else if ( strcmp( argv[n], global_str73 ) == 0 ) {
          n++;
          if ( n >= argc ) { // missing port num
            *local = 1;
            return;
          }
          *portNum = atol( argv[n] );
        }
        else if ( strcmp( argv[n], global_str76 ) == 0 ) {
	}
        else if ( strcmp( argv[n], global_str79 ) == 0 ) {
	}
        else if ( strcmp( argv[n], global_str96 ) == 0 ) {
	}
        else if ( strcmp( argv[n], global_str98 ) == 0 ) { //readonly
          setReadOnly();
	}
        else if ( strcmp( argv[n], global_str100 ) == 0 ) {
	}
        else if ( strcmp( argv[n], global_str106 ) == 0 ) { //noautomsg
	}
        else {
          *local = 1;
          return;
        }

        n++;

      }
      else {

        state = FILES;

      }

      break;

    case FILES:

      if ( argv[n][0] == '-' ) {

        *local = 1;
        return;

      }
      else {

        n++;

      }

      break;

    }

  }

  if ( strcmp( displayName, "" ) == 0 ) {

    strncpy( displayName, g_defaultDisplay, 63 );
    displayName[63] = 0;

  }

  return;

}

extern int main (
  int argc,
  char **argv )
{

int i, j, stat, numAppsRemaining, exitProg, shutdown, q_stat_r, q_stat_i,
 local, server, portNum, restart, n, x, y, icon, sessionNoEdit, screenNoEdit,
 oneInstance, openCmd, convertOnly, crawl, verbose, needConnect;
int needToRebuildDisplayList = 0;
THREAD_HANDLE delayH, serverH; //, caPendH;
argsPtr args;
appListPtr cur, next, appArgsHead, newOne, first;
processClass proc;
objBindingClass *obj;
pvBindingClass *pvObj;
char *tk, *buf1;
MAIN_NODE_PTR node;
char **argArray, displayName[63+1];
int appendDisplay;
float hours, seconds;

char checkPointFileName[255+1], screenName[255+1], tmpMsg[255+1];

Display *oneDisplay;
XtAppContext oneAppCtx;

FILE *f = NULL;
int primaryServerFlag, oneInstanceFlag, numCheckPointMacros;
 char checkPointMacros[1023+1];

char *envPtr;
int doXSync = 0;

appListPtr primary=NULL;
int primaryServerWantsExit;

int numLocaleFailures = 0;

int shutdownTry = 200; // aprox 10 seconds

  XSetErrorHandler( xErrorHandler );
  //XSetIOErrorHandler( xIoErrorHandler );

  if ( diagnosticMode() ) {
    logDiagnostic( "edm started\n" );
  }

  envPtr = getenv("DISPLAY");
  if ( envPtr ) {
    strncpy( g_defaultDisplay, envPtr, 63 );
    g_defaultDisplay[63] = 0;
  }
  else {
    strcpy( g_defaultDisplay, ":0.0" );
  }

  do {

    if ( numLocaleFailures == 0 ) {

      if ( setlocale( LC_ALL, "C" ) == NULL ) {
        if ( setlocale( LC_ALL, "" ) == NULL ) {
          fprintf( stderr, "Cannot set locale - abort\n" );
          exit(1);
        }
      }

    }
    else if ( numLocaleFailures == 1 ) {

      if ( setlocale( LC_ALL, "C" ) == NULL ) {
        fprintf( stderr, "Cannot set locale - abort\n" );
        exit(1);
      }

    }

    if ( !XSupportsLocale() ) {
      numLocaleFailures++;
      if ( numLocaleFailures > 1 ) {
        fprintf( stderr, "X does not support locale \"%s\" - abort\n",
         setlocale( LC_ALL, NULL ) );
        exit(1);
      }
    }
    else {
      numLocaleFailures = 0;
    }

  } while ( numLocaleFailures );

  if ( XSetLocaleModifiers( "" ) == NULL ) {
    fprintf( stderr, "Cannot set locale modifiers - abort\n" );
    exit(1);
  }

  // fprintf( stderr, "locale is \"%s\"\n", setlocale( LC_ALL, NULL ) );

  envPtr = getenv( "EDMXSYNC" );
  if ( envPtr ) doXSync = 1;

  g_numClients = 1;

  checkParams( argc, argv, &local, &server, &appendDisplay, displayName,
   &portNum, &restart, &oneInstance, &openCmd, &convertOnly, &crawl,
   &verbose );

  if ( debugMode() ) {
    fprintf( stderr, "server = %-d\n", server );
    fprintf( stderr, "oneInstanceFlag = %-d\n", oneInstance );
    fprintf( stderr, "displayName = [%s]\n", displayName );
    fprintf( stderr, "portNum = %-d\n", portNum );
  }

  // if doing a restart, read in check point file
  if ( restart ) {

    //fprintf( stderr, "restart\n" );

    getCheckPointFileName( checkPointFileName, g_restartId );
    f = fopen( checkPointFileName, "r" );
    if ( f ) {

      stat = getServerCheckPointParams( f, &primaryServerFlag, &oneInstanceFlag,
       displayName, &sessionNoEdit, &numCheckPointMacros, checkPointMacros );
      if ( !( stat & 1 ) ) { // couldn't read file

        fclose( f );
        f = fopen( checkPointFileName, "r" );
        if ( !f ) restart = 0;
        if ( f ) {

          stat = getMainCheckPointParams( f, &primaryServerFlag, &oneInstanceFlag,
           displayName, &sessionNoEdit, &numCheckPointMacros, checkPointMacros );
          if ( !( stat & 1 ) ) { // couldn't read file
            restart = 0;
          }

	}

      }

      if ( primaryServerFlag == 2 ) {
        server = 1;
        appendDisplay = 1;
        local = 0;
      }
      else {
        server = 0;
        local = 1;
      }

      if ( oneInstanceFlag ) {
        oneInstance = 1;
        openCmd = 0;
        server = 1;
        appendDisplay = 1;
        local = 0;
      }

      //fprintf( stderr, "primaryServerFlag = %-d\n", primaryServerFlag );
      //fprintf( stderr, "oneInstanceFlag = %-d\n", oneInstanceFlag );
      //fprintf( stderr, "server = %-d\n", server );
      //fprintf( stderr, "displayName = [%s]\n", displayName );
      //fprintf( stderr, "sessionNoEdit = %-d\n", sessionNoEdit );
      //fprintf( stderr, "numCheckPointMacros = %-d\n", numCheckPointMacros );
      //fprintf( stderr, "checkPointMacros = [%s]\n", checkPointMacros );

    }
    else {

      restart = 0;

    }

  }

  if ( server ) {

    checkForServer( argc, argv, portNum, appendDisplay, displayName,
     oneInstance, openCmd );

  }

  if ( server ) {

    // If openCmd is > 0, we want the server to take some action;
    // if no server is running, we do not want to launch an instance of edm
    if ( openCmd ) {
      fprintf( stderr, main_str46 );
      exit(100);
    }

    stat = sys_iniq( &g_mainFreeQueue );
    if ( !( stat & 1 ) ) {
      fprintf( stderr, main_str37 );
      exit(1);
    }
    stat = sys_iniq( &g_mainActiveQueue );
    if ( !( stat & 1 ) ) {
      fprintf( stderr, main_str38 );
      exit(1);
    }

    g_mainFreeQueue.flink = NULL;
    g_mainFreeQueue.blink = NULL;
    g_mainActiveQueue.flink = NULL;
    g_mainActiveQueue.blink = NULL;

    for ( i=0; i<MAIN_QUEUE_SIZE; i++ ) {

      stat = INSQTI( (void *) &g_mainNodes[i], (void *) &g_mainFreeQueue,
       0 );
      if ( !( stat & 1 ) ) {
        fprintf( stderr, main_str39 );
        exit(1);
      }

    }

  }

  appArgsHead = new appListType;
  appArgsHead->flink = appArgsHead;
  appArgsHead->blink = appArgsHead;

  //XSetErrorHandler( xErrorHandler );

  stat = thread_init();

  obj = new objBindingClass;
  pvObj = new pvBindingClass;

  stat = thread_create_handle( &serverH, (void *) &portNum );

  stat = thread_create_handle( &delayH, NULL );

  if ( server ) stat = thread_create_proc( serverH, serverThread );

#if 0
  stat = thread_create_handle( &caPendH, NULL );
  stat = thread_create_proc( caPendH, caPendThread );
#endif

  args = new argsType;

  if ( restart ) { // append display name and macros to args

    //fprintf( stderr, "adjust args for restart\n" );

    //fprintf( stderr, "argc = %-d\n", argc );

    n = 0;
    if ( !blank(displayName) ) n += 2;
    if ( numCheckPointMacros ) n += 2;
    if ( sessionNoEdit ) n++;
    n++; // add -x (execute)

    argArray = new char*[argc+n];

    i = 0;

    argArray[i] = new char[strlen(argv[i])+1];
    strcpy( argArray[i], argv[i] );
    i++;

    argArray[i] = new char[strlen("-x")+1];
    strcpy( argArray[i], "-x" );
    i++;

    if ( sessionNoEdit ) {

      argArray[i] = new char[strlen("-noedit")+1];
      strcpy( argArray[i], "-noedit" );
      i++;

    }

    if ( !blank(displayName) ) {

      argArray[i] = new char[strlen("-display")+1];
      strcpy( argArray[i], "-display" );
      i++;

      argArray[i] = new char[strlen(displayName)+1];
      strcpy( argArray[i], displayName );
      i++;

    }

    if ( numCheckPointMacros ) {

      argArray[i] = new char[strlen("-m")+1];
      strcpy( argArray[i], "-m" );
      i++;

      argArray[i] = new char[strlen(checkPointMacros)+1];
      strcpy( argArray[i], checkPointMacros );
      i++;

    }

    for ( j=1; j<argc; j++ ) {
      argArray[i] = new char[strlen(argv[j])+1];
      strcpy( argArray[i], argv[j] );
      i++;
    }

    args->argc = argc + n;

  }
  else {

    argArray = new char*[argc];
    for ( i=0; i<argc; i++ ) {
      argArray[i] = new char[strlen(argv[i])+1];
      strcpy( argArray[i], argv[i] );
    }

    args->argc = argc;

  }

  args->argv = argArray;

  cur = new appListType;
  cur->appArgs = args;

  cur->blink = appArgsHead->blink;
  appArgsHead->blink->flink = cur;
  cur->flink = appArgsHead;
  appArgsHead->blink = cur;

  args->appCtxPtr = new appContextClass;
  args->appCtxPtr->proc = &proc;

  //fprintf( stderr, "argc = %-d\n", args->argc );
  //for ( i=0; i<args->argc; i++ ) {
  //  fprintf( stderr, "argv[%-d] = [%s]\n", i, args->argv[i] );
  //}

  if ( server ) {
    addDisplayToList( args->argc, args->argv );
    stat = args->appCtxPtr->startApplication( args->argc, args->argv, 2,
     oneInstance, convertOnly );
  }
  else {
    stat = args->appCtxPtr->startApplication( args->argc, args->argv, 1,
     oneInstance, convertOnly );
  }
  if ( !( stat & 1 ) ) exit( 1 );

  if ( stat & 1 ) { // success
    oneAppCtx = args->appCtxPtr->appContext();
    XtAppSetErrorHandler( oneAppCtx, xtErrorHandler );
    XtAppSetWarningHandler( oneAppCtx, xtErrorHandler );
  }

  if ( crawl ) {

    {

      macroListPtr cur;
      fileListPtr curFile;
      crawlListPtr crawlList;

      int i, l, numMacros, found;
      char *fname;
      char **symbols;
      char **values;
      char crawlFileName[255+1];

      setCrawlVerbose( verbose );

      args->appCtxPtr->useStdErr( 1 );
      args->appCtxPtr->setErrMsgPrefix( "displayCrawlerStatus: " );

      numMacros = 0;
      cur = args->appCtxPtr->macroHead->flink;
      while ( cur != args->appCtxPtr->macroHead ) {
        numMacros++;
        cur = cur->flink;
      }

      symbols = new char*[numMacros];
      values = new char*[numMacros];

      i = 0;
      cur = args->appCtxPtr->macroHead->flink;
      while ( cur != args->appCtxPtr->macroHead ) {

        //fprintf( stderr, "[%s] = [%s]\n", cur->macro, cur->expansion );

	l = strlen( cur->macro );
        symbols[i] = new char[l+1];
	strcpy( symbols[i], cur->macro );

	l = strlen( cur->expansion );
        values[i] = new char[l+1];
        strcpy( values[i], cur->expansion );

	i++;
        cur = cur->flink;

      }

      curFile = args->appCtxPtr->fileHead->flink;
      while ( curFile != args->appCtxPtr->fileHead ) {

        getFirstFile( curFile->file, 255, crawlFileName, &found );
        if ( found ) {

          while ( found ) {

	    //fprintf( stderr, "file = [%s]\n", crawlFileName );

            fname = new char[strlen(crawlFileName)+1];
            strcpy( fname, crawlFileName );

            initCrawlList( &crawlList );

            setCrawlListBaseMacros( numMacros, symbols, values );

            addCrawlNode( crawlList, fname, numMacros,
             symbols, values );

            stat = crawlEdlFiles( args->appCtxPtr, crawlList );

            getNextFile( curFile->file, 255, crawlFileName, &found );

	  }

	}
	else {

          //fprintf( stderr, "one file = [%s]\n", curFile->file );

          initCrawlList( &crawlList );

          setCrawlListBaseMacros( numMacros, symbols, values );

          addCrawlNode( crawlList, curFile->file, numMacros,
           symbols, values );

          stat = crawlEdlFiles( args->appCtxPtr, crawlList );

	}

        curFile = curFile->flink;

      }

      stat = displayCrawlerResults();

    }

    exit( 0 );

  }

  if ( restart ) { // open all displays

    n = getNumCheckPointScreens( f );

    //fprintf( stderr, "%-d screen(s)\n", n );

    for ( i=0; i<n; i++ ) {

      stat = getScreenCheckPointParams( f, screenName, &x, &y, &icon,
       &screenNoEdit, &numCheckPointMacros, checkPointMacros );

      if ( stat & 1 ) {
        stat = args->appCtxPtr->openCheckPointScreen( screenName, x, y, icon,
         screenNoEdit, numCheckPointMacros, checkPointMacros );
      }

    }

    fclose( f );
    f = fopen( checkPointFileName, "r" );
    if ( !f ) restart = 0;

  }

  if ( restart ) {

    // now, get all client display checkpoint info
    do {

      stat = getMainCheckPointParams( f, &primaryServerFlag, &oneInstanceFlag,
       displayName, &sessionNoEdit, &numCheckPointMacros, checkPointMacros );
      if ( !( stat & 1 ) ) { // couldn't read file
        break;
      }

      if ( stat != 3 ) { // end of data

        //fprintf( stderr, "primaryServerFlag = %-d\n", primaryServerFlag );
        //fprintf( stderr, "oneInstanceFlag = %-d\n", oneInstanceFlag );
        //fprintf( stderr, "server = %-d\n", server );
        //fprintf( stderr, "displayName = [%s]\n", displayName );
        //fprintf( stderr, "sessionNoEdit = %-d\n", sessionNoEdit );
        //fprintf( stderr, "numCheckPointMacros = %-d\n", numCheckPointMacros );
        //fprintf( stderr, "checkPointMacros = [%s]\n", checkPointMacros );

        n = 0;
        if ( !blank(displayName) ) n += 2;
        if ( numCheckPointMacros ) n += 2;
        if ( sessionNoEdit ) n++;
        n++; // add -x (execute)

        argArray = new char*[argc+n];

        i = 0;

        argArray[i] = new char[strlen(argv[i])+1];
        strcpy( argArray[i], argv[i] );
        i++;

        argArray[i] = new char[strlen("-x")+1];
        strcpy( argArray[i], "-x" );
        i++;

        if ( sessionNoEdit ) {

          argArray[i] = new char[strlen("-noedit")+1];
          strcpy( argArray[i], "-noedit" );
          i++;

        }

        if ( !blank(displayName) ) {

          argArray[i] = new char[strlen("-display")+1];
          strcpy( argArray[i], "-display" );
          i++;

          argArray[i] = new char[strlen(displayName)+1];
          strcpy( argArray[i], displayName );
          i++;

        }

        if ( numCheckPointMacros ) {

          argArray[i] = new char[strlen("-m")+1];
          strcpy( argArray[i], "-m" );
          i++;

          argArray[i] = new char[strlen(checkPointMacros)+1];
          strcpy( argArray[i], checkPointMacros );
          i++;

        }

        for ( j=1; j<argc; j++ ) {
          argArray[i] = new char[strlen(argv[j])+1];
          strcpy( argArray[i], argv[j] );
          i++;
        }

        args = new argsType;

        args->argc = argc + n;
        args->argv = argArray;

        newOne = new appListType;
        newOne->appArgs = args;

        args->appCtxPtr = new appContextClass;
        args->appCtxPtr->proc = &proc;

        stat = args->appCtxPtr->startApplication( args->argc, args->argv, 0,
         0, 0 );

        if ( stat & 1 ) { // success

          addDisplayToList( args->argc, args->argv );

          newOne->blink = appArgsHead->blink;
          appArgsHead->blink->flink = newOne;
          newOne->flink = appArgsHead;
          appArgsHead->blink = newOne;

          oneAppCtx = args->appCtxPtr->appContext();
          XtAppSetErrorHandler( oneAppCtx, xtErrorHandler );
          XtAppSetWarningHandler( oneAppCtx, xtErrorHandler );

          g_numClients++;

	}

        n = getNumCheckPointScreens( f );

        //fprintf( stderr, "%-d screen(s)\n", n );

        for ( i=0; i<n; i++ ) {

          stat = getScreenCheckPointParams( f, screenName, &x, &y, &icon,
           &screenNoEdit, &numCheckPointMacros, checkPointMacros );

          if ( stat & 1 ) {
            stat = args->appCtxPtr->openCheckPointScreen( screenName, x, y,
             icon, screenNoEdit, numCheckPointMacros, checkPointMacros );
          }

        }

      }

    } while ( stat != 3 );

  }

  if ( f ) {
    fclose( f );
    if ( g_pidNum != 0 ) {
      unlink( checkPointFileName ); // delete checkpoint file
    }
  }

  proc.timeCount = 0;
  proc.cycleTimeFactor = 1.0;
  proc.halfSecCount = 5;
  stat = sys_get_time( &proc.tim0 );
  stat = thread_init_timer( delayH, 0.1 );
  exitProg = 0;
  shutdown = 0;
  primaryServerWantsExit = 0;

  while ( !exitProg ) {

#ifdef DIAGNOSTIC_ALLOC

    if ( z ) z--;
    if ( z == 1 ) {
      //args->appCtxPtr->xSynchronize( 1 );
      memTrackOn();
      zz = 200;
    }

    if ( zz ) zz--;
    if ( zz == 1 ) {
      //zz = 200;
      fprintf( stderr, "reset\n" );
      memTrackReset();
    }

    showMem();
    fprintf( stderr, "[%-d]\n", zz );

#endif

    if ( needToRebuildDisplayList ) {

      stat = thread_lock_master( serverH );

      needToRebuildDisplayList = 0;
      g_displayIndex = 0;
      //fprintf( stderr, "\n\nRebuild display list\n" );
      cur = appArgsHead->flink;
      while ( cur != appArgsHead ) {
        next = cur->flink;
        if ( strcmp( cur->appArgs->appCtxPtr->displayName, "" ) != 0 ) {
          //fprintf( stderr, "dsp = [%s]\n",
          // cur->appArgs->appCtxPtr->displayName );
          addDisplayToListByName( cur->appArgs->appCtxPtr->displayName );
	}
        cur = next;
      }
      //fprintf( stderr, "\n" );

      // if none found, add default display
      addDisplayToListByName( g_defaultDisplay );

      //fprintf( stderr, "Current list:\n" );
      //for ( i=0; i<g_displayIndex; i++ ) {
      //  fprintf( stderr, "  dsp = [%s]\n", g_displayNames[i] );
      //}

      //cur = appArgsHead->flink;
      //while ( cur != appArgsHead ) {
      //  next = cur->flink;
      //  if ( strcmp( cur->appArgs->appCtxPtr->displayName, "" ) != 0 ) {
      //    fprintf( stderr, "  dsp = [%s]\n",
      //     cur->appArgs->appCtxPtr->displayName );
      //}
      //  cur = next;
      //}
      //fprintf( stderr, "\n" );

      stat = thread_unlock_master( serverH );

    }

    numAppsRemaining = 0;
    cur = appArgsHead->flink;
    while ( cur != appArgsHead ) {

      if ( doXSync ) {
        if ( cur->appArgs->appCtxPtr->syncOnce ) {
          cur->appArgs->appCtxPtr->syncOnce = 0;
          cur->appArgs->appCtxPtr->xSynchronize( 1 );
        }
      }

      next = cur->flink; // cur might get deleted

      cur->appArgs->appCtxPtr->applicationLoop();

      if ( cur->appArgs->appCtxPtr->shutdownFlag ) {
        if ( !shutdown ) {
          getCheckPointFileNamefromPID( checkPointFileName );
          f = fopen( checkPointFileName, "w" );
	}
        shutdown = 1;
      }

      if ( cur->appArgs->appCtxPtr->exitFlag &&
           !cur->appArgs->appCtxPtr->objDelFlag ) {

        if ( !cur->appArgs->appCtxPtr->okToExit() ) {
          cur->appArgs->appCtxPtr->postMessage( main_str45 );
          cur->appArgs->appCtxPtr->exitFlag = 0;
	}

      }

      if ( cur->appArgs->appCtxPtr->exitFlag &&
           !cur->appArgs->appCtxPtr->objDelFlag ) {

        cur->appArgs->appCtxPtr->objDelFlag = 5;

        numAppsRemaining++;

        if ( cur->appArgs->appCtxPtr->primaryServer != 2 ) {
  
          cur->appArgs->appCtxPtr->closeDownAppCtx();

	  if ( cur->appArgs->appCtxPtr->displayName ) {
	    needToRebuildDisplayList = 1;
	  }

	  // blank display name
	  strcpy( cur->appArgs->appCtxPtr->displayName, "" );

          stat = thread_lock_master( serverH );
          g_numClients--;
          stat = thread_unlock_master( serverH );

        }

      }
      else if ( cur->appArgs->appCtxPtr->exitFlag &&
       cur->appArgs->appCtxPtr->objDelFlag > 1 ) {

        //fprintf( stderr, "decrement\n" );

        (cur->appArgs->appCtxPtr->objDelFlag)--;

        numAppsRemaining++;

      }
      else if ( cur->appArgs->appCtxPtr->exitFlag &&
       cur->appArgs->appCtxPtr->objDelFlag == 1 ) {

	if ( cur->appArgs->appCtxPtr->primaryServer == 2 ) {

          primaryServerWantsExit = 1;
          primary = cur;
          numAppsRemaining++;

	}
	else {

          //fprintf( stderr, "delete\n" );

          // unlink and delete
          cur->blink->flink = cur->flink;
          cur->flink->blink = cur->blink;

          oneAppCtx = cur->appArgs->appCtxPtr->appContext();
          oneDisplay = cur->appArgs->appCtxPtr->getDisplay();
          delete cur->appArgs->appCtxPtr;
          for ( i=0; i<cur->appArgs->argc; i++ )
            delete[] cur->appArgs->argv[i];
          delete[] cur->appArgs->argv;
          delete cur->appArgs;

          delete cur;

          // Can't execute the next two lines; program crashes. Have to live
          // with memory leak. This only applies to the case where one server
          // is managing multiple app ctx's / displays.
          //XtCloseDisplay( oneDisplay );
          //XtDestroyApplicationContext( oneAppCtx );

	}

      }
      else if ( shutdown ) {
        cur->appArgs->appCtxPtr->performShutdown( f );
        numAppsRemaining++;
      }
      else {
        numAppsRemaining++;
      }

      cur = next;

      needConnect = 0;
      if ( server ) {

        do {

          stat = thread_lock_master( serverH );

          q_stat_r = REMQHI( (void *) &g_mainActiveQueue, (void **) &node, 0 );

          if ( q_stat_r & 1 ) {

	    //fprintf( stderr, "START\n" );

            strncpy( tmpMsg, node->msg, 255 );
            tmpMsg[255] = 0;

	    //fprintf( stderr, "tmpMsg = [%s]\n", tmpMsg );

            buf1 = NULL;
            tk = strtok_r( tmpMsg, "|", &buf1 );
            if ( !tk ) goto parse_error;

	    //fprintf( stderr, "tk = [%s]\n", tk );

	    if ( strcmp( tk, "*OPN*" ) == 0 ) {

	      //fprintf( stderr, "OPN\n" );

              needConnect = 1;
              tk = strtok_r( NULL, "|", &buf1 ); // should contain display name

	      // make 1st app ctx open/deiconify/raise initial files
	      // and deiconify/raise main window so things look like
	      // a new instance of edm is starting
              first = appArgsHead->flink;
              while ( first != appArgsHead ) {
		if ( ( strcmp( tk, g_defaultDisplay ) == 0 ) ||
                     ( strcmp( tk,
                        first->appArgs->appCtxPtr->displayName ) == 0 ) ) {
                  tk = strtok_r( NULL, "|", &buf1 );
                  first->appArgs->appCtxPtr->openFiles( node->msg );
                  needConnect = 0;
                  break;
		}
                first = first->flink;
	      }

	    }
	    else if ( strcmp( tk, "*CTL*" ) == 0 ) {

	      //fprintf( stderr, "CTL\n" );

              needConnect = 0;
              tk = strtok_r( NULL, "|", &buf1 ); // should contain display name

	      // make 1st app ctx open/deiconify/raise initial files
	      // and deiconify/raise main window so things look like
	      // a new instance of edm is starting
              first = appArgsHead->flink;
              while ( first != appArgsHead ) {
		if ( ( strcmp( tk, g_defaultDisplay ) == 0 ) ||
                     ( strcmp( tk,
                        first->appArgs->appCtxPtr->displayName ) == 0 ) ) {
                  tk = strtok_r( NULL, "|", &buf1 );
                  first->appArgs->appCtxPtr->controlWinNames( "*CTL*", node->msg );
                  break;
		}
                first = first->flink;
	      }

	    }
	    else if ( strcmp( tk, "*PUT*" ) == 0 ) {

	      //fprintf( stderr, "PUT\n" );

              needConnect = 0;

	      int more = 1;
	      while ( more ) {
                tk = strtok_r( NULL, "|", &buf1 ); // dsp
                //if ( tk ) fprintf( stderr, "tk = %s\n", tk );
                if ( strcmp( tk, global_str146 ) == 0 ) break; // -put
                if ( !tk ) more = 0;
              }

	      if ( more ) {
                tk = strtok_r( NULL, "|,", &buf1 ); // put cmd
		while ( tk ) {
                  //fprintf( stderr, "cmd = %s\n", tk );
                  doCmdPut( tk );
                  tk = strtok_r( NULL, "|,", &buf1 ); // put cmd
		}
	      }

	    }
	    else if ( strcmp( tk, "*OIS*" ) == 0 ) {

	      //fprintf( stderr, "OIS\n" );

              needConnect = 1;
              tk = strtok_r( NULL, "|", &buf1 ); // should contain display name

	      // make 1st app ctx open/deiconify/raise initial files
	      // and deiconify/raise main window so things look like
	      // a new instance of edm is starting
              first = appArgsHead->flink;
              while ( first != appArgsHead ) {
		if ( ( strcmp( tk, g_defaultDisplay ) == 0 ) ||
                     ( strcmp( tk,
                        first->appArgs->appCtxPtr->displayName ) == 0 ) ) {
                  first->appArgs->appCtxPtr->openInitialFiles();
                  first->appArgs->appCtxPtr->findTop();
                  needConnect = 0;
                  break;
		}
                first = first->flink;
	      }

	    }

            strncpy( tmpMsg, node->msg, 255 );
            tmpMsg[255] = 0;
            buf1 = NULL;
            tk = strtok_r( tmpMsg, "|", &buf1 );
            if ( !tk ) goto parse_error;

	    if ( ( ( strcmp( tk, "*OIS*" ) != 0 ) &&
                   ( strcmp( tk, "*OPN*" ) != 0 ) ) ||
                 needConnect ) {

              args = new argsType;

	      if ( needConnect ) {
                strncpy( tmpMsg, node->msg, 255 );
                tmpMsg[255] = 0;
                buf1 = NULL;
                tk = strtok_r( tmpMsg, "|", &buf1 ); // discard two
		//if ( tk ) fprintf( stderr, "1 discard [%s]\n", tk );
                if ( !tk ) goto parse_error;
                tk = strtok_r( NULL, "|", &buf1 );
		//if ( tk ) fprintf( stderr, "2 discard [%s]\n", tk );
                if ( !tk ) goto parse_error;
                tk = strtok_r( NULL, "|", &buf1 );
		//if ( tk ) fprintf( stderr, "3 tk = [%s]\n", tk );
                if ( !tk ) goto parse_error;
	      }
	      else {
                strncpy( tmpMsg, node->msg, 255 );
                tmpMsg[255] = 0;
                buf1 = NULL;
                tk = strtok_r( tmpMsg, "|", &buf1 );
		//if ( tk ) fprintf( stderr, "4 tk = [%s]\n", tk );
                if ( !tk ) goto parse_error;
	      }

              argc = (int) atol( tk );
              if ( argc == 0 ) goto parse_error;

              argArray = new char*[argc];

              for ( i=0; i<argc; i++ ) {
                tk = strtok_r( NULL, "|", &buf1 );
                if ( !tk ) goto parse_error;
                argArray[i] = new char[strlen(tk)+1];
                strcpy( argArray[i], tk );
              }

              args->argc = argc;
              args->argv = argArray;

              newOne = new appListType;
              newOne->appArgs = args;

              args->appCtxPtr = new appContextClass;
              args->appCtxPtr->proc = &proc;

              stat = args->appCtxPtr->startApplication( args->argc, args->argv,
               0, 0, 0 );

              if ( stat & 1 ) { // success

                addDisplayToList( args->argc, args->argv );

                newOne->blink = appArgsHead->blink;
                appArgsHead->blink->flink = newOne;
                newOne->flink = appArgsHead;
                appArgsHead->blink = newOne;

                oneAppCtx = args->appCtxPtr->appContext();
                XtAppSetErrorHandler( oneAppCtx, xtErrorHandler );
                XtAppSetWarningHandler( oneAppCtx, xtErrorHandler );

                g_numClients++;

	      }
	      else {

		// delete ?

	      }

	    }

parse_error:

            q_stat_i = INSQTI( (void *) node, (void *) &g_mainFreeQueue, 0 );
            if ( !( q_stat_i & 1 ) ) {
              fprintf( stderr, main_str40 );
            }

          }
          else if ( q_stat_r != QUEWASEMP ) {
            fprintf( stderr, main_str41 );
          }

          stat = thread_unlock_master( serverH );

        } while ( q_stat_r & 1 );

      }

    }

    // See if primary server should exit
    if ( primaryServerWantsExit ) {

      if ( numAppsRemaining == 1 ) {

        cur = primary;

        stat = thread_lock_master( serverH );
        g_numClients--;
        stat = thread_unlock_master( serverH );

        if ( cur ) {

          cur->appArgs->appCtxPtr->closeDownAppCtx();

	  // blank display name
	  strcpy( cur->appArgs->appCtxPtr->displayName, "" );

          // unlink and delete
          cur->blink->flink = cur->flink;
          cur->flink->blink = cur->blink;

          oneAppCtx = cur->appArgs->appCtxPtr->appContext();
          oneDisplay = cur->appArgs->appCtxPtr->getDisplay();
          delete cur->appArgs->appCtxPtr;
          for ( i=0; i<cur->appArgs->argc; i++ )
            delete[] cur->appArgs->argv[i];
          delete[] cur->appArgs->argv;
          delete cur->appArgs;

          delete cur;

	}

        numAppsRemaining = 0;

      }
      else {

        shutdownTry--;

        if ( !shutdown || ( shutdownTry <= 0 ) ) {

          cur = primary;
	  if ( cur ) {
            cur->appArgs->appCtxPtr->postMessage( main_str47 );
            cur->appArgs->appCtxPtr->exitFlag = 0;
	  }
          primaryServerWantsExit = 0;
          shutdownTry = 200;

	}

      }

    }

    if ( !numAppsRemaining ) exitProg = 1;

    pend_event( 0.05 );

    proc.timeCount++;
    if ( proc.timeCount >= 100 ) { // 10 sec

      stat = sys_get_time( &proc.tim1 );
      stat = sys_get_time_diff_in_hours( &proc.tim0, &proc.tim1, &hours );
      proc.timeCount = 0;
      proc.tim0 = proc.tim1;
      seconds = hours * 3600.0;
      if ( seconds > 0.1 )
        proc.cycleTimeFactor = 10.0 / seconds;
      else
        proc.cycleTimeFactor = 1.0;
      proc.halfSecCount = (int) ceil( (double) 5.0 * proc.cycleTimeFactor );

      // process thread delete-request queue
      thread_cleanup_from_main_thread_only();

    }

  }

  if ( server ) {
    // ?
  }

  if ( shutdown ) {
    fprintf( f, "<<<EOD>>>\n" );
    fclose( f );
  }

  delete obj;
  obj = NULL;

  delete pvObj;
  pvObj = NULL;

#if 0
  stat = thread_destroy_handle( serverH );
  serverH = NULL;

  stat = thread_destroy_handle( delayH );
  delayH = NULL;

  if ( server ) {

    stat = sys_destroyq( &g_mainFreeQueue );

    stat = sys_destroyq( &g_mainActiveQueue );

  }
#endif

  if ( diagnosticMode() ) {
    fprintf( stderr, "edm terminated\n" );
    logDiagnostic( "edm terminated\n" );
  }

  return 0;

}
