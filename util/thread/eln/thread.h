#ifndef __thread_h
#define __thread_h 1

#define THREAD_SUCCESS( stat )\
  return stat;

#define THREAD_ERROR( stat )\
  return stat;

typedef void *THREAD_HANDLE;
typedef void *THREAD_LOCK_ARRAY_HANDLE;
typedef void *THREAD_LOCK_HANDLE;

globalvalue THR_SUCCESS;
globalvalue THR_NOMEM;
globalvalue THR_BADPARAM;
globalvalue THR_BADSTATE;

#endif
