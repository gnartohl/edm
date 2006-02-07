#ifndef __thread_priv_h
#define __thread_priv_h 1

/* The following is needed due to the change
   in RH 7.1 to /usr/include/sys/time.h
*/
#define __USE_GNU

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
//#include <linux/unistd.h>
#include <unistd.h>
#include <errno.h>
#include "sys_types.h"
#include "thread.h"

#define THREAD_PTR_TYPE_GENERIC 1
#define THREAD_PTR_TYPE_HANDLE 2
#define THREAD_PTR_TYPE_LOCK 3

typedef struct cleanupListTag {
  struct cleanupListTag *flink;
  int ptrType;
  void *ptr;
} cleanupListType, *cleanupListPtr;

/* typedef void *pthread_mutexattr_t; */
typedef void *ptread_condattr_t;
typedef void *ptread_attr_t;

typedef struct thread_lock_tag {
  pthread_mutexattr_t mu_attr;
  pthread_mutex_t mutex;
} THREAD_LOCK_TYPE, *THREAD_LOCK_PTR;

typedef struct thread_lock_array_tag {
  int num_elements;
  pthread_mutexattr_t *mu_attr_array;
  pthread_mutex_t *mutex_array;
} THREAD_LOCK_ARRAY_TYPE, *THREAD_LOCK_ARRAY_PTR;

typedef struct thread_id_tag {
  struct timeval timer_exp_time;
  struct timeval delta_time;
  pthread_mutexattr_t master_mu_attr;
  pthread_mutex_t master_mutex;
  ptread_condattr_t master_cv_attr;
  pthread_cond_t master_cv;
  pthread_mutexattr_t mu_attr;
  pthread_mutex_t mutex;
  pthread_condattr_t cv_attr;
  pthread_cond_t cv;
  pthread_attr_t thr_attr;
  pthread_t os_thread_id;
  double timer_tick;
  int wantJoin;
  void *application_data;
  int process_active;
  int parent_detached;
} THREAD_ID_TYPE;
typedef THREAD_ID_TYPE *THREAD_ID_PTR;

#define TIMEVAL_TO_TIMESPEC( val, spec )\
  spec->tv_sec = val->tv_sec;\
  spec->tv_nsec = val->tv_usec * 1000;

#define TIMESPEC_TO_TIMEVAL( spec, val )\
  val->tv_sec = spec->tv_sec;\
  val->tv_usec = spec->tv_nsec / 1000;

/* add timeval's */
#define timeradd( time1, time2, sum )\
{ \
double s1 = (double) time1->tv_sec + \
 ( (double) time1->tv_usec ) / 1.0e6; \
double s2 = (double) time2->tv_sec + \
 ( (double) time2->tv_usec ) / 1.0e6; \
double s3 = s1 + s2; \
sum->tv_sec = (int) s3; \
sum->tv_usec = (int) ( ( s3 - (double) sum->tv_sec ) * 1.0e6 ); \
}

#endif
