#ifndef __ncl_priv_h
#define __ncl_priv_h 1

/* ncl_priv.h - private include file for network server package */

#include $vaxelnc
#include $kernelmsg
#include ncl
#include port_priv

/***
/* defines
*/
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

#endif
