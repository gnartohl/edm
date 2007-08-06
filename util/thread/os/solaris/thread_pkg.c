#include <stdio.h>

#include "thread_priv.h"

static int g_init = 1;
static pthread_mutexattr_t g_master_mu_attr;
static pthread_mutex_t g_master_mutex;
static ptread_condattr_t g_master_cv_attr;
static pthread_cond_t g_master_cv;

static cleanupListPtr g_cleanupHead, g_cleanupTail;

static int do_init ( void ) {

int stat;

  stat = pthread_mutex_init( &g_master_mutex, NULL );
  if ( stat ) return stat;
  
  stat = pthread_cond_init( &g_master_cv, NULL );
  if ( stat ) return stat;

  g_cleanupHead = (cleanupListPtr) calloc( 1, sizeof(cleanupListType) );
  g_cleanupTail = g_cleanupHead;
  g_cleanupTail->flink = NULL;

  return 0;

}

int thread_init( void ) {

int stat;

  if ( g_init ) {
    g_init = 0;
    stat = do_init();
    if ( stat ) return UNIX_ERROR;
  }

  return THR_SUCCESS;

}

int thread_create_lock_handle (
  THREAD_LOCK_HANDLE *handle
) {

THREAD_LOCK_PTR priv_thr_lock;
int stat;

  if ( g_init ) return THR_BADSTATE;

  priv_thr_lock =
   (THREAD_LOCK_PTR) calloc( 1, sizeof(THREAD_LOCK_TYPE) );
  if ( !priv_thr_lock ) return THR_NOMEM;

  stat = pthread_mutex_init( &priv_thr_lock->mutex, NULL );
  if ( stat ) return UNIX_ERROR;

  *handle = (THREAD_LOCK_HANDLE) priv_thr_lock;

  return THR_SUCCESS;

}

int thread_lock (
  THREAD_LOCK_HANDLE handle
) {

THREAD_LOCK_PTR priv_thr_lock;
int stat;

  if ( g_init ) return THR_BADSTATE;

  priv_thr_lock = (THREAD_LOCK_PTR) handle;
  if ( !priv_thr_lock ) return THR_BADPARAM;

  stat = pthread_mutex_lock( &priv_thr_lock->mutex );
  if ( stat ) return UNIX_ERROR;

  return THR_SUCCESS;

}

int thread_unlock (
  THREAD_LOCK_HANDLE handle
) {

THREAD_LOCK_PTR priv_thr_lock;
int stat;

  if ( g_init ) return THR_BADSTATE;

  priv_thr_lock = (THREAD_LOCK_PTR) handle;
  if ( !priv_thr_lock ) return THR_BADPARAM;

  stat = pthread_mutex_unlock( &priv_thr_lock->mutex );
  if ( stat ) return UNIX_ERROR;

  return THR_SUCCESS;

}

int thread_create_lock_array_handle (
  THREAD_LOCK_ARRAY_HANDLE *handle,
  int num_locks
) {

THREAD_LOCK_ARRAY_PTR priv_thr_lock_array;
int i, stat;

  if ( g_init ) return THR_BADSTATE;

  if ( num_locks < 1 ) return THR_BADPARAM;
  if ( num_locks > 256 ) return THR_BADPARAM;

  priv_thr_lock_array =
   (THREAD_LOCK_ARRAY_PTR) calloc( 1, sizeof(THREAD_LOCK_ARRAY_TYPE) );
  if ( !priv_thr_lock_array ) return THR_NOMEM;

  priv_thr_lock_array->num_elements = num_locks;

  priv_thr_lock_array->mu_attr_array =
   (pthread_mutexattr_t *) calloc( num_locks, sizeof(pthread_mutexattr_t) );
  if ( !priv_thr_lock_array->mu_attr_array ) return THR_NOMEM;

  priv_thr_lock_array->mutex_array = (pthread_mutex_t *) calloc( num_locks,
   sizeof(pthread_mutex_t) );
  if ( !priv_thr_lock_array->mutex_array ) return THR_NOMEM;

  for ( i=0; i<num_locks; i++ ) {

    stat = pthread_mutex_init( &priv_thr_lock_array->mutex_array[i],
     NULL );
    if ( stat ) return UNIX_ERROR;

  }

  *handle = (THREAD_LOCK_ARRAY_HANDLE) priv_thr_lock_array;

  return THR_SUCCESS;

}

