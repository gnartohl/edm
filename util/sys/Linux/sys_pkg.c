#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>

#include "sys_types.h"
#include "thread.h"

char *Strncat(
  char *dest,
  char *src,
  int max ) {

  // max must be >= 0 and no more than stringsize - 1
  //
  // for char string[10];       max must be <= 9

int l, newMax;
char *s;

  l = strlen( dest );
  newMax = max - l;
  if ( newMax < 0 ) {
    dest[max] = 0;
    return dest;
  }

  s = strncat( dest, src, newMax );
  dest[max] = 0;

  return s;

}

void sys_wait_seconds (
  float *seconds
) {

struct timeval delay_time;

  delay_time.tv_sec = (int) *seconds;
  delay_time.tv_usec = (int) ((*seconds - (float) delay_time.tv_sec) * 1.0e6);

  select( 0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &delay_time );

}

int sys_iniq (
  void *queue
) {

int stat;

GEN_QUEUE_PTR gen_queue;

  gen_queue = (GEN_QUEUE_PTR) queue;

  stat = thread_init();
  if ( !( stat & 1 ) ) return stat;

  stat = thread_create_lock_handle( &gen_queue->lock );
  if ( !( stat & 1 ) ) return stat;

  return 1;

}

int sys_destroyq (
  void *queue
) {

int stat;

GEN_QUEUE_PTR gen_queue;

  gen_queue = (GEN_QUEUE_PTR) queue;

  stat = thread_destroy_lock_handle( gen_queue->lock );
  if ( !( stat & 1 ) ) return stat;

  return 1;

}

int sys_insqt (
  void *node,
  void *queue,
  int flag
) {

int stat;

GEN_NODE_PTR gen_node;
GEN_QUEUE_PTR gen_queue;

  gen_node = (GEN_NODE_PTR) node;
  gen_queue = (GEN_QUEUE_PTR) queue;

  stat = thread_lock( gen_queue->lock );
  if ( !( stat & 1 ) ) return stat;

  if ( !gen_queue->flink && !gen_queue->blink ) { /* queue is empty */

    gen_queue->flink = gen_node;
    gen_node->flink = (GEN_NODE_PTR) gen_queue;

    gen_queue->blink = gen_node;
    gen_node->blink = (GEN_NODE_PTR) gen_queue;

    stat = thread_unlock( gen_queue->lock );
    if ( !( stat & 1 ) ) return stat;

    return SYS_ONEENTQUE;

  }
  else {

    gen_queue->blink->flink = gen_node;
    gen_node->flink = (GEN_NODE_PTR) gen_queue;

    gen_node->blink = gen_queue->blink;
    gen_queue->blink = gen_node;

    stat = thread_unlock( gen_queue->lock );
    if ( !( stat & 1 ) ) return stat;

    return 1;

  }

}

int sys_remqh (
  void *queue,
  void **node,
  int flag
) {

int stat;

GEN_NODE_PTR gen_node;
GEN_QUEUE_PTR gen_queue;

  gen_queue = (GEN_QUEUE_PTR) queue;

  stat = thread_lock( gen_queue->lock );
  if ( !( stat & 1 ) ) return stat;

  if ( !gen_queue->flink ) { /* queue is empty */

    stat = thread_unlock( gen_queue->lock );
    if ( !( stat & 1 ) ) return stat;

    return SYS_QUEWASEMP;

  }
  else {

    gen_node = gen_queue->flink;

    gen_queue->flink = gen_node->flink;
    gen_queue->flink->blink = (GEN_NODE_PTR) gen_queue;

    if ( (GEN_NODE_PTR) gen_queue == gen_queue->flink ) {
      gen_queue->flink = (GEN_NODE_PTR) 0;
      gen_queue->blink = (GEN_NODE_PTR) 0;
    }

    *node = (void *) gen_node;

    stat = thread_unlock( gen_queue->lock );
    if ( !( stat & 1 ) ) return stat;

    return 1;

  }

}

int sys_get_proc_id (
  SYS_PROC_ID_PTR proc_id
) {

  proc_id->id = getpid();

  return 1;

}

int sys_get_user_name (
  int max_chars,
  char *name
) {

char *value;

  value = getenv( "USER" );

  if ( value )
    strncpy( name, value, max_chars );
  else
    strcpy( name, "" );

  return 1;

}

int sys_get_time (
  SYS_TIME_PTR tim
) {

  time( &tim->cal_time );
  tim->tm_time = *localtime( &tim->cal_time );

  return 1;

}

int sys_get_datetime_string (
  int string_size,
  char *string
) {

SYS_TIME_TYPE tim;
int stat;

  stat = sys_get_time( &tim );
  strncpy( string, ctime( &tim.cal_time ), string_size );
  string[strlen(string)-1] = 0;

  return 1;

}

