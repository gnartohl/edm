#ifndef __thread_priv_h
#define __thread_priv_h 1

#include $vaxelnc
#include $mutex
#include $kernelmsg
#include thread

typedef struct thread_lock_tag {
  MUTEX mutex;
} THREAD_LOCK_TYPE, *THREAD_LOCK_PTR;

typedef struct thread_lock_array_tag {
  int num_elements;
  MUTEX *mutex_array;
} THREAD_LOCK_ARRAY_TYPE, *THREAD_LOCK_ARRAY_PTR;

typedef struct thread_id_tag {
  MUTEX master_mutex;
  EVENT master_cv;
  MUTEX mutex;
  EVENT cv;
  SYS_TIME_TYPE delta_time;
  SYS_TIME_TYPE timer_exp_time;
  PROCESS os_thread_id;
  int exit_status;
  void *application_data;
} THREAD_ID_TYPE;
typedef THREAD_ID_TYPE *THREAD_ID_PTR;

#endif