int thread_lock_array_element (
  THREAD_LOCK_ARRAY_HANDLE handle,
  int element
) {

THREAD_LOCK_ARRAY_PTR priv_thr_lock_array;
int stat;

  if ( g_init ) return THR_BADSTATE;

  priv_thr_lock_array = (THREAD_LOCK_ARRAY_PTR) handle;
  if ( !priv_thr_lock_array ) return THR_BADPARAM;

  if ( ( element < 0 ) || ( element >= priv_thr_lock_array->num_elements ) )
    return THR_BADPARAM;

  stat = pthread_mutex_lock( &priv_thr_lock_array->mutex_array[element] );
  if ( stat ) return UNIX_ERROR;

  return THR_SUCCESS;

}

int thread_unlock_array_element (
  THREAD_LOCK_ARRAY_HANDLE handle,
  int element
) {

THREAD_LOCK_ARRAY_PTR priv_thr_lock_array;
int stat;

  if ( g_init ) return THR_BADSTATE;

  priv_thr_lock_array = (THREAD_LOCK_ARRAY_PTR) handle;
  if ( !priv_thr_lock_array ) return THR_BADPARAM;

  if ( ( element < 0 ) || ( element >= priv_thr_lock_array->num_elements ) )
    return THR_BADPARAM;

  stat = pthread_mutex_unlock( &priv_thr_lock_array->mutex_array[element] );
  if ( stat ) return UNIX_ERROR;

  return THR_SUCCESS;

}

int thread_create_handle (
  THREAD_HANDLE *handle,
  void *app_data
) {

THREAD_ID_PTR priv_handle;
int ret_stat, stat;

  *handle = (THREAD_HANDLE) NULL;

  priv_handle = (THREAD_ID_PTR) calloc( 1, sizeof(THREAD_ID_TYPE) );
  if ( !priv_handle ) {
    ret_stat = THR_NOMEM;
    goto err_return;
  }

  priv_handle->process_active = 0;
  priv_handle->parent_detached = 0;

  if ( g_init ) {
    g_init = 0;
    stat = do_init();
    if ( stat ) return UNIX_ERROR;
  }

  memcpy( &priv_handle->master_mu_attr, &g_master_mu_attr,
   sizeof(pthread_mutexattr_t) );
  memcpy( &priv_handle->master_mutex, &g_master_mutex,
   sizeof(pthread_mutex_t) );
  memcpy( &priv_handle->master_cv_attr, &g_master_cv_attr,
   sizeof(ptread_condattr_t) );
  memcpy( &priv_handle->master_cv, &g_master_cv,
   sizeof(pthread_cond_t) );

  stat = pthread_mutex_init( &priv_handle->mutex, NULL );
  if ( stat ) return UNIX_ERROR;

  stat = pthread_cond_init( &priv_handle->cv, NULL );
  if ( stat ) return UNIX_ERROR;

  priv_handle->wantJoin = 0;

  priv_handle->application_data = app_data;

  *handle = (THREAD_HANDLE) priv_handle;

  return THR_SUCCESS;

err_return:
  return ret_stat;

}

int thread_destroy_handle (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;
int ret_stat, stat, active;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) {
    ret_stat = THR_BADPARAM;
    goto err_return;
  }

  ret_stat = THR_SUCCESS;

  stat = pthread_mutex_lock( &priv_handle->mutex );
  if ( stat ) return UNIX_ERROR;

  if ( priv_handle->process_active ) {
    stat = pthread_detach( priv_handle->os_thread_id );
    priv_handle->parent_detached = 1;
    active = 1;
  }
  else {
    active = 0;
  }

  stat = pthread_mutex_unlock( &priv_handle->mutex );
  if ( stat ) return UNIX_ERROR;

  if ( !active ) {
    thread_request_free_handle( handle ); /* this locks */
  }

  return ret_stat;

