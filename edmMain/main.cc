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

static MAIN_QUE_TYPE mainFreeQueue, mainActiveQueue;
static MAIN_NODE_TYPE mainNodes[MAIN_QUEUE_SIZE];
static IPRPC_PORT con_port;

//#include "alloc.h"
#ifdef DIAGNOSTIC_ALLOC
int z=2, zz=0;
#endif


static int xErrorHandler (
  Display *d,
  XErrorEvent *err )
{

char msg[80];

  if ( err->error_code != BadAccess ) {
    XGetErrorText( d, err->error_code, msg, 80 );
    fprintf( stderr, main_str1, err->error_code, msg );
  }

  return 0;

}

int getHostAddr (
  char *addr )
{

int stat;
struct hostent *entry;
char name[255+1];
unsigned char *adr;

  strcpy( addr, "0.0.0.0" );

  stat = gethostname( name, 255 );
  if ( stat ) return stat;

  entry = gethostbyname( name );
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
  char *displayName )
{

char addr[127+1];
int i, len, pos, max, argCount, stat;
IPRPC_PORT port;
char msg[255+1], portNumStr[15+1];
SYS_TIME_TYPE timeout;

  sprintf( portNumStr, "%-d", portNum );

  stat = getHostAddr( addr );
  if ( stat ) return;

  if ( appendDisplay ) {
    argCount = argc + 2; // we will add two parameters below
  }
  else {
    argCount = argc;
  }

  sprintf( msg, "%-d|", argCount );
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
    strncat( &msg[pos], "|", max );
    pos = strlen(msg);
    max = 255 - pos;
  }

  for ( i=1; i<argc; i++ ) {
    strncpy( &msg[pos], argv[i], max );
    pos = strlen(msg);
    max = 255 - pos;
    strncat( &msg[pos], "|", max );
    pos = strlen(msg);
    max = 255 - pos;
  }

  stat = sys_cvt_seconds_to_timeout( 10.0, &timeout );
  if ( !( stat & 1 ) ) {
    printf( main_str3 );
    return;
  }

  stat = ipncl_create_port( 1, 4096, "test", &port );
  if ( !( stat & 1 ) ) {
    printf( main_str4 );
    return;
  }

  stat = ipncl_connect( addr, portNumStr, "", port );
  if ( !( stat & 1 ) ) {
    return;
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

int stat;

  do {

#ifdef __epics__
    stat = ca_pend_io( 5.0 );
    stat = ca_pend_event( 0.05 );
#endif

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

int stat, n, q_stat_r, q_stat_i, result;
THREAD_HANDLE delayH;
MAIN_NODE_PTR node;
IPRPC_PORT port;
SYS_TIME_TYPE timeout;
int len;
char msg[255+1], portNumStr[15+1];

int *portNumPtr = (int *) thread_get_app_data( h );

  stat = thread_create_handle( &delayH, NULL );

  stat = sys_cvt_seconds_to_timeout( 10.0, &timeout );
  if ( !( stat & 1 ) ) {
    printf( main_str3 );
    goto err_return;
  }

  sprintf( portNumStr, "%-d", *portNumPtr );

  stat = ipnsv_create_named_port( portNumStr, 1, 255, "edm", &con_port );
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

    stat = ipnsv_accept_connection( con_port, port, " " );
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
      stat = ipnsv_disconnect( con_port );
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

    stat = thread_lock_master( h );

    q_stat_r = REMQHI( (void *) &mainFreeQueue, (void **) &node, 0 );
    if ( q_stat_r & 1 ) {
      n++;
      strncpy( node->msg, msg, 255 );
      q_stat_i = INSQTI( (void *) node, (void *) &mainActiveQueue, 0 );
      if ( !( q_stat_i & 1 ) ) {
        printf( main_str17 );
      }
    }
    else {
      printf( main_str18 );
    }

    stat = thread_unlock_master( h );

    stat = ipnsv_disconnect( port );
    stat = ipnsv_delete_port( &port );
    stat = ipnsv_disconnect( con_port );

    thread_delay( delayH, 0.5 );

  }

err_return:

  stat = 0;

#ifdef __linux__
  return NULL;
#endif

#ifdef __solaris__
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
  int *portNum )
{

char buf[1023+1], mac[1023+1], exp[1023+1];
int state = SWITCHES;
int stat, nm=0, n = 1;
char *envPtr, *tk;
Display *testDisplay;

  strcpy( displayName, "" );
  *local = 0;
  *server = 0;
  *appendDisplay = 1;
  *portNum = 19000;

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
        else if ( strcmp( argv[n], global_str10 ) == 0 ) {
          *server = 1;
          *local = 0;
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
          tk = strtok( buf, "=," );
          while ( tk ) {
            strncpy( mac, tk, 1023 );
            tk = strtok( NULL, "=," );
            if ( tk ) {
              strncpy( exp, tk, 1023 );
            }
            else {
              *local = 1;
              return;
            }
            nm++;
            tk = strtok( NULL, "=," );
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
        else if ( strcmp( argv[n], "-port" ) == 0 ) {
          n++;
          if ( n >= argc ) { // missing port num
            *local = 1;
            return;
          }
          *portNum = atol( argv[n] );
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

      strncat( displayName, ":0.0", 31 );

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

int i, stat, numAppsRemaining, exitProg, q_stat_r, q_stat_i, local, server,
 portNum;
THREAD_HANDLE delayH, serverH, caPendH;
argsPtr args;
appListPtr cur, next, appArgsHead, newOne;
processClass proc;
objBindingClass *obj;
pvBindingClass *pvObj;
char *tk;
MAIN_NODE_PTR node;
char **argArray, displayName[127+1];
int appendDisplay;
float hours, seconds;

  checkParams( argc, argv, &local, &server, &appendDisplay, displayName,
   &portNum );

  if ( server ) {

    checkForServer( argc, argv, portNum, appendDisplay, displayName );

  }

  if ( server ) {

    stat = sys_iniq( &mainFreeQueue );
    if ( !( stat & 1 ) ) {
      printf( main_str37 );
      exit(0);
    }
    stat = sys_iniq( &mainActiveQueue );
    if ( !( stat & 1 ) ) {
      printf( main_str38 );
      exit(0);
    }

    mainFreeQueue.flink = NULL;
    mainFreeQueue.blink = NULL;
    mainActiveQueue.flink = NULL;
    mainActiveQueue.blink = NULL;

    for ( i=0; i<MAIN_QUEUE_SIZE; i++ ) {

      stat = INSQTI( (void *) &mainNodes[i], (void *) &mainFreeQueue,
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
#ifdef __epics__
  stat = thread_create_handle( &caPendH, NULL );
  stat = thread_create_proc( caPendH, caPendThread );
#endif
#endif

  args = new argsType;

  argArray = new char*[argc];
  for ( i=0; i<argc; i++ ) {
    argArray[i] = new char[strlen(argv[i])+1];
    strcpy( argArray[i], argv[i] );
  }

  args->argc = argc;
  args->argv = argArray;

  cur = new appListType;
  cur->appArgs = args;

  cur->blink = appArgsHead->blink;
  appArgsHead->blink->flink = cur;
  cur->flink = appArgsHead;
  appArgsHead->blink = cur;

  args->appCtxPtr = new appContextClass;
  args->appCtxPtr->proc = &proc;

  stat = args->appCtxPtr->startApplication( args->argc, args->argv );
  if ( !( stat & 1 ) ) exit( 0 );

  proc.timeCount = 0;
  proc.cycleTimeFactor = 1.0;
  proc.halfSecCount = 5;
  stat = sys_get_time( &proc.tim0 );
  stat = thread_init_timer( delayH, 0.1 );
  exitProg = 0;
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

      next = cur->flink;

      cur->appArgs->appCtxPtr->applicationLoop();
      if ( cur->appArgs->appCtxPtr->exitFlag ) {
        cur->blink->flink = cur->flink;
        cur->flink->blink = cur->blink;

        delete cur->appArgs->appCtxPtr;
        for ( i=0; i<cur->appArgs->argc; i++ ) delete cur->appArgs->argv[i];
        delete cur->appArgs->argv;

#if 0
        delete cur;
#endif

      }
      else {
        numAppsRemaining++;
      }

      cur = next;

      if ( server ) {

        do {

          stat = thread_lock_master( serverH );

          q_stat_r = REMQHI( (void *) &mainActiveQueue, (void **) &node, 0 );

          if ( q_stat_r & 1 ) {

            args = new argsType;

            tk = strtok( node->msg, "|" );
            if ( !tk ) goto parse_error;

            argc = (int) atol( tk );
            if ( argc == 0 ) goto parse_error;

            argArray = new char*[argc];

            for ( i=0; i<argc; i++ ) {
              tk = strtok( NULL, "|" );
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

            args->appCtxPtr->startApplication( args->argc, args->argv );

parse_error:

            q_stat_i = INSQTI( (void *) node, (void *) &mainFreeQueue, 0 );
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

    stat = thread_wait_for_timer( delayH );
    stat = thread_init_timer( delayH, 0.1 );

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
    stat = ipnsv_delete_port( &con_port );
  }

}