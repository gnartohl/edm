#include stdio
#include stdlib
#include descrip
#include $vaxelnc
#include $kernelmsg

#include sys_types

static g_init = 1;
static $DESCRIPTOR( onesec, "0 0:0:1.0" );
static LARGE_INTEGER sec;


int sys_cvt_seconds_to_timeout (
  float seconds,
  SYS_TIME_PTR timeout
) {

int result, stat;

  if ( g_init ) {
    g_init = 0;
    sec = eln$time_value( &onesec );
  }

/***
/* seconds must be positive
*/
  if ( seconds < 0.01 ) seconds = 0.01;
    
  memcpy( timeout, &sec, sizeof(SYS_TIME_TYPE) );
  stat = lib$multf_delta_time( &seconds, timeout );

  return stat;

}

void sys_wait_seconds (
  float *seconds
) {

LARGE_INTEGER delay;
int result, stat;

  if ( g_init ) {
    g_init = 0;
    sec = eln$time_value( &onesec );
  }
    
  delay = sec;
  lib$multf_delta_time( seconds, &delay );
  ker$wait_any( &stat, &result, &delay );

}

int sys_get_proc_id (
  SYS_PROC_ID_PTR proc_id
) {

int stat, item;
unsigned int pid;

  proc_id->id = 0;
  stat = 1;
  return stat;

}

int sys_get_user_name (
  int max_chars,
  char *name
) {

struct dsc$descriptor dsc;
int stat, item;
short len;

  strcpy( name, "" );

  stat = 1;
  return stat;

}

int sys_get_time (
  SYS_TIME_PTR time
) {

int stat;

  stat = sys$gettim( time );

  return stat;

}

int sys_add_times (
  SYS_TIME_PTR time1,
  SYS_TIME_PTR time2
) {

/*
* time1 = time1 + time2
*/

  eln$add_large_integers( time1, time2 );

}
