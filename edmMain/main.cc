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

#include "sys_types.h"
#include "ipnsv.h"
#include "ipncl.h"
#include "thread.h"

#include "main.str"

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

#define MAIN_QUEUE_SIZE 10

#define DONE -1
#define SWITCHES 1
#define FILES 2

#define QUERY_LOAD 1
#define CONNECT 2
#define OPEN_INITIAL 3
#define OPEN 4

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
static IPRPC_PORT g_conPort;
static int g_numClients = 0;
static int g_pidNum = 0;
static char g_restartId[31+1];

//#include "alloc.h"
#ifdef DIAGNOSTIC_ALLOC
int z=2, zz=0;
#endif


#define MAX_X_ERRORS 100

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

  num++;

  if ( err->error_code != BadAccess ) {
    XGetErrorText( d, err->error_code, msg, 80 );
    fprintf( stderr, main_str1, err->error_code, msg );
  }

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
  //printf( "[%s]\n", checkPointFileName );

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
  //printf( "%-d, [%s]\n", g_pidNum, checkPointFileName );

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
int i, tmp;

  *server = 0;
  *oneInstance = 0;
  strcpy( displayName, "" );
  *numCheckPointMacros = 0;
  strcpy( checkPointMacros, "" );

  cptr = fgets( text, 1023, f );
  text[1024] = 0;
  if ( !cptr ) return 2; // fail
  if ( strcmp( text, "<<<EOD>>>\n" ) == 0 ) return 3; // no more data
  tmp = atol( text );
  *server = tmp & 0xf;
  *oneInstance = ( tmp >> 8 );

  readStringFromFile( displayName, 127, f );

  cptr = fgets( text, 1023, f );
  text[1024] = 0;
  if ( !cptr ) return 2; // fail
  *noEdit = atol( text );

  cptr = fgets( text, 1023, f );
  text[1024] = 0;
  if ( !cptr ) return 2; // fail
  *numCheckPointMacros = atol( text );

  for ( i=0; i<*numCheckPointMacros; i++ ) {
    cptr = fgets( text, 1023, f );
    text[1024] = 0;
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

static int getNumCheckPointScreens (
  FILE *f
) {

char text[32], *cptr;
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
  text[1024] = 0;
  if ( !cptr ) return 2; // fail
  *x = atol( text );

  cptr = fgets( text, 1023, f );
  text[1024] = 0;
  if ( !cptr ) return 2; // fail
  *y = atol( text );

  cptr = fgets( text, 1023, f );
  text[1024] = 0;
  if ( !cptr ) return 2; // fail
  *icon = atol( text );

  cptr = fgets( text, 1023, f );
  text[1024] = 0;
  if ( !cptr ) return 2; // fail
  *noEdit = atol( text );

  cptr = fgets( text, 1023, f );
  text[1024] = 0;
  if ( !cptr ) return 2; // fail
  *numCheckPointMacros = atol( text );

  for ( i=0; i<*numCheckPointMacros; i++ ) {
    cptr = fgets( text, 1023, f );
    text[1024] = 0;
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

int getHostAddr (
  char *name,
  char *addr )
{

struct hostent *entry;
unsigned char *adr;
char *tk, *buf;

  strcpy( addr, "0.0.0.0" );

  tk = strtok_r( name, ":,", &buf );
  if ( !tk ) tk = name;

  entry = gethostbyname( tk );
  if ( entry->h_length != 4 ) return -1;

  adr = (unsigned char *) (entry->h_addr_list)[0];
  sprintf( addr, "%-d.%-d.%-d.%-d", adr[0], adr[1], adr[2], adr[3] );

  return 0;

}

void checkForServer (
  int argc,
  char **argv,
  int portNum,
  int appendDisplay,
  char *displayName,
  int oneInstance,
  int openCmd )
{

char chkHost[31+1], host[31+1], addr[31+1];
int i, len, pos, max, argCount, stat, result, item, useItem;
IPRPC_PORT port;
char msg[255+1], portNumStr[15+1];
SYS_TIME_TYPE timeout;
char *envPtr, *tk1, *tk2, *buf1, *buf2;
double merit, min, num;

  envPtr = getenv( "EDMSERVERS" );

  if ( !envPtr ) {
    stat = gethostname( host, 31 );
    if ( stat ) return;
    envPtr = host;
  }

  sprintf( portNumStr, "%-d", portNum );

  if ( appendDisplay ) {
    argCount = argc + 2; // we will add two parameters below
  }
  else {
    argCount = argc;
  }

  stat = sys_cvt_seconds_to_timeout( 10.0, &timeout );
  if ( !( stat & 1 ) ) {
    printf( main_str3 );
    return;
  }

  // do simple load balancing

  tk1 = strtok_r( envPtr, ",", &buf1 );
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

    //printf( "Checking host [%s], merit = %-f\n", chkHost, merit );

    stat = getHostAddr( chkHost, addr );
    if ( stat ) return;

    stat = ipncl_create_port( 1, 4096, "edm", &port );
    if ( !( stat & 1 ) ) {
      printf( main_str4 );
      return;
    }

    stat = ipncl_connect( addr, portNumStr, "", port );
    if ( !( stat & 1 ) ) {
      return;
    }

    msg[0] = (char) QUERY_LOAD;
    msg[1] = 0;

    len = strlen(msg) + 1;

    stat = ipncl_send_msg( port, len, msg );
    if ( !( stat & 1 ) ) {
      printf( main_str5 );
      return;
    }

    stat = ipncl_wait_on_port( port, &timeout, &result );
    if ( !( stat & 1 ) ) {
      printf( main_str42 );
      stat = ipncl_disconnect( port );
      stat = ipncl_delete_port( &port );
      goto nextHost;
    }

    if ( !result ) {

      // timeout
      stat = ipncl_disconnect( port );
      stat = ipncl_delete_port( &port );
      stat = ipncl_disconnect( g_conPort );
      goto nextHost;

    }
    else {

      stat = ipncl_receive_msg( port, 255, &len, msg );
      if ( !( stat & 1 ) ) {
        printf( main_str43 );
        stat = ipncl_disconnect( port );
        stat = ipncl_delete_port( &port );
        goto nextHost;
      }

      num = (double) ntohl( *( (int *) msg ) );

      //printf( "received num = %-f\n", num );

      num = num / merit;

      if ( ( num < min ) || ( min == -1 ) ) {
        min = num;
        strncpy( host, chkHost, 31 );
        host[31] = 0;
        useItem = item;
      }

      //printf( "min = %-f, adj num = %-f\n", min, num );

    }

    // don't check status, we're probably already disconnected
    stat = ipncl_disconnect( port );

    stat = ipncl_delete_port( &port );
    if ( !( stat & 1 ) ) {
      printf( main_str7 );
    }

nextHost:

    item++;
    tk1 = strtok_r( NULL, ",", &buf1 );

  }

  //printf( "Using host [%s], item %-d\n", host, useItem );

  stat = getHostAddr( host, addr );
  if ( stat ) return;

  stat = ipncl_create_port( 1, 4096, "edm", &port );
  if ( !( stat & 1 ) ) {
    printf( main_str4 );
    return;
  }

  stat = ipncl_connect( addr, portNumStr, "", port );
  if ( !( stat & 1 ) ) {
    return;
  }

  if ( oneInstance ) {

    if ( openCmd ) {

      msg[0] = (char) OPEN;
      pos = 1;
      max = 255 - pos;

      strncpy( &msg[pos], "*OPN*|", max );
      pos = strlen(msg);
      max = 255 - pos;

    }
    else {

      msg[0] = (char) OPEN_INITIAL;
      pos = 1;
      max = 255 - pos;

      strncpy( &msg[pos], "*OIS*|", max );
      pos = strlen(msg);
      max = 255 - pos;

    }

    strncpy( &msg[pos], displayName, max );
    pos = strlen(msg);
    max = 255 - pos;

    strncpy( &msg[pos], "|", max );
    pos = strlen(msg);
    max = 255 - pos;

    snprintf( &msg[pos], max, "%-d|", argCount );
    pos = strlen(msg);
    max = 255 - pos;

    strncpy( &msg[pos], "edm|", max );
    pos = strlen(msg);
    max = 255 - pos;

    if ( appendDisplay ) {
      strncpy( &msg[pos], global_str56, max );
      pos = strlen(msg);
      max = 255 - pos;
      strncpy( &msg[pos], displayName, max );
      pos = strlen(msg);
      max = 255 - pos;
      Strncat( &msg[pos], "|", max );
      pos = strlen(msg);
      max = 255 - pos;
    }

    for ( i=1; i<argc; i++ ) {
      strncpy( &msg[pos], argv[i], max );
      pos = strlen(msg);
      max = 255 - pos;
      Strncat( &msg[pos], "|", max );
      pos = strlen(msg);
      max = 255 - pos;
    }

  }
  else {

    msg[0] = (char) CONNECT;
    pos = 1;

    sprintf( &msg[pos], "%-d|", argCount );
    pos = strlen(msg);
    max = 255 - pos;

    strncpy( &msg[pos], "edm|", max );
    pos = strlen(msg);
    max = 255 - pos;

    if ( appendDisplay ) {
      strncpy( &msg[pos], global_str56, max );
      pos = strlen(msg);
      max = 255 - pos;
      strncpy( &msg[pos], displayName, max );
      pos = strlen(msg);
      max = 255 - pos;
      Strncat( &msg[pos], "|", max );
      pos = strlen(msg);
      max = 255 - pos;
    }

    for ( i=1; i<argc; i++ ) {
      strncpy( &msg[pos], argv[i], max );
      pos = strlen(msg);
      max = 255 - pos;
      Strncat( &msg[pos], "|", max );
      pos = strlen(msg);
      max = 255 - pos;
    }

  }

  len = strlen(msg) + 1;

  stat = ipncl_send_msg( port, len, msg );
  if ( !( stat & 1 ) ) {
    printf( main_str5 );
    return;
  }

  // don't check status, we're probably already disconnected
  stat = ipncl_disconnect( port );

  stat = ipncl_delete_port( &port );
  if ( !( stat & 1 ) ) {
    printf( main_str7 );
  }

  exit(0);

}

#ifdef __linux__
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

}

#ifdef __linux__
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

int stat, n, q_stat_r, q_stat_i, result;
THREAD_HANDLE delayH;
MAIN_NODE_PTR node;
IPRPC_PORT port;
SYS_TIME_TYPE timeout;
int len, num, cmd;
char msg[255+1], portNumStr[15+1];

int *portNumPtr = (int *) thread_get_app_data( h );

  stat = thread_create_handle( &delayH, NULL );

  stat = sys_cvt_seconds_to_timeout( 10.0, &timeout );
  if ( !( stat & 1 ) ) {
    printf( main_str3 );
    goto err_return;
  }

  sprintf( portNumStr, "%-d", *portNumPtr );

  stat = ipnsv_create_named_port( portNumStr, 1, 255, "edm", &g_conPort );
  if ( !( stat & 1 ) ) {
    printf( main_str8 );
    goto err_return;
  }

  n = 0;
  while ( 1 ) {

    stat = ipnsv_create_port( 1, 255, "edm data", &port );
    if ( !( stat & 1 ) ) {
      printf( main_str9 );
      goto err_return;
    }

    stat = ipnsv_accept_connection( g_conPort, port, " " );
    if ( !( stat & 1 ) ) {
      printf( main_str11 );
      goto err_return;
    }

    stat = ipnsv_wait_on_port( port, &timeout, &result );
    if ( !( stat & 1 ) ) {
      printf( main_str13 );
      stat = ipnsv_disconnect( port );
      stat = ipnsv_delete_port( &port );
      continue;
    }

    if ( !result ) {

      // timeout
      stat = ipnsv_disconnect( port );
      stat = ipnsv_delete_port( &port );
      stat = ipnsv_disconnect( g_conPort );
      continue;

    }
    else {

      stat = ipnsv_receive_msg( port, 255, &len, msg );
      if ( !( stat & 1 ) ) {
        printf( main_str15 );
        stat = ipnsv_disconnect( port );
        stat = ipnsv_delete_port( &port );
        continue;
      }

    }

    cmd = (int) msg[0];

    switch ( cmd ) {

    case OPEN_INITIAL:
    case OPEN:

      stat = thread_lock_master( h );

      q_stat_r = REMQHI( (void *) &g_mainFreeQueue, (void **) &node, 0 );
      if ( q_stat_r & 1 ) {
        strncpy( node->msg, &msg[1], 254 );
        q_stat_i = INSQTI( (void *) node, (void *) &g_mainActiveQueue, 0 );
        if ( !( q_stat_i & 1 ) ) {
          printf( main_str17 );
        }
      }
      else {
        printf( main_str18 );
      }

      stat = thread_unlock_master( h );

      break;

    case QUERY_LOAD:

      stat = thread_lock_master( h );
      num = g_numClients;
      stat = thread_unlock_master( h );

      *( (int *) msg ) = htonl( num );
      len = 4;

      stat = ipnsv_send_msg( port, len, msg );
      if ( !( stat & 1 ) ) {
        printf( main_str44 );
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
          printf( main_str17 );
        }
      }
      else {
        printf( main_str18 );
      }

      stat = thread_unlock_master( h );

      break;

    }

    stat = ipnsv_disconnect( port );
    stat = ipnsv_delete_port( &port );
    stat = ipnsv_disconnect( g_conPort );

    if ( cmd == CONNECT ) {
      thread_delay( delayH, 0.5 );
    }

  }

