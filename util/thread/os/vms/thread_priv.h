#ifndef __thread_priv_h
#define __thread_priv_h 1

#include thread
#include <cma.h>

typedef struct thread_lock_tag {
  cma_t_attr mu_attr;
  cma_t_mutex mutex;
} THREAD_LOCK_TYPE, *THREAD_LOCK_PTR;

typedef struct thread_lock_array_tag {
  int num_elements;
  cma_t_attr *mu_attr_array;
  cma_t_mutex *mutex_array;
} THREAD_LOCK_ARRAY_TYPE, *THREAD_LOCK_ARRAY_PTR;

typedef struct thread_id_tag {
  cma_t_attr master_mu_attr;
  cma_t_mutex master_mutex;
  cma_t_attr master_cv_attr;
  cma_t_cond master_cv;
  cma_t_attr mu_attr;
  cma_t_mutex mutex;
  cma_t_attr cv_attr;
  cma_t_cond cv;
  cma_t_attr thr_attr;
  cma_t_thread os_thread_id;
  float timer_tick;
  cma_t_date_time timer_exp_time;
  cma_t_date_time delta_time;
  void *application_data;
  int process_active;
} THREAD_ID_TYPE;
typedef THREAD_ID_TYPE *THREAD_ID_PTR;

#endif