static int isInteger (
  char *string
) {

int i;

  for ( i=0; i<strlen(string); i++ ) {
    if ( !isdigit( string[i] ) ) return 0;
  }

  return 1;

}

static int getMonth (
  char *string,
  int len,
  int *mon
) {

char buf[255+1];
int i;
static char *months[] = {
 "jan",
 "feb",
 "mar",
 "apr",
 "may",
 "jun",
 "jul",
 "aug",
 "sep",
 "oct",
 "nov",
 "dec"
};

  if ( len > 255 ) len = 255;

  for ( i=0; i<len; i++ ) {
    buf[i] = tolower(string[i]);
  }
  buf[len] = 0;

  for ( i=0; i<12; i++ ) {
    if ( strcmp( buf, months[i] ) == 0 ) {
      *mon = i;
      return 1;
    }
  }

  return -1;

}

int sys_cvt_string_to_time (
  char *string,
  int string_size,
  SYS_TIME_PTR time
) {

char *tk, buf[255+1];
int stat, year;

  /* convert "day-month-year hour:min:sec" into machine time */

  /* get current time */
  stat = sys_get_time( time );
  if ( !( stat & 1 ) ) return stat;

  /* zero time fields */
  time->tm_time.tm_hour = 0;
  time->tm_time.tm_min = 0;
  time->tm_time.tm_sec = 0;

  /* parse string

       The date may be ommitted but if it is specified, all fields must
       be included

       The time may be ommitted or seconds may be ommitted or minutes:seconds
       may be ommitted

       The following are examples of legal strings:

       jan-1-2000
       jan-1-2000 3
       jan-1-2000 3:01
       jan-1-2000 3:12
       jan-1-2000 3:01:01
       jan-1-2000 3:12:01
       jan-1-2000 3:12:12
       3
       3:01
       3:12
       3:01:01
       3:12:01
       3:12:12

  */

  strncpy( buf, string, 255 );
  buf[255] = 0;

  tk = strtok( buf, " \t\n-:" );
  if ( !tk ) return SYS_FAIL;

  /* tk is either a month abbreviation or and integer */

  if ( isInteger( tk ) ) {

    /* date has been ommitted, get time fields */

    time->tm_time.tm_hour = atol( tk );

    tk = strtok( NULL, ": \t\n" );
    if ( !tk ) goto done; /* time fields may be ommitted */
    if ( !isInteger( tk ) ) return SYS_FAIL;
    time->tm_time.tm_min = atol( tk );

    tk = strtok( NULL, ": \t\n" );
    if ( !tk ) goto done; /* time fields may be ommitted */
    if ( !isInteger( tk ) ) return SYS_FAIL;
    time->tm_time.tm_sec = atol( tk );

  }
  else {

    /* tk should be a month abbreviation */

    /* date */
    stat = getMonth( tk, strlen(tk), &time->tm_time.tm_mon );
    if ( !( stat & 1 ) ) return SYS_FAIL;

    tk = strtok( NULL, "-" );
    if ( !tk ) return SYS_FAIL; /* date must be wholey specified */
    if ( !isInteger( tk ) ) return SYS_FAIL;
    time->tm_time.tm_mday = atol( tk );

    tk = strtok( NULL, " \t\n" );
    if ( !tk ) return SYS_FAIL; /* date must be wholey specified */
    if ( !isInteger( tk ) ) return SYS_FAIL;
    year = atol( tk );
    if ( year < 70 ) year += 2000;
    time->tm_time.tm_year =  year - 1900;

    /* time */
    tk = strtok( NULL, ": \t\n" );
    if ( !tk ) goto done; /* time fields may be ommitted */
    if ( !isInteger( tk ) ) return SYS_FAIL;
    time->tm_time.tm_hour = atol( tk );

    tk = strtok( NULL, ": \t\n" );
    if ( !tk ) goto done; /* time fields may be ommitted */
    if ( !isInteger( tk ) ) return SYS_FAIL;
    time->tm_time.tm_min = atol( tk );

    tk = strtok( NULL, ": \t\n" );
    if ( !tk ) goto done; /* time fields may be ommitted */
    if ( !isInteger( tk ) ) return SYS_FAIL;
    time->tm_time.tm_sec = atol( tk );

  }

done:

  time->cal_time = mktime( &time->tm_time );
  if ( time->cal_time == -1 ) return SYS_FAIL;

  return 1;

}

int sys_cvt_time_to_string (
  SYS_TIME_PTR tim,
  int string_size,
  char *string
) {


  strncpy( string, ctime( &tim->cal_time ), string_size );
  string[strlen(string)-1] = 0;

  return 1;

}

int sys_cvt_time_to_julian_date (
  SYS_TIME_PTR tim,
  int *julian_date
) {

  *julian_date = tim->cal_time / 3600 / 24;

  return 1;

}

