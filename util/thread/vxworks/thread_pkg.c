#include <stdio.h>
#include <string.h>

#include <time.h>
#include <sys/times.h>
#include <taskVarLib.h>
#include <taskLib.h>
#include <wdLib.h>
#include <drv/timer/CIOTimer.h>

#include "sys_types.h"
#include "thread_priv.h"

static int g_thread_init = 1;
static SEM_ID g_thread_master_mutex;
static MSG_Q_ID g_thread_master_cv;

static void do_init ( void ) {

  g_thread_master_mutex = semBCreate( SEM_Q_PRIORITY, SEM_FULL );

  g_thread_master_cv = msgQCreate( 1, 1, MSG_Q_PRIORITY ); /* use as signal */

}

int thread_init( void ) {

  taskLock();
  if ( g_thread_init ) {
    g_thread_init = 0;
    do_init();
  }
  taskUnlock();

  return THR_SUCCESS;

}

int thread_create_lock_handle (
  THREAD_LOCK_HANDLE *handle
) {

THREAD_LOCK_PTR priv_thr_lock;

  if ( g_thread_init ) return THR_BADSTATE;

  priv_thr_lock =
   (THREAD_LOCK_PTR) calloc( 1, sizeof(THREAD_LOCK_TYPE) );
  if ( !priv_thr_lock ) return THR_NOMEM;

   priv_thr_lock->mutex = semBCreate( SEM_Q_PRIORITY, SEM_FULL );

  *handle = (THREAD_LOCK_HANDLE) priv_thr_lock;

  return THR_SUCCESS;

}

int thread_lock (
  THREAD_LOCK_HANDLE handle
) {

THREAD_LOCK_PTR priv_thr_lock;

  if ( g_thread_init ) return THR_BADSTATE;

  priv_thr_lock = (THREAD_LOCK_PTR) handle;
  if ( !priv_thr_lock ) return THR_BADPARAM;

  semTake( priv_thr_lock->mutex, WAIT_FOREVER );

  return THR_SUCCESS;

}

int thread_unlock (
  THREAD_LOCK_HANDLE handle
) {

THREAD_LOCK_PTR priv_thr_lock;

  if ( g_thread_init ) return THR_BADSTATE;

  priv_thr_lock = (THREAD_LOCK_PTR) handle;
  if ( !priv_thr_lock ) return THR_BADPARAM;

  semGive( priv_thr_lock->mutex );

  return THR_SUCCESS;

}

int thread_create_lock_array_handle (
  THREAD_LOCK_ARRAY_HANDLE *handle,
  int num_locks
) {

THREAD_LOCK_ARRAY_PTR priv_thr_lock_array;
int i;

  if ( g_thread_init ) return THR_BADSTATE;

  if ( num_locks < 1 ) return THR_BADPARAM;
  if ( num_locks > 256 ) return THR_BADPARAM;

  priv_thr_lock_array =
   (THREAD_LOCK_ARRAY_PTR) calloc( 1, sizeof(THREAD_LOCK_ARRAY_TYPE) );
  if ( !priv_thr_lock_array ) return THR_NOMEM;

  priv_thr_lock_array->num_elements = num_locks;

  priv_thr_lock_array->mutex_array = (SEM_ID *) calloc( num_locks,
   sizeof(SEM_ID) );
  if ( !priv_thr_lock_array->mutex_array ) return THR_NOMEM;

  for ( i=0; i<num_locks; i++ ) {

    priv_thr_lock_array->mutex_array[i] =
     semBCreate( SEM_Q_PRIORITY, SEM_FULL );

  }

  *handle = (THREAD_LOCK_ARRAY_HANDLE) priv_thr_lock_array;

  return THR_SUCCESS;

}

int thread_lock_array_element (
  THREAD_LOCK_ARRAY_HANDLE handle,
  int element
) {

THREAD_LOCK_ARRAY_PTR priv_thr_lock_array;

  if ( g_thread_init ) return THR_BADSTATE;

  priv_thr_lock_array = (THREAD_LOCK_ARRAY_PTR) handle;
  if ( !priv_thr_lock_array ) return THR_BADPARAM;

  if ( ( element < 0 ) || ( element >= priv_thr_lock_array->num_elements ) )
    return THR_BADPARAM;

  semTake( priv_thr_lock_array->mutex_array[element], WAIT_FOREVER );

  return THR_SUCCESS;

}

int thread_unlock_array_element (
  THREAD_LOCK_ARRAY_HANDLE handle,
  int element
) {

THREAD_LOCK_ARRAY_PTR priv_thr_lock_array;

  if ( g_thread_init ) return THR_BADSTATE;

  priv_thr_lock_array = (THREAD_LOCK_ARRAY_PTR) handle;
  if ( !priv_thr_lock_array ) return THR_BADPARAM;

  if ( ( element < 0 ) || ( element >= priv_thr_lock_array->num_elements ) )
    return THR_BADPARAM;

  semGive( priv_thr_lock_array->mutex_array[element] );

  return THR_SUCCESS;

}

