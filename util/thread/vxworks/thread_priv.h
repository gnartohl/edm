#ifndef __thread_priv_h
#define __thread_priv_h 1

#include <vxWorks.h>
#include <semLib.h>
#include <msgQLib.h>
#include "thread.h"

#define THR_BASE_PRIOR 128
#define THR_HIGH_PRIOR 192
#define THR_LOW_PRIOR 64
#define THR_DEF_STACK_SIZE 10240

typedef struct thread_lock_tag {
  SEM_ID mutex;
} THREAD_LOCK_TYPE, *THREAD_LOCK_PTR;

typedef struct thread_lock_array_tag {
  int num_elements;
  SEM_ID *mutex_array;
} THREAD_LOCK_ARRAY_TYPE, *THREAD_LOCK_ARRAY_PTR;

typedef struct thread_id_tag {
  char *name;
  int stack_size;
  int prior;
  SEM_ID master_mutex;
  MSG_Q_ID master_cv;
  SEM_ID mutex;
  MSG_Q_ID cv;
  SEM_ID timer_mutex;
  int os_thread_id;
  float seconds;
  int timer_ticks;
  WDOG_ID wdog_id;
  int wdog_state;
  struct timespec timer_exp_time;
  struct timespec delta_time;
  void *application_data;
} THREAD_ID_TYPE;
typedef THREAD_ID_TYPE *THREAD_ID_PTR;

#endif
