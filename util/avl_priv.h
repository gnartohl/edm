#ifndef ___avl_priv_h
#define ___avl_priv_h 1

/***
* avl_priv.h - private include file for avl tree routines in avl.c
*/

#ifdef VMS
#include avl
#endif

#ifndef VMS
#include "avl.h"
#endif

#define MAX_DEPTH	    64

#define EH		    0x1001
#define RH		    0x1002
#define LH		    0x1003
#define FIND_CUR_NODE	    0x1004
#define FIND_LARGER_PARENT  0x1005
#define FIND_SMALLER_PARENT 0x1006
#define NODE_FOUND	    0x1007
#define NODE_NOT_FOUND	    0x1008
#define AVL_OK		    0x1009
#define STACK_OVFLO	    0x100A
#define STACK_UNFLO	    0x100B

typedef struct avl_node_tag {
  unsigned int depth;
  int bf;
  struct avl_node_tag *right;
  struct avl_node_tag *left;
} AVL_NODE_TYPE;
typedef AVL_NODE_TYPE *AVL_NODE_PTR;

typedef struct {
  int ptr;
  AVL_NODE_PTR contents[MAX_DEPTH];
  int result[MAX_DEPTH];
} AVL_STACK_T;

typedef struct avl_handle_tag {
  AVL_NODE_PTR root;
  AVL_NODE_PTR cur_node;
  AVL_INT_FUNC comp_node_func;
  AVL_INT_FUNC comp_item_func;
  AVL_INT_FUNC copy_node_func;
  AVL_STACK_T stack;
} AVL_HANDLE_TYPE;
typedef AVL_HANDLE_TYPE *PRIV_AVL_HANDLE;

#endif
