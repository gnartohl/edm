#include stdio
#include sys_types
#include thread_priv

static int g_init = 1;
static _align(LONGWORD) MUTEX g_master_mutex;
static EVENT g_master_cv;

static void do_init ( void ) {

int stat;

  ELN$CREATE_MUTEX( g_master_mutex, &stat );
  ker$create_event( &stat, &g_master_cv, EVENT$CLEARED );

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

  ELN$CREATE_MUTEX( priv_thr_lock->mutex, &stat );
  if ( !( stat & 1 ) ) return stat;

  *handle = (THREAD_LOCK_HANDLE) priv_thr_lock;

  return THR_SUCCESS;

}

int thread_lock (
  THREAD_LOCK_HANDLE handle
) {

THREAD_LOCK_PTR priv_thr_lock;

  if ( g_init ) return THR_BADSTATE;

  priv_thr_lock = (THREAD_LOCK_PTR) handle;
  if ( !priv_thr_lock ) return THR_BADPARAM;

  ELN$LOCK_MUTEX( priv_thr_lock->mutex );

  return THR_SUCCESS;

}

int thread_unlock (
  THREAD_LOCK_HANDLE handle
) {

THREAD_LOCK_PTR priv_thr_lock;

  if ( g_init ) return THR_BADSTATE;

  priv_thr_lock = (THREAD_LOCK_PTR) handle;
  if ( !priv_thr_lock ) return THR_BADPARAM;

  ELN$UNLOCK_MUTEX( priv_thr_lock->mutex );

  return THR_SUCCESS;

}

int thread_create_lock_array_handle (
  THREAD_LOCK_ARRAY_HANDLE *handle,
  int num_locks
) {

THREAD_LOCK_ARRAY_PTR priv_thr_lock_array;
int stat;
int i;

  if ( g_init ) return THR_BADSTATE;

  if ( num_locks < 1 ) return THR_BADPARAM;
  if ( num_locks > 256 ) return THR_BADPARAM;

  priv_thr_lock_array =
   (THREAD_LOCK_ARRAY_PTR) calloc( 1, sizeof(THREAD_LOCK_ARRAY_TYPE) );
  if ( !priv_thr_lock_array ) return THR_NOMEM;

  priv_thr_lock_array->num_elements = num_locks;

  priv_thr_lock_array->mutex_array = (MUTEX *) calloc( num_locks,
   sizeof(MUTEX) );
  if ( !priv_thr_lock_array->mutex_array ) return THR_NOMEM;

  for ( i=0; i<num_locks; i++ ) {

    ELN$CREATE_MUTEX( priv_thr_lock_array->mutex_array[i], &stat );
    if ( !( stat & 1 ) ) return stat;

  }

  *handle = (THREAD_LOCK_ARRAY_HANDLE) priv_thr_lock_array;

  return THR_SUCCESS;

}

int thread_lock_array_element (
  THREAD_LOCK_ARRAY_HANDLE handle,
  int element
) {

THREAD_LOCK_ARRAY_PTR priv_thr_lock_array;

  if ( g_init ) return THR_BADSTATE;

  priv_thr_lock_array = (THREAD_LOCK_ARRAY_PTR) handle;
  if ( !priv_thr_lock_array ) return THR_BADPARAM;

  if ( ( element < 0 ) || ( element >= priv_thr_lock_array->num_elements ) )
    return THR_BADPARAM;

  ELN$LOCK_MUTEX( priv_thr_lock_array->mutex_array[element] );

  return THR_SUCCESS;

}

int thread_unlock_array_element (
  THREAD_LOCK_ARRAY_HANDLE handle,
  int element
) {

THREAD_LOCK_ARRAY_PTR priv_thr_lock_array;

  if ( g_init ) return THR_BADSTATE;

  priv_thr_lock_array = (THREAD_LOCK_ARRAY_PTR) handle;
  if ( !priv_thr_lock_array ) return THR_BADPARAM;

  if ( ( element < 0 ) || ( element >= priv_thr_lock_array->num_elements ) )
    return THR_BADPARAM;

  ELN$UNLOCK_MUTEX( priv_thr_lock_array->mutex_array[element] );

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

  if ( g_init ) {
    g_init = 0;
    do_init();
  }

  memcpy( &priv_handle->master_mutex, &g_master_mutex, sizeof(MUTEX) );
  memcpy( &priv_handle->master_cv, &g_master_cv, sizeof(EVENT) );

  ELN$CREATE_MUTEX( priv_handle->mutex, &stat );
  if ( !( stat & 1 ) ) {
    ret_stat = stat;
    goto err_return;
  }

  ker$create_event( &stat, &priv_handle->cv, EVENT$CLEARED );
  if ( !( stat & 1 ) ) {
    ret_stat = stat;
    goto err_return;
  }

  priv_handle->application_data = app_data;

  *handle = (THREAD_HANDLE) priv_handle;

norm_return:
  return THR_SUCCESS;

err_return:
  return ret_stat;

}

