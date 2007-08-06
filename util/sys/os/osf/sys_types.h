#ifndef __sys_types_h
#define __sys_types_h 1

#include "unix.msh"
#include "sys.msh"

/***
* macros
*/

/***
* types
*/

typedef struct gen_node_tag {
  struct gen_node_tag *flink;
  struct gen_node_tag *blink;
} GEN_NODE_TYPE, *GEN_NODE_PTR;

typedef struct gen_queue_tag {
  struct gen_node_tag *flink;
  struct gen_node_tag *blink;
  void *lock;			/* becomes a thread_lock_handle */
} GEN_QUEUE_TYPE, *GEN_QUEUE_PTR;

typedef struct {
  short stat;
  short count;
  int dev_specific_info;
} IOSB_TYPE;

typedef struct sys_time_tag {
  struct timeval timeval_time;
  time_t cal_time;
  struct tm tm_time;
} SYS_TIME_TYPE;
typedef SYS_TIME_TYPE *SYS_TIME_PTR;

typedef struct sys_proc_id_tag {
  pid_t id;
} SYS_PROC_ID_TYPE;
typedef SYS_PROC_ID_TYPE *SYS_PROC_ID_PTR;

#ifdef __cplusplus
#extern "C" {
#endif

/***
* function prototypes
*/

char *Strncat(
  char *dest,
  char *src,
  int max );

int sys_iniq (
  void *queue
);

int sys_destroyq (
  void *queue
);

int sys_insqt (
  void *node,
  void *queue,
  int flag
);

int sys_remqh (
  void *queue,
  void **node,
  int flag
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

int sys_get_datetime_string (
  int string_size,
  char *string
);

int sys_cvt_string_to_time (
  char *string,
  int string_size,
  SYS_TIME_PTR time
);

int sys_cvt_time_to_string (
  SYS_TIME_PTR tim,
  int string_size,
  char *string
);

int sys_cvt_time_to_julian_date (
  SYS_TIME_PTR tim,
  int *julian_date
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

int sys_add_times (	/* time1 = time1 + time2 */
  SYS_TIME_PTR time1,
  SYS_TIME_PTR time2
);

int sys_subtract_times (	/* time1 = time1 - time2 */
  SYS_TIME_PTR time1,
  SYS_TIME_PTR time2
);

int sys_cvt_seconds_to_timeout (
  float seconds,
  SYS_TIME_PTR time
);

int sys_cvt_timeout_to_seconds (
  SYS_TIME_PTR tim,
  float *seconds
);

#ifdef __cplusplus
}
#endif

#endif
