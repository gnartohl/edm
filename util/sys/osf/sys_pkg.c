#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#ifdef __vxworks__
#include <time.h>
#include <sys/times.h>
#include <taskLib.h>
#endif

#ifdef __osf__
#include <time.h>
#include <sys/time.h>
#endif

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

  stat = thread_destroy_lock_handle( &gen_queue->lock );
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

void sys_wait_seconds (
  float *seconds
) {

struct timespec delay_time, time_remaining;
int stat;

  delay_time.tv_sec = (int) *seconds;
  delay_time.tv_nsec = (int) ((*seconds - (float) delay_time.tv_sec) * 1.0e9);

  stat = nanosleep( &delay_time, &time_remaining );

}

int sys_get_proc_id (
  SYS_PROC_ID_PTR proc_id
) {

#ifdef __osf__
  proc_id->id = getpid();
#endif

#ifdef __vxworks__
  proc_id->id = taskIdSelf();
#endif

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

#ifdef __vxworks__
int stat;
#endif

#ifndef __vxworks__
struct tm *t;
#endif

  time( &tim->cal_time );

#ifdef __vxworks__
  stat = localtime_r( &tim->cal_time, &tim->tm_time );
#endif

#ifndef __vxworks__
  t = localtime_r( &tim->cal_time, &tim->tm_time );
#endif

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

int sys_cvt_string_to_time (
  char *string,
  int string_size,
  SYS_TIME_PTR time
) {

  strncpy( string, "Not Implemented", string_size );

  return 1;

}

int sys_cvt_time_to_string (
  SYS_TIME_PTR tim,
  int string_size,
  char *string
) {

#ifdef __osf__
  ctime_r( &tim->cal_time, string, string_size );
#endif

#ifdef __vxworks__
  ctime_r( &tim->cal_time, string, (size_t *) &string_size );
#endif

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

#ifdef __vxworks__

int stat;

  tim->cal_time = (int) ( hours * 3600.0 );
  stat = localtime_r( &tim->cal_time, &tim->tm_time );

#endif

#ifndef __vxworks__

struct tm *t;

  tim->cal_time = (int) ( hours * 3600.0 );
  t = localtime_r( &tim->cal_time, &tim->tm_time );

#endif

  return 1;

}

int sys_add_times (	/* time1 = time1 + time2 */
  SYS_TIME_PTR time1,
  SYS_TIME_PTR time2
) {

#ifdef __vxworks__
int stat;
#endif

#ifndef __vxworks__
struct tm *t;
#endif

double sec, sec1, sec2;
time_t s;

  sec1 = (double) time1->timeval_time.tv_sec +
   ( (double) time1->timeval_time.tv_usec ) / 1.0e6;

  sec2 = (double) time2->timeval_time.tv_sec +
   ( (double) time2->timeval_time.tv_usec ) / 1.0e6;

  sec = sec1 + sec2;

  time1->timeval_time.tv_sec = (int) sec;
  time1->timeval_time.tv_usec =
   (int) ( ( sec - (double) time1->timeval_time.tv_sec ) * 1.0e6 );

  s = time2->cal_time;
  time1->cal_time = time1->cal_time + s;
/*  time1->cal_time += time2->cal_time; */


#ifdef __vxworks__
  stat = localtime_r( &time1->cal_time, &time1->tm_time );
#endif

#ifndef __vxworks__
  t = localtime_r( &time1->cal_time, &time1->tm_time );
#endif

  return 1;

}

int sys_subtract_times (	/* time1 = time1 - time2 */
  SYS_TIME_PTR time1,
  SYS_TIME_PTR time2
) {

#ifdef __vxworks__
int stat;
#endif

#ifndef __vxworks__
struct tm *t;
#endif

double sec, sec1, sec2;

  sec1 = (double) time1->timeval_time.tv_sec +
   ( (double) time1->timeval_time.tv_usec ) / 1.0e6;

  sec2 = (double) time2->timeval_time.tv_sec +
   ( (double) time2->timeval_time.tv_usec ) / 1.0e6;

  sec = sec1 - sec2;

  printf( "sec1 = %-f, sec2 = %-f, sec = %-f\n", sec1, sec2, sec );

  time1->timeval_time.tv_sec = (int) sec;
  time1->timeval_time.tv_usec =
   (int) ( ( sec - (double) time1->timeval_time.tv_sec ) * 1.0e6 );
  
  time1->cal_time -= time2->cal_time;

#ifdef __vxworks__
  stat = localtime_r( &time1->cal_time, &time1->tm_time );
#endif

#ifndef __vxworks__
  t = localtime_r( &time1->cal_time, &time1->tm_time );
#endif

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

int sys_get_char ( void ) {

/* gets one character with noecho */

  return 0;

}

int sys_next_char ( void ) {

/* gets next char in typeahead buffer (nondestructive) */

  return 0;

}

int sys_num_chars ( void ) {

/* gets number of chars in typeahead buffer */

  return 0;

}