int thread_destroy_handle (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;
int ret_stat, stat;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) {
    ret_stat = THR_BADPARAM;
    goto err_return;
  }

  ELN$DELETE_MUTEX( priv_handle->mutex, &stat );
  if ( !( stat & 1 ) ) {
    ret_stat = stat;
    goto err_return;
  }

  ker$delete( &stat, priv_handle->cv );
  if ( !( stat & 1 ) ) {
    ret_stat = stat;
    goto err_return;
  }

  free( priv_handle );

norm_return:
  return THR_SUCCESS;

err_return:
  return ret_stat;

}

void *thread_get_app_data (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;
int ret_stat;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return (void *) NULL;

  return priv_handle->application_data;

}

int thread_create_proc (
  THREAD_HANDLE handle,
  void (*proc)()
) {

THREAD_ID_PTR priv_handle;
int ret_stat, stat;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) {
    ret_stat = THR_BADPARAM;
    goto err_return;
  }

  ker$create_process( &stat, &priv_handle->os_thread_id, proc,
   &priv_handle->exit_status, handle );
  if ( !( stat & 1 ) ) {
    ret_stat = stat;
    goto err_return;
  }

norm_return:
  return THR_SUCCESS;

err_return:
  return ret_stat;

}

int thread_set_proc_priority (
  THREAD_HANDLE handle,
  char *priority        /* "h","m","l" */
) {

THREAD_ID_PTR priv_handle;
int prior;
int ret_stat, stat;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) {
    ret_stat = THR_BADPARAM;
    goto err_return;
  }

  switch ( priority[0] ) {

    case 'h':
    case 'H':
      prior = 5;
      break;

    case 'm':
    case 'M':
      prior = 8;
      break;

    case 'l':
    case 'L':
      prior = 11;
      break;

    default:
      if ( ( priority[0] >= 0 ) && ( priority[0] <=15 ) )
        prior = priority[0];
      else {
        ret_stat = THR_BADPARAM;
        goto err_return;
      }

  }

  ker$set_process_priority( &stat, priv_handle->os_thread_id, prior );
  if ( !( stat & 1 ) ) {
    ret_stat = stat;
    goto err_return;
  }

norm_return:
  return THR_SUCCESS;

err_return:
  return ret_stat;

}

int thread_wait_til_complete (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;
int result, stat;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;


  ker$wait_any( &stat, &result, NULL, priv_handle->os_thread_id );

  return stat;

}

int thread_lock_master (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  ELN$LOCK_MUTEX( priv_handle->master_mutex );

  return THR_SUCCESS;

}

int thread_unlock_master (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  ELN$UNLOCK_MUTEX( priv_handle->master_mutex );

  return THR_SUCCESS;

}

int thread_lock_global ( void ) {

  if ( g_init ) return THR_BADSTATE;

  ELN$LOCK_MUTEX( g_master_mutex );

  return THR_SUCCESS;

}

int thread_unlock_global ( void ) {

  if ( g_init ) return THR_BADSTATE;

  ELN$UNLOCK_MUTEX( g_master_mutex );

  return THR_SUCCESS;

}

int thread_wait_for_signal (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;
int result, stat;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  ker$wait_any( &stat, &result, NULL, priv_handle->cv );
  if ( !( stat & 1 ) ) return stat;

  ker$clear_event( &stat, priv_handle->cv );
  if ( !( stat & 1 ) ) return stat;

  return THR_SUCCESS;

}

int thread_signal (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;
int stat;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  ker$signal( &stat, priv_handle->cv );
  if ( !( stat & 1 ) ) return stat;

  return THR_SUCCESS;

}

int thread_delay (
  THREAD_HANDLE handle,
  double seconds
) {

THREAD_ID_PTR priv_handle;
int stat;
float fsec;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  fsec = (float) seconds;
  sys_wait_seconds( &fsec );

  return THR_SUCCESS;

}

int thread_init_timer (
  THREAD_HANDLE handle,
  double seconds
) {

THREAD_ID_PTR priv_handle;
int stat;
float fsec;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  fsec = (float) seconds;
  stat = sys_cvt_seconds_to_timeout( fsec, &priv_handle->delta_time );
  if ( !( stat & 1 ) ) return stat;

  stat = sys_get_time( &priv_handle->timer_exp_time );
  if ( !( stat & 1 ) ) return stat;

  stat = sys_add_times( &priv_handle->timer_exp_time, &priv_handle->delta_time );
  if ( !( stat & 1 ) ) return stat;

  return THR_SUCCESS;

}

int thread_wait_for_timer (
  THREAD_HANDLE handle
) {

THREAD_ID_PTR priv_handle;
int result, stat;

  if ( g_init ) return THR_BADSTATE;

  priv_handle = (THREAD_ID_PTR) handle;
  if ( !priv_handle ) return THR_BADPARAM;

  ker$wait_any( &stat, &result, &priv_handle->timer_exp_time );
  if ( !( stat & 1 ) ) return stat;

  stat = sys_add_times( &priv_handle->timer_exp_time, &priv_handle->delta_time );
  if ( !( stat & 1 ) ) return stat;

  return THR_SUCCESS;

}