int sys_get_time_diff_in_hours (
  SYS_TIME_PTR cur_tim,
  SYS_TIME_PTR tim,
  float *hours
) {

double seconds;

  seconds = difftime( tim->cal_time, cur_tim->cal_time );
  *hours = (float) ( seconds / 3600.0 );

  return 1;

}

int sys_cvt_hours_to_time (
  float hours,
  SYS_TIME_PTR tim
) {

  tim->cal_time = (int) ( hours * 3600.0 );
  tim->tm_time = *localtime( &tim->cal_time );

  return 1;

}

int sys_add_times (	/* time1 = time1 + time2 */
  SYS_TIME_PTR time1,
  SYS_TIME_PTR time2
) {

  time1->cal_time += time2->cal_time;
  time1->tm_time = *localtime( &time1->cal_time );

  return 1;

}

int sys_subtract_times (	/* time1 = time1 - time2 */
  SYS_TIME_PTR time1,
  SYS_TIME_PTR time2
) {

  time1->cal_time -= time2->cal_time;
  time1->tm_time = *localtime( &time1->cal_time );

  return 1;

}

int sys_cvt_seconds_to_timeout (
  float seconds,
  SYS_TIME_PTR tim
) {

  tim->timeval_time.tv_sec = (int) seconds;
  tim->timeval_time.tv_usec =
   (int) ( ( seconds - (float) tim->timeval_time.tv_sec ) * 1.0e6 );

  return 1;

}

int sys_cvt_timeout_to_seconds (
  SYS_TIME_PTR tim,
  float *seconds
) {

  *seconds = (float) tim->timeval_time.tv_sec +
   ( (float) tim->timeval_time.tv_usec ) / 1.0e6;

  return 1;

}
#ifdef zzz

static int tt_init = 1;
static $DESCRIPTOR( tt_dev, "tt:" );
static int tt_chan, tt_ef;

int sys_get_char ( void ) {

/* gets one character with noecho */

int stat, buf_size;
unsigned int func = IO$M_NOECHO | IO$_READVBLK;
IOSB_TYPE iosb;
char buf[4];

  if ( tt_init ) {
    tt_chan = 0;
    stat = sys$assign( &tt_dev, &tt_chan, 0, 0, 0 );
    if ( stat != SS$_NORMAL ) return 0;
    tt_init = 0;
    stat = lib$get_ef( &tt_ef );
    if ( stat != SS$_NORMAL ) tt_ef = 0;
  }

  buf_size = 1;
  stat = sys$qiow( tt_ef, tt_chan, func, &iosb, 0, 0, &buf,
   buf_size, 0, 0, 0, 0 );

  if ( stat != SS$_NORMAL )
    return 0;
  else
    return (int) buf[0];

}

int sys_next_char ( void ) {

/* gets next char in typeahead buffer (nondestructive) */

typedef struct {
  short num;
  char first;
  char res_c;
  long res_l;
} TT_TABUF_TYPE;

int stat, buf_size;
unsigned int func = IO$_SENSEMODE | IO$M_TYPEAHDCNT;
IOSB_TYPE iosb;
TT_TABUF_TYPE buf;

  if ( tt_init ) {
    tt_chan = 0;
    stat = sys$assign( &tt_dev, &tt_chan, 0, 0, 0 );
    if ( stat != SS$_NORMAL ) return 0;
    tt_init = 0;
    stat = lib$get_ef( &tt_ef );
    if ( stat != SS$_NORMAL ) tt_ef = 0;
  }

  buf_size = 1;
  stat = sys$qiow( tt_ef, tt_chan, func, &iosb, 0, 0, &buf,
   sizeof(buf), 0, 0, 0, 0 );

  if ( stat != SS$_NORMAL )
    return 0;
  else
    return (int) buf.first;

}

int sys_num_chars ( void ) {

/* gets number of chars in typeahead buffer */

typedef struct {
  short num;
  char first;
  char res_c;
  long res_l;
} TT_TABUF_TYPE;

int stat, buf_size;
unsigned int func = IO$_SENSEMODE | IO$M_TYPEAHDCNT;
IOSB_TYPE iosb;
TT_TABUF_TYPE buf;

  if ( tt_init ) {
    tt_chan = 0;
    stat = sys$assign( &tt_dev, &tt_chan, 0, 0, 0 );
    if ( stat != SS$_NORMAL ) return 0;
    tt_init = 0;
    stat = lib$get_ef( &tt_ef );
    if ( stat != SS$_NORMAL ) tt_ef = 0;
  }

  buf_size = 1;
  stat = sys$qiow( tt_ef, tt_chan, func, &iosb, 0, 0, &buf,
   sizeof(buf), 0, 0, 0, 0 );

  if ( stat != SS$_NORMAL )
    return 0;
  else
    return (int) buf.num;

}
#endif




