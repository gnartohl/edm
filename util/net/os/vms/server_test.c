#include stdio
#include descrip

#include sys_types
#include ipnsv

#include "vsys_include:ast_args_c.h"
#include "vsys_root:[include]vdb_type.h"
#include "vsys_root:[include]vdb_class.h"
#include "vsys_root:[include]vdb_structure.h"
#include "vsys_root:[include]vdb_convert.h"
#include "vsys_root:[include]vdb_hardware.h"
#include "vsys_root:[include]vdb_routines.h"
#include "vsys_root:[include]vdb_errors.h"
#include "vsys_root:[include]vdb_descrip.h"
#include "vsys_root:[include]vgi_include.h"

#define MAKE_CSTRING_DESCRIP( list, num_bytes, d_list )\
  d_list.dsc$w_length = num_bytes;\
  d_list.dsc$b_dtype = 0;\
  d_list.dsc$b_class = 0;\
  d_list.dsc$a_pointer = list

main (
  int argc,
  int **argv
) {

int i, stat, result, len, send_len, value;
IPRPC_PORT con_port, port;
char msg[255+1];
SYS_TIME_TYPE timeout;
char name[128];
struct dsc$descriptor dsc;
int chix[4];
int values[4] = { 0,0,0,0 };

  value = 0;

/*
  strcpy( name, "knob_db::orib28:knob1" );
  MAKE_CSTRING_DESCRIP( name, strlen(name), dsc );
  stat = vdb_channel_index( &dsc, &chix[0] );
  stat = vdb_iput( &chix[0], &value );

  strcpy( name, "knob_db::orib28:knob2" );
  MAKE_CSTRING_DESCRIP( name, strlen(name), dsc );
  stat = vdb_channel_index( &dsc, &chix[1] );
  stat = vdb_iput( &chix[1], &value );

  strcpy( name, "knob_db::orib28:knob3" );
  MAKE_CSTRING_DESCRIP( name, strlen(name), dsc );
  stat = vdb_channel_index( &dsc, &chix[2] );
  stat = vdb_iput( &chix[2], &value );

  strcpy( name, "knob_db::orib28:knob4" );
  MAKE_CSTRING_DESCRIP( name, strlen(name), dsc );
  stat = vdb_channel_index( &dsc, &chix[3] );
  stat = vdb_iput( &chix[3], &value );
*/

  stat = sys_cvt_seconds_to_timeout( 60.0, &timeout );
  if ( !( stat & 1 ) ) {
    printf( "error from sys_cvt_seconds_to_timeout\n" );
    exit(stat);
  }

  stat = ipnsv_create_named_port( "15123", 1, 4096, "test (s)", &con_port );
  if ( !( stat & 1 ) ) {
    printf( "error from ipnsv_create_named_port\n" );
    exit(stat);
  }

  stat = ipnsv_create_port( 1, 4096, "test", &port );
  if ( !( stat & 1 ) ) {
    printf( "error from ipnsv_create_port\n" );
    exit(stat);
  }

  stat = ipnsv_accept_connection( con_port, port, " " );
  if ( !( stat & 1 ) ) {
    printf( "error from ipnsv_accept_connection\n" );
    exit(stat);
  }

  while ( 1 ) {

    stat = ipnsv_wait_on_port( port, &timeout, &result );
    if ( !( stat & 1 ) ) {
      printf( "error from ipnsv_wait_on_port\n" );
      exit(stat);
    }
    if ( !result ) {

      printf( "Timeout\n" );
      continue;

    }
    else {

      strcpy( msg, "              " );
      stat = ipnsv_receive_msg( port, 20, &len, msg );
      if ( !( stat & 1 ) ) {
        printf( "error from ipnsv_receive_msg\n" );
        exit(stat);
      }

      if ( strcmp( msg, "EXIT" ) == 0 ) goto done;

      printf( "%s", msg );

      send_len = strlen( msg );

/*
      stat = ipnsv_send_msg( port, len+1, msg );
      if ( !( stat & 1 ) ) {
        printf( "error from ipnsv_send_msg\n" );
        exit(stat);
      }
*/

      stat = ipnsv_send_event_msg( port, len+1, msg );
      if ( !( stat & 1 ) ) {
        printf( "error from ipnsv_send_msg\n" );
        exit(stat);
      }

    }

  }

done:

  stat = ipnsv_disconnect( port );
  if ( !( stat & 1 ) ) {
    printf( "error from ipnsv_disconnect\n" );
    exit(stat);
  }

  stat = ipnsv_delete_port( &port );
  if ( !( stat & 1 ) ) {
    printf( "error from ipnsv_delete_port\n" );
    exit(stat);
  }

}
