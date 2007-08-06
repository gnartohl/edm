#ifndef __thread_h
#define __thread_h 1

#define THREAD_SUCCESS( stat )\
  return stat;

#define THREAD_ERROR( stat )\
  cma_thread_exit_error();

typedef void *THREAD_HANDLE;
typedef void *THREAD_LOCK_ARRAY_HANDLE;
typedef void *THREAD_LOCK_HANDLE;

globalvalue THR_SUCCESS;
globalvalue THR_NOMEM;
globalvalue THR_BADPARAM;
globalvalue THR_BADSTATE;

/*
** prototypes
*/
int thread_init( void );

int thread_create_handle (
  THREAD_HANDLE *handle,
  void *app_data
);

int thread_destroy_handle (
  THREAD_HANDLE handle
);

int thread_destroy_lock_handle (
  THREAD_LOCK_HANDLE handle
);

void *thread_get_app_data (
  THREAD_HANDLE handle
);

int thread_create_proc (
  THREAD_HANDLE handle,
  void (*proc)() );

int thread_set_proc_priority (
  THREAD_HANDLE handle,
  char *priority	/* "h","m","l" */
);

int thread_wait_til_complete (
  THREAD_HANDLE handle
);

int thread_lock_master (
  THREAD_HANDLE handle
);

int thread_unlock_master (
  THREAD_HANDLE handle
);

int thread_lock_global ( void );

int thread_unlock_global ( void );

int thread_wait_for_signal (
  THREAD_HANDLE handle
);

int thread_signal (
  THREAD_HANDLE handle
);

int thread_delay (
  THREAD_HANDLE handle,
  double seconds
);

int thread_init_timer (
  THREAD_HANDLE handle,
  double seconds
);

int thread_wait_for_timer (
  THREAD_HANDLE handle
);

#endif