int thread_create_handle (
  THREAD_HANDLE *handle,
  void *app_data
) {

THREAD_ID_PTR priv_handle;
int ret_stat;

  *handle = (THREAD_HANDLE) NULL;

  priv_handle = (THREAD_ID_PTR) calloc( 1, sizeof(THREAD_ID_TYPE) );
  if ( !priv_handle ) {
    ret_stat = THR_NOMEM;
    goto err_return;
  }

  taskLock();
  if ( g_thread_init ) {
    g_thread_init = 0;
    do_init();
  }
  taskUnlock();

  priv_handle->name = NULL;

  priv_handle->stack_size = THR_DEF_STACK_SIZE;

  priv_handle->prior = THR_BASE_PRIOR;

  memcpy( &priv_handle->master_mutex, &g_thread_master_mutex,
   sizeof(g_thread_master_mutex) );
  memcpy( &priv_handle->master_cv, &g_thread_master_cv,
   sizeof(g_thread_master_cv) );

  priv_handle->mutex = semBCreate( SEM_Q_PRIORITY, SEM_FULL );

  priv_handle->timer_mutex = semBCreate( SEM_Q_PRIORITY, SEM_FULL );

  priv_handle->cv = msgQCreate( 1, 1, MSG_Q_PRIORITY ); /* use as signal */

  priv_handle->wdog_id = wdCreate();
  priv_handle->wdog_state = 0;

  priv_handle->application_data = app_data;

  *handle = (THREAD_HANDLE) priv_handle;

  return THR_SUCCESS;

err_return:
  return ret_stat;

}

int thread_set_name (
  THREAD_HANDLE handle,
  char *name
) {

THREAD_ID_PTR priv_handle;
int ret_stat, l;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) {
    ret_stat = THR_NOMEM;
    goto err_return;
  }

  l = strlen(name);
  if ( l > 255 ) l = 255;
  priv_handle->name = (char *) calloc( 1, l+1 );

  strncpy( priv_handle->name, name, l );

  return THR_SUCCESS;

err_return:
  return ret_stat;

}

int thread_set_stack_size (
  THREAD_HANDLE handle,
  int size
) {

THREAD_ID_PTR priv_handle;
int ret_stat;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) {
    ret_stat = THR_NOMEM;
    goto err_return;
  }

  priv_handle->stack_size = size;

  return THR_SUCCESS;

err_return:
  return ret_stat;

}

int thread_destroy_handle (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;
int ret_stat;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) {
    ret_stat = THR_BADPARAM;
    goto err_return;
  }

  semDelete( priv_handle->mutex );

  msgQDelete( priv_handle->cv );

  free( priv_handle );

  return THR_SUCCESS;

err_return:
  return ret_stat;

}

void *thread_get_app_data (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;

  if ( g_thread_init ) return (void *) NULL;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return (void *) NULL;

  return priv_handle->application_data;

}
#if 0

int vsys_get_pid();
void vsys_set_pid( long, int );

int thread_make_vxworks_task_vsys_child (	/* VxWorks only */
  THREAD_HANDLE handle
) {

int ret_stat, vsys_pid;
THREAD_ID_PTR priv_handle;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) {
    ret_stat = THR_BADPARAM;
    goto err_return;
  }

  vsys_pid = vsys_get_pid();
  vsys_set_pid( priv_handle->os_thread_id, vsys_pid );

  return THR_SUCCESS;

err_return:
  return ret_stat;

}
#endif

int thread_add_vxworks_task_var (	/* VxWorks only */
  THREAD_HANDLE handle,
  int *task_variable
) {

int stat, ret_stat;
THREAD_ID_PTR priv_handle;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) {
    ret_stat = THR_BADPARAM;
    goto err_return;
  }

  stat = taskLock();    /* disable scheduling */
  if ( stat != OK ) {
    ret_stat = UNIX_ERROR;
    goto err_return;
  }

  stat = taskVarAdd( priv_handle->os_thread_id, task_variable );
  if ( stat != OK ) {
    ret_stat = UNIX_ERROR;
    goto err_return;
  }

  stat = taskUnlock();  /* enable scheduling */
  if ( stat != OK ) {
    ret_stat = UNIX_ERROR;
    goto err_return;
  }

  return THR_SUCCESS;

err_return:
  return ret_stat;

}

int thread_create_proc (
  THREAD_HANDLE handle,
  void (*proc)()
) {

THREAD_ID_PTR priv_handle;
int ret_stat;

  if ( g_thread_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) {
    ret_stat = THR_BADPARAM;
    goto err_return;
  }

  priv_handle->os_thread_id = taskSpawn( priv_handle->name, priv_handle->prior,
   VX_FP_TASK, priv_handle->stack_size, (FUNCPTR) proc, (int) handle, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL );

  if ( priv_handle->os_thread_id == ERROR ) {
    ret_stat = UNIX_ERROR;
    goto err_return;
  }

  return THR_SUCCESS;

err_return:
  return ret_stat;

}

