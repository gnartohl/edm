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

typedef struct {
  short stat;
  short count;
  int dev_specific_info;
} IOSB_TYPE;

typedef struct sys_time_tag {
  int high;
  unsigned int low;
} SYS_TIME_TYPE;
typedef SYS_TIME_TYPE *SYS_TIME_PTR;

typedef struct sys_proc_id_tag {
  unsigned int id;
} SYS_PROC_ID_TYPE;
typedef SYS_PROC_ID_TYPE *SYS_PROC_ID_PTR;

/***
/* function prototypes
*/

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

int sys_get_time_diff_in_hours (
  SYS_TIME_PTR cur_time,
  SYS_TIME_PTR time,
  float *hours
);

int sys_cvt_hours_to_time (
  float hours,
  SYS_TIME_PTR time
);

int sys_cvt_seconds_to_timeout (
  float seconds,
  SYS_TIME_PTR time
);

int sys_cvt_time_to_string (
  SYS_TIME_PTR time,
  int string_size,
  char *string
);

int sys_cvt_string_to_time (
  char *string,
  int string_size,
  SYS_TIME_PTR time
);

#endif