err_return:
  return ret_stat;

}

int thread_destroy_lock_handle (
  THREAD_LOCK_HANDLE handle
) {

THREAD_LOCK_PTR priv_thr_lock;
int stat, ret_stat;

  if ( g_init ) return THR_BADSTATE;

  priv_thr_lock = (THREAD_LOCK_PTR) handle;
  if ( !priv_thr_lock ) return THR_BADPARAM;

  ret_stat = THR_SUCCESS;

  thread_request_free_lock( handle );
  priv_thr_lock = NULL;

  return ret_stat;

err_return:
  return ret_stat;

}

int thread_detach (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;
int ret_stat, stat, doJoin;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) {
    ret_stat = THR_BADPARAM;
    goto err_return;
  }

  stat = pthread_mutex_lock( &priv_handle->mutex );
  if ( stat ) return UNIX_ERROR;

  doJoin = priv_handle->wantJoin;

  stat = pthread_mutex_unlock( &priv_handle->mutex );
  if ( stat ) return UNIX_ERROR;

  if ( doJoin ) { /* thread has already called thread_exit */

    thread_request_free_handle( handle ); /* this locks */

  }
  else {

    stat = pthread_mutex_lock( &priv_handle->mutex );
    if ( stat ) return UNIX_ERROR;

    stat = pthread_detach( priv_handle->os_thread_id );
    if ( stat ) {
      pthread_mutex_unlock( &priv_handle->mutex );
      ret_stat = UNIX_ERROR;
      goto err_return;
    }

    priv_handle->process_active = 0;
    priv_handle->parent_detached = 1;

    stat = pthread_mutex_unlock( &priv_handle->mutex );
    if ( stat ) return UNIX_ERROR;

  }

  return THR_SUCCESS;

err_return:
  return ret_stat;

}

void *thread_get_app_data (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;

  if ( g_init ) return (void *) NULL;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return (void *) NULL;

  return priv_handle->application_data;

}

int thread_set_stack (
  THREAD_HANDLE handle,
  int size
) {

THREAD_ID_PTR priv_handle;
int stat;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) {
    return THR_BADPARAM;
  }

  stat = pthread_attr_setstacksize( &priv_handle->thr_attr,
   (size_t) size );

  if ( !stat )
    return THR_SUCCESS;
  else
    return THR_BADPARAM;

}

int thread_get_stack (
  THREAD_HANDLE handle,
  int *size
) {

THREAD_ID_PTR priv_handle;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) {
    return THR_BADPARAM;
  }

  return THR_SUCCESS;

}

int thread_set_stack_size (
  THREAD_HANDLE handle,
  int size
) {

  return thread_set_stack( handle, size );

}

int thread_get_stack_size (
  THREAD_HANDLE handle,
  int *size
) {

  return thread_get_stack( handle, size );

}

int thread_create_proc (
  THREAD_HANDLE handle,
  void *(*proc)( void * )
) {

THREAD_ID_PTR priv_handle;
int ret_stat, stat;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) {
    ret_stat = THR_BADPARAM;
    goto err_return;
  }

  stat = pthread_create( &priv_handle->os_thread_id, NULL,
   proc, handle );
  if ( stat ) {
    ret_stat = UNIX_ERROR;
    goto err_return;
  }

  priv_handle->process_active = 1;
  priv_handle->parent_detached = 0;

  return THR_SUCCESS;

err_return:
  return ret_stat;

}