int thread_set_proc_priority (
  THREAD_HANDLE handle,
  char *priority	/* "h","m","l" */
) {

THREAD_ID_PTR priv_handle;
int value, ret_stat;

  if ( g_thread_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) {
    ret_stat = THR_BADPARAM;
    goto err_return;
  }

  switch ( priority[0] ) {

    case 'h':
    case 'H':
      priv_handle->prior = THR_HIGH_PRIOR;
      break;

    case 'm':
    case 'M':
      priv_handle->prior = THR_BASE_PRIOR;
      break;

    case 'l':
    case 'L':
      priv_handle->prior = THR_LOW_PRIOR;
      break;

    case '/':
      value = atoi( &priority[1] );
      priv_handle->prior = value;
      break;

    default:
      ret_stat = THR_BADPARAM;
      goto err_return;

  }

  return THR_SUCCESS;

err_return:
  return ret_stat;

}

int thread_wait_til_complete (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;
int stat, stat1;
struct timespec timer_exp_time, remaining;

  if ( g_thread_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  timer_exp_time.tv_sec = 5; /* 5 seconds */
  timer_exp_time.tv_nsec = 0;

  do {

    stat = taskIdVerify( priv_handle->os_thread_id );

    if ( stat == OK ) {
      stat1 = nanosleep( &timer_exp_time, &remaining );
      if ( stat1 == ERROR ) return UNIX_ERROR;
    }

  } while ( stat == OK );

  return THR_SUCCESS;

}

int thread_lock_master (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;

  if ( g_thread_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  semTake( priv_handle->master_mutex, WAIT_FOREVER );

  return THR_SUCCESS;

}

int thread_unlock_master (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;

  if ( g_thread_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  semGive( priv_handle->master_mutex );

  return THR_SUCCESS;

}

int thread_lock_global ( void ) {

  if ( g_thread_init ) return THR_BADSTATE;

  semTake( g_thread_master_mutex, WAIT_FOREVER );

  return THR_SUCCESS;

}

int thread_unlock_global ( void ) {

  if ( g_thread_init ) return THR_BADSTATE;

  semGive( g_thread_master_mutex );

  return THR_SUCCESS;

}

int thread_wait_for_signal (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;
char buf[4];
int stat;

  if ( g_thread_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  stat = msgQReceive( priv_handle->cv, buf, 1, WAIT_FOREVER );

  if ( stat == ERROR ) return UNIX_ERROR;

  return THR_SUCCESS;

}

int thread_signal (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;
char buf[4];
int stat;

  if ( g_thread_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  stat = msgQSend( priv_handle->cv, buf, 1, NO_WAIT, MSG_PRI_NORMAL );

  if ( stat == ERROR ) return UNIX_ERROR;

  return THR_SUCCESS;

}

int thread_signal_from_ast (
THREAD_HANDLE handle
) {

int stat;

  stat = thread_signal( handle );
  return stat;

}

int thread_delay (
  THREAD_HANDLE handle,
  double seconds
) {

THREAD_ID_PTR priv_handle;
struct timespec remaining;
int stat;

  if ( g_thread_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  semTake( priv_handle->mutex, WAIT_FOREVER );

  priv_handle->timer_exp_time.tv_sec = (int) seconds;
  priv_handle->timer_exp_time.tv_nsec =
   (int) ( ( seconds - (double) priv_handle->timer_exp_time.tv_sec ) *
   1000000000.0 + 0.5 );

  stat = nanosleep( &priv_handle->timer_exp_time, &remaining );

  semGive( priv_handle->mutex );

  return THR_SUCCESS;

}

static int thread_sig_from_wdog (
  int handle
) {

THREAD_ID_PTR priv_handle;

  priv_handle = (THREAD_ID_PTR) handle;

  wdStart( priv_handle->wdog_id, priv_handle->timer_ticks,
   thread_sig_from_wdog, (int) priv_handle );

  semGive( priv_handle->timer_mutex );

  return THR_SUCCESS;

}

int thread_init_timer (
  THREAD_HANDLE handle,
  double seconds
) {

THREAD_ID_PTR priv_handle;

  if ( g_thread_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  priv_handle->seconds = seconds;
  priv_handle->timer_ticks = (int) ( sysClkRateGet() * seconds );

  priv_handle->delta_time.tv_sec = (int) seconds;
  priv_handle->delta_time.tv_nsec =
   (int) ( ( seconds - (double) priv_handle->timer_exp_time.tv_sec ) *
   1000000000.0 + 0.5 );

  if ( priv_handle->wdog_state ) {
    priv_handle->wdog_state = 0;
    wdCancel( priv_handle->wdog_id );
  }

  semTake( priv_handle->timer_mutex, 1 );

  wdStart( priv_handle->wdog_id, priv_handle->timer_ticks,
   thread_sig_from_wdog, (int) priv_handle );

  return THR_SUCCESS;

}

int thread_wait_for_timer (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;

  if ( g_thread_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  semTake( priv_handle->timer_mutex, WAIT_FOREVER );

  return THR_SUCCESS;

}
