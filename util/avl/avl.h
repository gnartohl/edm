#ifndef ___avl_h
#define ___avl_h 1

#ifdef __cplusplus
extern "C" {
#endif

/***
* avl.h - public include file for avl tree routines in avl.c
*/

#define AVL_FIELDS( super_struct_tag_name )\
  unsigned int __avl_depth;\
  int __avl_bf;\
  struct super_struct_tag_name *__avl_right;\
  struct super_struct_tag_name *__avl_left;\

typedef void *AVL_HANDLE;
typedef int (*AVL_INT_FUNC)( void *, void * );

#ifdef VMS
globalvalue AVL_BADHNDL;
globalvalue AVL_LBERR;
globalvalue AVL_NOMEM;
globalvalue AVL_NULLEFT;
globalvalue AVL_NULRIGHT;
globalvalue AVL_NULROOT;
globalvalue AVL_RBERR;
globalvalue AVL_STKOFL;
globalvalue AVL_SUCCESS;
globalvalue AVL_UNKBF;
#endif

#ifndef VMS
#include "avl.msh"
#endif

int avl_init_tree(
  AVL_INT_FUNC comp_node_func,
  AVL_INT_FUNC comp_item_func,
  AVL_INT_FUNC copy_node_func,
  AVL_HANDLE *handle
);

int avl_destroy(
  AVL_HANDLE handle
);

int avl_dup_handle(
  AVL_HANDLE handle,
  AVL_HANDLE *dup_handle
);

int avl_insert_node(
  AVL_HANDLE handle,
  void *new_node,
  int *duplicate
);

int avl_delete_node(
  AVL_HANDLE handle,
  void **node		/* this parameter is a pointer to your record passed */
);			/* by reference */

int avl_get_match(
  AVL_HANDLE handle,
  void *key_value,
  void **node
);

int avl_get_first(
  AVL_HANDLE handle,
  void **node
);

int avl_get_next(
  AVL_HANDLE handle,
  void **node
);

int avl_get_last(
  AVL_HANDLE handle,
  void **node
);

int avl_get_prev(
  AVL_HANDLE handle,
  void **node
);

void avl_find_depth(
  AVL_HANDLE handle,
  int *depth,
  int *shortest_branch
);

#ifdef __cplusplus
}
#endif

#endif