int thread_set_proc_priority (
  THREAD_HANDLE handle,
  char *priority	/* "h","m","l" */
) {

THREAD_ID_PTR priv_handle;
int stat;
struct sched_param sched;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  switch ( priority[0] ) {

    case 'h':
    case 'H':
      sched.sched_priority = sched_get_priority_max( SCHED_OTHER );
      break;

    case 'm':
    case 'M':
      sched.sched_priority =
       ( sched_get_priority_max( SCHED_OTHER ) +
         sched_get_priority_min( SCHED_OTHER ) ) / 2;
      break;

    case 'l':
    case 'L':
      sched.sched_priority = sched_get_priority_min( SCHED_OTHER );
      break;

    default:
      return THR_BADPARAM;

  }

  stat = sched_setscheduler( priv_handle->os_thread_id, SCHED_OTHER, &sched );
  if ( stat ) return UNIX_ERROR;

  return THR_SUCCESS;

}

int thread_wait_til_complete (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;
int stat;
void *child_stat;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  stat = pthread_join( priv_handle->os_thread_id, (void *) &child_stat );
  if ( stat ) return UNIX_ERROR;

  priv_handle->process_active = 0;

  return THR_SUCCESS;

}

int thread_wait_til_complete_no_block (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;
int stat, doJoin;
void *child_stat;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  stat = pthread_mutex_lock( &priv_handle->mutex );
  if ( stat ) return UNIX_ERROR;

  doJoin = priv_handle->wantJoin;

  stat = pthread_mutex_unlock( &priv_handle->mutex );
  if ( stat ) return UNIX_ERROR;

  if ( doJoin ) {

    stat = pthread_join( priv_handle->os_thread_id, (void *) &child_stat );
    if ( stat ) return UNIX_ERROR;

    priv_handle->process_active = 0;

  }
  else {

    return THR_NOJOIN;

  }

  return THR_SUCCESS;

}

int thread_lock_master (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;
int stat;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  stat = pthread_mutex_lock( &priv_handle->master_mutex );
  if ( stat ) return UNIX_ERROR;

  return THR_SUCCESS;

}

int thread_unlock_master (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;
int stat;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  stat = pthread_mutex_unlock( &priv_handle->master_mutex );
  if ( stat ) return UNIX_ERROR;

  return THR_SUCCESS;

}

int thread_lock_global ( void ) {

int stat;

  if ( g_init ) return THR_BADSTATE;

  stat = pthread_mutex_lock( &g_master_mutex );
  if ( stat ) return UNIX_ERROR;

  return THR_SUCCESS;

}

int thread_unlock_global ( void ) {

int stat;

  if ( g_init ) return THR_BADSTATE;

  stat = pthread_mutex_unlock( &g_master_mutex );
  if ( stat ) return UNIX_ERROR;

  return THR_SUCCESS;

}

int thread_wait_for_signal (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;
int stat;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  pthread_mutex_lock( &priv_handle->mutex );
  stat = pthread_cond_wait( &priv_handle->cv, &priv_handle->mutex );
  pthread_mutex_unlock( &priv_handle->mutex );

  return THR_SUCCESS;

}

int thread_timed_wait_for_signal (
  THREAD_HANDLE handle,
  double seconds
) {

THREAD_ID_PTR priv_handle;
int stat;
struct timezone tz = { 0, 0 };
struct timeval timer_exp_time;
struct timeval delta_time;
struct timespec posix_time;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  delta_time.tv_sec = (int) seconds;
  delta_time.tv_usec =
   (int) ( ( seconds - (double) delta_time.tv_sec ) * 1.0e6 );

  stat = gettimeofday( &timer_exp_time, &tz );
  if ( stat ) return UNIX_ERROR;

  timeradd( (&timer_exp_time), (&delta_time), (&timer_exp_time) );

  TIMEVAL_TO_TIMESPEC( (&timer_exp_time), (&posix_time) );

  pthread_mutex_lock( &priv_handle->mutex );
  stat = pthread_cond_timedwait( &priv_handle->cv, &priv_handle->mutex,
   &posix_time );
  pthread_mutex_unlock( &priv_handle->mutex );
  if ( stat == ETIMEDOUT ) return THR_TIMEOUT;
  if ( stat != 0 ) return THR_FAIL;

  return THR_SUCCESS;

}

