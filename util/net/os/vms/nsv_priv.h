#ifndef __nsv_priv_h
#define __nsv_priv_h 1

/* nsv_priv.h - private include file for network server package */

#include nsv
#include port_priv

/***
/* defines
*/
#define TEMP_MBX	0
#define BUF_QUO		4096
#define MAX_MBX_MSG	128
#define MAX_DATA_MSG	512
#define MAX_NCB		110

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
typedef struct io_stat_blk_tag {
  short int status;
  short int msg_len;
  int unused;
} IO_STAT_BLK_T;

typedef struct thread_signal_tag {
  cma_t_cond cv;
  cma_t_attr cv_attr;
  cma_t_mutex mu;
  cma_t_attr mu_attr;
  unsigned int break_connection;
} THREAD_SIGNAL_TYPE;
typedef THREAD_SIGNAL_TYPE *THREAD_SIGNAL_PTR;

#endif