err_return:

  stat = 0;

#ifdef __linux__
  return NULL;
#endif

#ifdef __solaris__
  return NULL;
#endif

#ifdef HP_UX
  return NULL;
#endif

}

void checkParams (
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
  int *convertOnly )
{

char buf[1023+1], mac[1023+1], exp[1023+1];
int state = SWITCHES;
int stat, l, nm = 0, n = 1;
char *envPtr, *tk, *buf1;
Display *testDisplay;

  strcpy( displayName, "" );
  *local = 0;
  *server = 0;
  *oneInstance = 0;
  *openCmd = 0;
  *appendDisplay = 1;
  *portNum = 19000;
  *restart = 0;
  *convertOnly = 0;

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

        if ( strcmp( argv[n], global_str9 ) == 0 ) {
          *local = 1;
        }
	else if ( strcmp( argv[n], global_str91 ) == 0 ) {
          *convertOnly = 1;
	  *oneInstance = 0;
          *server = 0;
          *local = 1;
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
          strncpy( displayName, argv[n], 127 );
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

    envPtr = getenv("DISPLAY");
    if ( envPtr ) strncpy( displayName, envPtr, 31 );

    if ( strcmp( displayName, "" ) == 0 ) {

      stat = gethostname( displayName, 31 );
      if ( stat ) {
        printf( main_str35 );
        exit(0);
      }

      Strncat( displayName, ":0.0", 31 );

    }

  }

  testDisplay = XOpenDisplay( displayName );
  if ( !testDisplay ) {
    printf( main_str36 );
    exit(0);
  }

  XCloseDisplay( testDisplay );

  return;

}