int thread_signal (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  pthread_cond_signal( &priv_handle->cv );

  return THR_SUCCESS;

}

int thread_signal_from_ast (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  pthread_cond_signal( &priv_handle->cv );

  return THR_SUCCESS;

}

int thread_delay (
  void *handle,
  double seconds
) {

THREAD_ID_PTR priv_handle;
int ret_stat, stat, sec;
struct timezone tz = { 0, 0 };
struct timespec  posix_time;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  stat = pthread_mutex_lock( &priv_handle->mutex );
  if ( stat ) return UNIX_ERROR;

  sec = (int) seconds;
  priv_handle->delta_time.tv_sec = sec;
  priv_handle->delta_time.tv_usec =
   (int) ( ( seconds - (double) sec ) * 1.0e6 );

  stat = gettimeofday( &priv_handle->timer_exp_time, &tz );
  if ( stat ) {
    ret_stat = UNIX_ERROR;
    goto err_return;
  }

  timeradd( (&priv_handle->timer_exp_time), (&priv_handle->delta_time),
   (&priv_handle->timer_exp_time) );
  if ( stat ) {
    ret_stat = UNIX_ERROR;
    goto err_return;
  }

  TIMEVAL_TO_TIMESPEC( (&priv_handle->timer_exp_time), (&posix_time) );

  pthread_cond_timedwait( &priv_handle->cv, &priv_handle->mutex, &posix_time );

  stat = pthread_mutex_unlock( &priv_handle->mutex );
  if ( stat ) {
    ret_stat = UNIX_ERROR;
    goto err_return_no_unlock;
  }

  return THR_SUCCESS;

err_return:
  stat = pthread_mutex_unlock( &priv_handle->mutex );
  return ret_stat;

err_return_no_unlock:
  return ret_stat;

}

int thread_init_timer (
  THREAD_HANDLE handle,
  double seconds
) {

THREAD_ID_PTR priv_handle;
int stat;
struct timezone tz = { 0, 0 };

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  priv_handle->timer_tick = seconds;
  priv_handle->delta_time.tv_sec = (int) seconds;
  priv_handle->delta_time.tv_usec =
   (int) ( ( seconds - (double) priv_handle->delta_time.tv_sec ) * 1.0e6 );

  stat = gettimeofday( &priv_handle->timer_exp_time, &tz );
  if ( stat ) return UNIX_ERROR;

  timeradd( (&priv_handle->timer_exp_time), (&priv_handle->delta_time),
   (&priv_handle->timer_exp_time) );

  return THR_SUCCESS;

}

int thread_wait_for_timer (
  THREAD_HANDLE handle
) {

struct timespec posix_time;
THREAD_ID_PTR priv_handle;
int ret_stat, stat;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  stat = pthread_mutex_lock( &priv_handle->mutex );
  if ( stat ) return UNIX_ERROR;

  TIMEVAL_TO_TIMESPEC( (&priv_handle->timer_exp_time), (&posix_time) );

  pthread_cond_timedwait( &priv_handle->cv, &priv_handle->mutex, &posix_time );

  timeradd( (&priv_handle->timer_exp_time), (&priv_handle->delta_time),
   (&priv_handle->timer_exp_time) );
  if ( stat ) {
    ret_stat = UNIX_ERROR;
    goto err_return;
  }

  stat = pthread_mutex_unlock( &priv_handle->mutex );
  if ( stat ) {
    ret_stat = UNIX_ERROR;
    goto err_return_no_unlock;
  }

  return THR_SUCCESS;

err_return:
  stat = pthread_mutex_unlock( &priv_handle->mutex );
  return ret_stat;

err_return_no_unlock:
  return ret_stat;

}

