#ifndef __sys_types_h
#define __sys_types_h 1

/***
/* macros
*/

#define MAKE_FIXED_DESCRIP( list, num_bytes, d_list )\
  d_list.dsc$w_length = num_bytes;\
  d_list.dsc$b_dtype = 14;\
  d_list.dsc$b_class = 1;\
  d_list.dsc$a_pointer = list;

/***
/* types
*/

typedef struct sys_time_tag {
  LARGE_INTEGER time;
} SYS_TIME_TYPE;
typedef SYS_TIME_TYPE *SYS_TIME_PTR;

typedef struct sys_proc_id_tag {
  unsigned int id;
} SYS_PROC_ID_TYPE;
typedef SYS_PROC_ID_TYPE *SYS_PROC_ID_PTR;

/***
/* function prototypes
*/

int sys_cvt_seconds_to_timeout (
  float seconds,
  SYS_TIME_PTR timeout
);

void sys_wait_seconds (
  float *seconds
);

int sys_get_proc_id (
  SYS_PROC_ID_PTR proc_id
);

int sys_get_user_name (
  int max_chars,
  char *name
);

int sys_get_time (
  SYS_TIME_PTR time
);

#endif
