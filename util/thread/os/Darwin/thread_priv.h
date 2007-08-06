#ifndef __thread_priv_h
#define __thread_priv_h 1

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
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

#endif