int thread_exit(
  THREAD_HANDLE handle,
  void *retval
) {

THREAD_ID_PTR priv_handle;
int stat, detached;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  stat = pthread_mutex_lock( &priv_handle->mutex );
  if ( stat ) return UNIX_ERROR;

  priv_handle->wantJoin = 1;
  detached = priv_handle->parent_detached;

  stat = pthread_mutex_unlock( &priv_handle->mutex );
  if ( stat ) return UNIX_ERROR;

  if ( detached ) {
    thread_request_free_handle( handle ); /* this locks */
  }

  pthread_exit( retval ); /* this never returns */

  return THR_SUCCESS; /* so compiler won't complain */

}

int thread_detached_exit(
  THREAD_HANDLE handle,
  void *retval
) {

THREAD_ID_PTR priv_handle;
int stat;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  thread_request_free_handle( handle );

  pthread_exit( retval ); /* this never returns */

  return THR_SUCCESS; /* so compiler won't complain */

}

int thread_request_free(
  int ptrType,
  void *_ptr
) {

cleanupListPtr cur;
int stat;

  if ( g_init ) return THR_BADSTATE;

  stat = pthread_mutex_lock( &g_master_mutex );
  if ( stat ) return UNIX_ERROR;

  cur = (cleanupListPtr) malloc( sizeof(cleanupListType) );
  if ( cur ) {
    cur->ptrType = ptrType;
    cur->ptr = _ptr;
    g_cleanupTail->flink = cur;
    g_cleanupTail = cur;
    g_cleanupTail->flink = NULL;
  }
  else {
    stat = pthread_mutex_unlock( &g_master_mutex );
    return THR_NOMEM;
  }

  stat = pthread_mutex_unlock( &g_master_mutex );
  if ( stat ) return UNIX_ERROR;

  return THR_SUCCESS;

}

int thread_request_free_handle(
  THREAD_HANDLE handle
) {

int stat;

  stat = thread_request_free( THREAD_PTR_TYPE_HANDLE, (void *) handle );

  return stat;

}

int thread_request_free_lock(
  THREAD_LOCK_HANDLE handle
) {

int stat;

  stat = thread_request_free( THREAD_PTR_TYPE_LOCK, (void *) handle );

  return stat;

}

int thread_request_free_ptr(
  void* ptr
) {

int stat;

  stat = thread_request_free( THREAD_PTR_TYPE_GENERIC, ptr );

  return stat;

}

int thread_cleanup_from_main_thread_only( void ) {

/* This function must be called from the main thread only */

cleanupListPtr cur, next;
int stat, n=0;
THREAD_ID_PTR priv_handle;
THREAD_LOCK_PTR priv_thr_lock;

  if ( g_init ) return THR_BADSTATE;

  stat = pthread_mutex_lock( &g_master_mutex );
  if ( stat ) return UNIX_ERROR;

  cur = g_cleanupHead->flink;
  while ( cur ) {

    next = cur->flink;

    g_cleanupHead->flink = next;

    if ( cur->ptr ) {

      switch ( cur->ptrType ) {

      case THREAD_PTR_TYPE_HANDLE:
        priv_handle = (THREAD_ID_PTR) cur->ptr;
        pthread_mutex_destroy( &priv_handle->mutex );
        pthread_cond_destroy( &priv_handle->cv );
        free( priv_handle );
        break;

      case THREAD_PTR_TYPE_LOCK:
        priv_thr_lock = (THREAD_LOCK_PTR) cur->ptr;
        pthread_mutex_destroy( &priv_thr_lock->mutex );
        free( priv_thr_lock );
        break;

      case THREAD_PTR_TYPE_GENERIC:
      default:
        free( cur->ptr );
        break;

      }

    }

    free( cur );
    n++;
    cur = next;

  }

  g_cleanupTail = g_cleanupHead;
  g_cleanupTail->flink = NULL;

  stat = pthread_mutex_unlock( &g_master_mutex );
  if ( stat ) return UNIX_ERROR;

  return THR_SUCCESS;

}