extern int main (
  int argc,
  char **argv )
{

int i, j, stat, numAppsRemaining, exitProg, shutdown, q_stat_r, q_stat_i,
 local, server, portNum, restart, n, x, y, icon, sessionNoEdit, screenNoEdit,
 oneInstance, openCmd, convertOnly, needConnect;
THREAD_HANDLE delayH, serverH; //, caPendH;
argsPtr args;
appListPtr cur, next, appArgsHead, newOne, first;
processClass proc;
objBindingClass *obj;
pvBindingClass *pvObj;
char *tk, *buf1;
MAIN_NODE_PTR node;
char **argArray, displayName[127+1];
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

  envPtr = getenv( "EDMXSYNC" );
  if ( envPtr ) doXSync = 1;

  g_numClients = 1;

  checkParams( argc, argv, &local, &server, &appendDisplay, displayName,
   &portNum, &restart, &oneInstance, &openCmd, &convertOnly );

  // if doing a restart, read in check point file
  if ( restart ) {

    //printf( "restart\n" );

    getCheckPointFileName( checkPointFileName, g_restartId );
    f = fopen( checkPointFileName, "r" );
    if ( f ) {

      stat = getMainCheckPointParams( f, &primaryServerFlag, &oneInstanceFlag,
       displayName, &sessionNoEdit, &numCheckPointMacros, checkPointMacros );
      if ( !( stat & 1 ) ) { // couldn't read file
        restart = 0;
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

      //printf( "primaryServerFlag = %-d\n", primaryServerFlag );
      //printf( "oneInstanceFlag = %-d\n", oneInstanceFlag );
      //printf( "server = %-d\n", server );
      //printf( "displayName = [%s]\n", displayName );
      //printf( "sessionNoEdit = %-d\n", sessionNoEdit );
      //printf( "numCheckPointMacros = %-d\n", numCheckPointMacros );
      //printf( "checkPointMacros = [%s]\n", checkPointMacros );

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

    // If openCmd is true, we want the server to open some screens;
    // if no server is running, we do not want to launch an instance of edm
    if ( openCmd ) {
      printf( main_str46 );
      exit(0);
    }

    stat = sys_iniq( &g_mainFreeQueue );
    if ( !( stat & 1 ) ) {
      printf( main_str37 );
      exit(0);
    }
    stat = sys_iniq( &g_mainActiveQueue );
    if ( !( stat & 1 ) ) {
      printf( main_str38 );
      exit(0);
    }

    g_mainFreeQueue.flink = NULL;
    g_mainFreeQueue.blink = NULL;
    g_mainActiveQueue.flink = NULL;
    g_mainActiveQueue.blink = NULL;

    for ( i=0; i<MAIN_QUEUE_SIZE; i++ ) {

      stat = INSQTI( (void *) &g_mainNodes[i], (void *) &g_mainFreeQueue,
       0 );
      if ( !( stat & 1 ) ) {
        printf( main_str39 );
        exit(0);
      }

    }

  }

  appArgsHead = new appListType;
  appArgsHead->flink = appArgsHead;
  appArgsHead->blink = appArgsHead;

  XtSetLanguageProc( NULL, NULL, NULL );

  XSetErrorHandler( xErrorHandler );

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

    //printf( "adjust args for restart\n" );

    //printf( "argc = %-d\n", argc );

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

  //printf( "argc = %-d\n", args->argc );
  //for ( i=0; i<args->argc; i++ ) {
  //  printf( "argv[%-d] = [%s]\n", i, args->argv[i] );
  //}

  if ( server ) {
    stat = args->appCtxPtr->startApplication( args->argc, args->argv, 2,
     oneInstance, convertOnly );
  }
  else {
    stat = args->appCtxPtr->startApplication( args->argc, args->argv, 1,
     oneInstance, convertOnly );
  }
  if ( !( stat & 1 ) ) exit( 0 );

  if ( stat & 1 ) { // success
    oneAppCtx = args->appCtxPtr->appContext();
    XtAppSetErrorHandler( oneAppCtx, xtErrorHandler );
    XtAppSetWarningHandler( oneAppCtx, xtErrorHandler );
  }

  if ( restart ) { // open all displays

    n = getNumCheckPointScreens( f );

    //printf( "%-d screen(s)\n", n );

    for ( i=0; i<n; i++ ) {

      stat = getScreenCheckPointParams( f, screenName, &x, &y, &icon,
       &screenNoEdit, &numCheckPointMacros, checkPointMacros );

      if ( stat & 1 ) {
        stat = args->appCtxPtr->openCheckPointScreen( screenName, x, y, icon,
         screenNoEdit, numCheckPointMacros, checkPointMacros );
      }

    }

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

        newOne->blink = appArgsHead->blink;
        appArgsHead->blink->flink = newOne;
        newOne->flink = appArgsHead;
        appArgsHead->blink = newOne;

        args->appCtxPtr = new appContextClass;
        args->appCtxPtr->proc = &proc;

        stat = args->appCtxPtr->startApplication( args->argc, args->argv, 0,
         0, 0 );

        if ( stat & 1 ) { // success
          oneAppCtx = args->appCtxPtr->appContext();
          XtAppSetErrorHandler( oneAppCtx, xtErrorHandler );
          XtAppSetWarningHandler( oneAppCtx, xtErrorHandler );
	}

        g_numClients++;

        n = getNumCheckPointScreens( f );

        //printf( "%-d screen(s)\n", n );

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
      printf( "reset\n" );
      memTrackReset();
    }

    showMem();
    printf( "[%-d]\n", zz );

#endif

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

        cur->appArgs->appCtxPtr->closeDownAppCtx();

        cur->appArgs->appCtxPtr->objDelFlag = 5;

	// blank display name
	strcpy( cur->appArgs->appCtxPtr->displayName, "" );

        stat = thread_lock_master( serverH );
        g_numClients--;
        stat = thread_unlock_master( serverH );

        numAppsRemaining++;

      }
      else if ( cur->appArgs->appCtxPtr->exitFlag &&
       cur->appArgs->appCtxPtr->objDelFlag > 1 ) {

        //printf( "decrement\n" );

        (cur->appArgs->appCtxPtr->objDelFlag)--;

        numAppsRemaining++;

      }
      else if ( cur->appArgs->appCtxPtr->exitFlag &&
       cur->appArgs->appCtxPtr->objDelFlag == 1 ) {

        //printf( "delete\n" );

        // unlink and delete
        cur->blink->flink = cur->flink;
        cur->flink->blink = cur->blink;

        oneAppCtx = cur->appArgs->appCtxPtr->appContext();
        oneDisplay = cur->appArgs->appCtxPtr->getDisplay();
        delete cur->appArgs->appCtxPtr;
        for ( i=0; i<cur->appArgs->argc; i++ ) delete[] cur->appArgs->argv[i];
        delete[] cur->appArgs->argv;
        delete cur->appArgs;

        delete cur;

        // Can't execute the next two lines; program crashes. Have to live
        // with memory leak. This only applies to the case where one server
        // is managing multiple app ctx's / displays.
        //XtCloseDisplay( oneDisplay );
        //XtDestroyApplicationContext( oneAppCtx );

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

            strncpy( tmpMsg, node->msg, 255 );
            tmpMsg[255] = 0;

            buf1 = NULL;
            tk = strtok_r( tmpMsg, "|", &buf1 );
            if ( !tk ) goto parse_error;

	    if ( strcmp( tk, "*OPN*" ) == 0 ) {

              needConnect = 1;
              tk = strtok_r( NULL, "|", &buf1 ); // should contain display name

	      // make 1st app ctx open/deiconify/raise initial files
	      // and deiconify/raise main window so things look like
	      // a new instance of edm is starting
              first = appArgsHead->flink;
              while ( first != appArgsHead ) {
		if ( ( strcmp( tk, ":0.0" ) == 0 ) ||
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
	    else if ( strcmp( tk, "*OIS*" ) == 0 ) {

              needConnect = 1;
              tk = strtok_r( NULL, "|", &buf1 ); // should contain display name

	      // make 1st app ctx open/deiconify/raise initial files
	      // and deiconify/raise main window so things look like
	      // a new instance of edm is starting
              first = appArgsHead->flink;
              while ( first != appArgsHead ) {
		if ( ( strcmp( tk, ":0.0" ) == 0 ) ||
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
                if ( !tk ) goto parse_error;
                tk = strtok_r( NULL, "|", &buf1 );
                if ( !tk ) goto parse_error;
                tk = strtok_r( NULL, "|", &buf1 );
                if ( !tk ) goto parse_error;
	      }
	      else {
                strncpy( tmpMsg, node->msg, 255 );
                tmpMsg[255] = 0;
                buf1 = NULL;
                tk = strtok_r( tmpMsg, "|", &buf1 );
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

              newOne->blink = appArgsHead->blink;
              appArgsHead->blink->flink = newOne;
              newOne->flink = appArgsHead;
              appArgsHead->blink = newOne;

              args->appCtxPtr = new appContextClass;
              args->appCtxPtr->proc = &proc;

              stat = args->appCtxPtr->startApplication( args->argc, args->argv,
               0, 0, 0 );

              if ( stat & 1 ) { // success
                oneAppCtx = args->appCtxPtr->appContext();
                XtAppSetErrorHandler( oneAppCtx, xtErrorHandler );
                XtAppSetWarningHandler( oneAppCtx, xtErrorHandler );
	      }

              g_numClients++;

	    }

parse_error:

            q_stat_i = INSQTI( (void *) node, (void *) &g_mainFreeQueue, 0 );
            if ( !( q_stat_i & 1 ) ) {
              printf( main_str40 );
            }

          }
          else if ( q_stat_r != QUEWASEMP ) {
            printf( main_str41 );
          }

          stat = thread_unlock_master( serverH );

        } while ( q_stat_r & 1 );

      }

    }

    if ( !numAppsRemaining ) exitProg = 1;

    pend_event( 0.1 );
    //stat = thread_wait_for_timer( delayH );
    //stat = thread_init_timer( delayH, 0.1 );

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
    }

  }

  if ( server ) {
    stat = ipnsv_delete_port( &g_conPort );
  }

  if ( shutdown ) {
    fprintf( f, "<<<EOD>>>\n" );
    fclose( f );
  }

  delete obj;
  obj = NULL;

  delete pvObj;
  pvObj = NULL;

  stat = thread_destroy_handle( serverH );
  serverH = NULL;

  stat = thread_destroy_handle( delayH );
  delayH = NULL;

  if ( server ) {

    stat = sys_destroyq( &g_mainFreeQueue );

    stat = sys_destroyq( &g_mainActiveQueue );

  }

}


