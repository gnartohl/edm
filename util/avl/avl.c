/***
* avl.c - avl tree routines
*/

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifdef VMS
#include stdlib
#include stdio
#include avl_priv
#endif

#ifndef VMS
#include <stdlib.h>
#include <stdio.h>
#include "avl_priv.h"
#endif


static void avl___flush_stack(
  AVL_STACK_T *stack
) {

  stack->ptr = MAX_DEPTH;

}

static int avl___push(
  AVL_STACK_T *stack,
  AVL_NODE_PTR p,
  int result
) {

  if ( stack->ptr == 0 ) return STACK_OVFLO;
  stack->ptr--;
  stack->contents[stack->ptr] = p;
  stack->result[stack->ptr] = result;

  return AVL_SUCCESS;

}

static int avl___pop(
  AVL_STACK_T *stack,
  AVL_NODE_PTR *p,
  int *result
) {

  if ( stack->ptr == MAX_DEPTH ) return STACK_UNFLO;
  *p = stack->contents[stack->ptr];
  *result = stack->result[stack->ptr];
  stack->ptr++;

  return AVL_SUCCESS;

}

static int avl___rotate_left(
  AVL_NODE_PTR *p
) {

AVL_NODE_PTR temp;

  if ( !(*p) ) {
    return AVL_NULROOT;
  }

  if ( !(*p)->right ) {
    return AVL_NULRIGHT;
  }

  temp = (*p)->right;
  (*p)->right = temp->left;
  temp->left = (*p);
  (*p) = temp;

  return AVL_SUCCESS;

}

static int avl___rotate_right(
  AVL_NODE_PTR *p
) {

AVL_NODE_PTR temp;

  if ( !(*p) ) {
    return AVL_NULROOT;
  }

  if ( !(*p)->left ) {
    return AVL_NULLEFT;
  }

  temp = (*p)->left;
  (*p)->left= temp->right;
  temp->right = (*p);
  (*p) = temp;

  return AVL_SUCCESS;

}

static int avl___right_balance_ins(
  AVL_NODE_PTR *root,
  int *taller
) {

AVL_NODE_PTR x, w;
int stat;

  x = (*root)->right;

  switch ( x->bf ) {

    case RH:
      (*root)->bf = EH;
      x->bf = EH;
      stat = avl___rotate_left( root );
      if ( stat != AVL_SUCCESS ) return stat;
      *taller = 0;
      break;

    case EH:
      return AVL_RBERR;

    case LH:
      w = x->left;
      if ( w->bf == EH ) {
        (*root)->bf = EH;
	x->bf = EH;
      }
      else if ( w->bf == LH ) {
        (*root)->bf = EH;
        x->bf = RH;
      }
      else if ( w->bf == RH ) {
        (*root)->bf = LH;
        x->bf = EH;
      }
      w->bf = EH;
      stat = avl___rotate_right( &x );
      if ( stat != AVL_SUCCESS ) return stat;
      (*root)->right = x;
      avl___rotate_left( root );
      *taller = FALSE;
      break;

    default:
      return AVL_UNKBF;

  }

  return AVL_SUCCESS;

}

static int avl___left_balance_ins(
  AVL_NODE_PTR *root,
  int *taller
) {

AVL_NODE_PTR x, w;
int stat;

  x = (*root)->left;

  switch ( x->bf ) {

    case LH:
      (*root)->bf = EH;
      x->bf = EH;
      stat = avl___rotate_right( root );
      if ( stat != AVL_SUCCESS ) return stat;
      *taller = 0;
      break;

    case EH:
      return AVL_LBERR;

    case RH:
      w = x->right;
      if ( w->bf == EH ) {
        (*root)->bf = EH;
	x->bf = EH;
      }
      else if ( w->bf == RH ) {
        (*root)->bf = EH;
        x->bf = LH;
      }
      else if ( w->bf == LH ) {
        (*root)->bf = RH;
        x->bf = EH;
      }
      w->bf = EH;
      stat = avl___rotate_left( &x );
      if ( stat != AVL_SUCCESS ) return stat;
      (*root)->left = x;
      avl___rotate_right( root );
      *taller = FALSE;
      break;

    default:
      return AVL_UNKBF;

  }

  return AVL_SUCCESS;

}

static int avl___insert(
  AVL_NODE_PTR *root,
  AVL_NODE_PTR new_item,
  AVL_INT_FUNC comp_node_func,
  int *taller,
  int *dup
) {

int tallersubtree, result, stat;

  *dup = FALSE;

  if ( !(*root) ) {

    *root = new_item;
    (*root)->left = NULL;
    (*root)->right = NULL;
    (*root)->depth = 0;
    (*root)->bf = EH;
    *taller = TRUE;

  }
  else {

    result = (*comp_node_func)( (void *) new_item, (void *) *root );

    if ( result == 0 ) {

      *dup = TRUE;
      return AVL_SUCCESS;

    }
    else if ( result < 0 ) {    /* new value < root value */

      stat = avl___insert( &((*root)->left), new_item, comp_node_func,
       &tallersubtree, dup );
      if ( stat != AVL_SUCCESS ) return stat;
      if ( *dup ) return AVL_SUCCESS;
      if ( tallersubtree ) {
        switch ( (*root)->bf ) {
	  case LH:
	    stat = avl___left_balance_ins( root, taller );
	    if ( stat != AVL_SUCCESS ) return stat;
	    break;
	  case EH:
	    (*root)->bf = LH;
	    *taller = TRUE;
	    break;
	  case RH:
	    (*root)->bf = EH;
	    *taller = FALSE;
	    break;
	  default:
	    return AVL_UNKBF;
	}
      }
      else {
        *taller = FALSE;
      }

    }
    else {    /* new value > root value */

      stat = avl___insert( &((*root)->right), new_item, comp_node_func,
       &tallersubtree, dup );
      if ( stat != AVL_SUCCESS ) return stat;
      if ( *dup ) return AVL_SUCCESS;
      if ( tallersubtree ) {
        switch ( (*root)->bf ) {
	  case LH:
	    (*root)->bf = EH;
	    *taller = FALSE;
	    break;
	  case EH:
	    (*root)->bf = RH;
	    *taller = TRUE;
	    break;
	  case RH:
	    stat = avl___right_balance_ins( root, taller );
	    if ( stat != AVL_SUCCESS ) return stat;
	    break;
	  default:
	    return AVL_UNKBF;
	}
      }
      else {
        *taller = FALSE;
      }

    }

  }

  return AVL_SUCCESS;

}

static int avl___right_balance_del(
  AVL_NODE_PTR *root,
  int *shorter
) {

AVL_NODE_PTR q, w;
int stat;

  q = (*root)->right;

  switch ( q->bf ) {

    case RH:
      (*root)->bf = EH;
      q->bf = EH;
      stat = avl___rotate_left( root );
      if ( stat != AVL_SUCCESS ) return stat;
      *shorter = 1;
      break;

    case EH:
      q->bf = LH;
      stat = avl___rotate_left( root );
      if ( stat != AVL_SUCCESS ) return stat;
      *shorter = 0;
      break;

    case LH:
      w = q->left;
      if ( w->bf == EH ) {
        (*root)->bf = EH;
	q->bf = EH;
      }
      else if ( w->bf == LH ) {
        (*root)->bf = EH;
        q->bf = RH;
      }
      else if ( w->bf == RH ) {
        (*root)->bf = LH;
        q->bf = EH;
      }
      w->bf = EH;
      stat = avl___rotate_right( &q );
      if ( stat != AVL_SUCCESS ) return stat;
      (*root)->right = q;
      stat = avl___rotate_left( root );
      if ( stat != AVL_SUCCESS ) return stat;
      *shorter = 1;
      break;

    default:
      return AVL_UNKBF;

  }

  return AVL_SUCCESS;

}

static int avl___left_balance_del(
  AVL_NODE_PTR *root,
  int *shorter
) {

AVL_NODE_PTR q, w;
int stat;

  q = (*root)->left;

  switch ( q->bf ) {

    case LH:
      (*root)->bf = EH;
      q->bf = EH;
      stat = avl___rotate_right( root );
      if ( stat != AVL_SUCCESS ) return stat;
      *shorter = 1;
      break;

    case EH:
      q->bf = RH;
      stat = avl___rotate_right( root );
      if ( stat != AVL_SUCCESS ) return stat;
      *shorter = 0;
      break;

    case RH:
      w = q->right;
      if ( w->bf == EH ) {
        (*root)->bf = EH;
	q->bf = EH;
      }
      else if ( w->bf == RH ) {
        (*root)->bf = EH;
        q->bf = LH;
      }
      else if ( w->bf == LH ) {
        (*root)->bf = RH;
        q->bf = EH;
      }
      w->bf = EH;
      stat = avl___rotate_left( &q );
      if ( stat != AVL_SUCCESS ) return stat;
      (*root)->left = q;
      stat = avl___rotate_right( root );
      if ( stat != AVL_SUCCESS ) return stat;
      *shorter = 1;
      break;

    default:
      return AVL_UNKBF;

  }

  return AVL_SUCCESS;

}

static int avl___delete(
  AVL_NODE_PTR *root,
  AVL_NODE_PTR node,
  AVL_INT_FUNC comp_node_func,
  int *shorter
) {

/***
* on entry to this function, node is guaranteed to have no right child; also
* node is not the main root of the tree
*/

int shortersubtree, result1, result2, stat;

  result1 = (*comp_node_func)( (void *) node, (void *) *root );

  if ( result1 < 0 ) {    /* cur value < root value */

    result2 = (*comp_node_func)( (void *) node, (void *) (*root)->left );

    if ( result2 == 0 ) {

      if ( node->left )
        (*root)->left = node->left;
      else
        (*root)->left = node->right;
      *shorter = TRUE;

      switch ( (*root)->bf ) {
        case LH:
	  (*root)->bf = EH;
	  *shorter = TRUE;
	  break;
        case EH:
	  (*root)->bf = RH;
	  *shorter = FALSE;
	  break;
        case RH:
	  stat = avl___right_balance_del( root, shorter );
	  if ( stat != AVL_SUCCESS ) return stat;
	  break;
        default:
	  return AVL_UNKBF;
      }

      return AVL_SUCCESS;

    }
    else {

      stat = avl___delete( &((*root)->left), node, comp_node_func,
       &shortersubtree );
      if ( stat != AVL_SUCCESS ) return stat;
      if ( shortersubtree ) {
        switch ( (*root)->bf ) {
          case LH:
	    (*root)->bf = EH;
	    *shorter = TRUE;
	    break;
          case EH:
	    (*root)->bf = RH;
	    *shorter = FALSE;
	    break;
          case RH:
	    stat = avl___right_balance_del( root, shorter );
	    if ( stat != AVL_SUCCESS ) return stat;
	    break;
          default:
	    return AVL_UNKBF;
        }
      }
      else {
        *shorter = FALSE;
      }

    }

  }
  else {    /* cur value > root value */

    result2 = (*comp_node_func)( (void *) node, (void *) (*root)->right );

    if ( result2 == 0 ) {

      if ( node->left )
        (*root)->right = node->left;
      else
        (*root)->right = node->right;
      *shorter = TRUE;

      switch ( (*root)->bf ) {
        case LH:
	  stat = avl___left_balance_del( root, shorter );
	  if ( stat != AVL_SUCCESS ) return stat;
	  break;
        case EH:
	  (*root)->bf = LH;
	  *shorter = FALSE;
	  break;
        case RH:
	  (*root)->bf = EH;
	  *shorter = TRUE;
	  break;
        default:
	  return AVL_UNKBF;
      }

      return AVL_SUCCESS;

    }
    else {

      stat = avl___delete( &((*root)->right), node, comp_node_func,
       &shortersubtree );
      if ( stat != AVL_SUCCESS ) return stat;
      if ( shortersubtree ) {
        switch ( (*root)->bf ) {
          case LH:
	    stat = avl___left_balance_del( root, shorter );
	    if ( stat != AVL_SUCCESS ) return stat;
	    break;
          case EH:
	    (*root)->bf = LH;
	    *shorter = FALSE;
	    break;
          case RH:
	    (*root)->bf = EH;
	    *shorter = TRUE;
	    break;
          default:
	    return AVL_UNKBF;
        }
      }
      else {
        *shorter = FALSE;
      }

    }

  }

  return AVL_SUCCESS;

}

static int avl___get_match(
  AVL_NODE_PTR node,
  void *item,
  AVL_INT_FUNC comp_item_func,
  AVL_NODE_PTR *found_node
) {

int result;

  while ( 1 ) {

    if ( node == NULL ) {
      *found_node = NULL;
      return AVL_SUCCESS;
    }

    result = (*comp_item_func)( item, (void *) node );

    if ( result == 0 ) {    /* found it */

      *found_node = node;
      return AVL_SUCCESS;

    }
    else if ( result < 0 ) {	/* item is smaller */

      node = node->left;

    }
    else if ( result > 0 ) {	/* item is larger */

      node = node->right;

    }

  }

}

static int avl___get_first(
  AVL_NODE_PTR node,	/* this is initially the tree root node */
  AVL_NODE_PTR *first_node
) {

  while ( 1 ) {

    if ( node->left == NULL ) {
      *first_node = node;
      return AVL_SUCCESS;
    }

    node = node->left;

  }

}

static int avl___get_last(
  AVL_NODE_PTR node,
  AVL_NODE_PTR *last_node
) {

  while ( 1 ) {

    if ( node->right == NULL ) {
      *last_node = node;
      return AVL_SUCCESS;
    }

    node = node->right;

  }

}

static int avl___get_next_from_branch(
  AVL_NODE_PTR node,
  AVL_NODE_PTR *next_node
) {

  while ( 1 ) {

    if ( node->left == NULL ) {
      *next_node = node;
      return AVL_SUCCESS;
    }

    node = node->left;

  }

}

static int avl___get_next_from_leaf(
  int *state,
  AVL_STACK_T *stack,
  AVL_NODE_PTR root,
  AVL_NODE_PTR cur_node,
  AVL_INT_FUNC comp_node_func,
  AVL_NODE_PTR *next_node
) {

int result, stat;

  do {

    if ( *state == FIND_CUR_NODE ) {

      result = (*comp_node_func)( (void *) cur_node, (void *) root );

      if ( result == 0 ) {

        *state = FIND_LARGER_PARENT;
        stat = avl___pop( stack, &root, &result );
	if ( stat == STACK_UNFLO ) {
	  *next_node = NULL;
	  *state = NODE_NOT_FOUND;
        }

      }
      else if ( result > 0 ) {

        stat = avl___push( stack, root, result );
        if ( stat != AVL_SUCCESS ) {
	  return AVL_STKOFL;
        }
        root = root->right;
        if ( !root ) {
          *next_node = NULL;
	  *state = NODE_NOT_FOUND;
        }

      }
      else {

        stat = avl___push( stack, root, result );
        if ( stat != AVL_SUCCESS ) {
	  return AVL_STKOFL;
        }
        root = root->left;
	if ( !root ) {
          *next_node = NULL;
	  *state = NODE_NOT_FOUND;
        }

      }

    }

    else if ( *state == FIND_LARGER_PARENT ) {

      if ( result < 0 ) {
        *state = NODE_FOUND;
        *next_node = root;
      }
      else {
        stat = avl___pop( stack, &root, &result );
	if ( stat == STACK_UNFLO ) {
	  *next_node = NULL;
	  *state = NODE_NOT_FOUND;
        }
      }

    }

  } while ( ( *state != NODE_FOUND ) && ( *state != NODE_NOT_FOUND ) );

  avl___flush_stack( stack );

  return AVL_SUCCESS;

}

static int avl___get_prev_from_branch(
  AVL_NODE_PTR node,
  AVL_NODE_PTR *prev_node
) {

  while ( 1 ) {

    if ( node->right == NULL ) {
      *prev_node = node;
      return AVL_SUCCESS;
    }

    node = node->right;

  }

}

static int avl___get_prev_from_leaf(
  int *state,
  AVL_STACK_T *stack,
  AVL_NODE_PTR root,
  AVL_NODE_PTR cur_node,
  AVL_INT_FUNC comp_node_func,
  AVL_NODE_PTR *prev_node
) {

int result, stat;

  do {

    if ( *state == FIND_CUR_NODE ) {

      result = (*comp_node_func)( (void *) cur_node, (void *) root );

      if ( result == 0 ) {

        *state = FIND_SMALLER_PARENT;
        stat = avl___pop( stack, &root, &result );
	if ( stat == STACK_UNFLO ) {
	  *prev_node = NULL;
	  *state = NODE_NOT_FOUND;
        }

      }
      else if ( result > 0 ) {

        stat = avl___push( stack, root, result );
        if ( stat != AVL_SUCCESS ) {
	  return AVL_STKOFL;
        }
        root = root->right;
        if ( !root ) {
          *prev_node = NULL;
	  *state = NODE_NOT_FOUND;
        }

      }
      else {

        stat = avl___push( stack, root, result );
        if ( stat != AVL_SUCCESS ) {
	  return AVL_STKOFL;
        }
        root = root->left;
	if ( !root ) {
          *prev_node = NULL;
	  *state = NODE_NOT_FOUND;
        }

      }

    }

    else if ( *state == FIND_SMALLER_PARENT ) {

      if ( result > 0 ) {
        *state = NODE_FOUND;
        *prev_node = root;
      }
      else {
        stat = avl___pop( stack, &root, &result );
	if ( stat == STACK_UNFLO ) {
	  *prev_node = NULL;
	  *state = NODE_NOT_FOUND;
        }
      }

    }

  } while ( ( *state != NODE_FOUND ) && ( *state != NODE_NOT_FOUND ) );

  avl___flush_stack( stack );

  return AVL_SUCCESS;

}

static void avl___find_depth(
  AVL_NODE_PTR node,
  unsigned short n,
  int *cur_depth,
  int *cur_shortest_branch
) {

  if ( node == NULL ) return;

  node->depth = n;
  if ( *cur_depth < n ) *cur_depth = n;

  if ( node->right == NULL && node->left == NULL ) {
    if ( *cur_shortest_branch > n ) *cur_shortest_branch = n;
    return;
  }

  n++;
  avl___find_depth( node->right, n, cur_depth, cur_shortest_branch );
  avl___find_depth( node->left, n, cur_depth, cur_shortest_branch );

}

extern int avl_destroy(
  AVL_HANDLE handle
) {

PRIV_AVL_HANDLE priv_handle;

  priv_handle = (PRIV_AVL_HANDLE) handle;
  if ( !priv_handle ) return AVL_BADHNDL;

  free( priv_handle );

  return AVL_SUCCESS;

}

extern int avl_init_tree(
  AVL_INT_FUNC comp_node_func,
  AVL_INT_FUNC comp_item_func,
  AVL_INT_FUNC copy_node_func,
  AVL_HANDLE *handle
) {

PRIV_AVL_HANDLE priv_handle;

  priv_handle = (PRIV_AVL_HANDLE) malloc( sizeof(AVL_HANDLE_TYPE) );
  if ( !priv_handle ) return AVL_NOMEM;

  priv_handle->root = NULL;
  priv_handle->cur_node = NULL;
  priv_handle->comp_node_func = comp_node_func;
  priv_handle->comp_item_func = comp_item_func;
  priv_handle->copy_node_func = copy_node_func;

  priv_handle->stack.ptr = MAX_DEPTH;

  *handle = (AVL_HANDLE) priv_handle;

  return AVL_SUCCESS;

}

extern int avl_dup_handle(
  AVL_HANDLE handle,
  AVL_HANDLE *dup_handle
) {

PRIV_AVL_HANDLE priv_handle, priv_dup_handle;

  priv_handle = (PRIV_AVL_HANDLE) handle;
  if ( !priv_handle ) return AVL_BADHNDL;

  priv_dup_handle = (PRIV_AVL_HANDLE) malloc( sizeof(AVL_HANDLE_TYPE) );
  if ( !priv_dup_handle ) return AVL_NOMEM;

  priv_dup_handle->root = priv_handle->root;
  priv_dup_handle->cur_node = priv_handle->cur_node;
  priv_dup_handle->comp_node_func = priv_handle->comp_node_func;
  priv_dup_handle->comp_item_func = priv_handle->comp_item_func;
  priv_dup_handle->copy_node_func = priv_handle->copy_node_func;

  *dup_handle = (AVL_HANDLE) priv_dup_handle;

  return AVL_SUCCESS;

}

extern int avl_insert_node(
  AVL_HANDLE handle,
  void *new_item,
  int *dup
) {

PRIV_AVL_HANDLE priv_handle;
int taller, stat;

  priv_handle = (PRIV_AVL_HANDLE) handle;

  stat = avl___insert( &priv_handle->root, (AVL_NODE_PTR) new_item,
   priv_handle->comp_node_func, &taller, dup );
  if ( stat != AVL_SUCCESS ) return stat;

  return AVL_SUCCESS;

}

extern int avl_delete_node(
  AVL_HANDLE handle,
  void **node
) {

PRIV_AVL_HANDLE priv_handle;
AVL_NODE_PTR cur, prev;
int shorter, stat, saveBf;
struct avl_node_tag *saveRight;
struct avl_node_tag *saveLeft;

  priv_handle = (PRIV_AVL_HANDLE) handle;
  cur = (AVL_NODE_PTR) *node;

  if ( ( cur->left != NULL ) && ( cur->right != NULL ) ) {

    /* find immediate predecessor of this node */
    prev = cur->left;
    while ( prev->right ) {
      prev = prev->right;
    }

    /* delete prev */
    stat = avl___delete( &priv_handle->root, prev, priv_handle->comp_node_func,
     &shorter );
    if ( stat != AVL_SUCCESS ) return stat;

    saveBf = cur->bf;
    saveRight = cur->right;
    saveLeft = cur-> left;

    /* copy contents of prev into cur but keep the old balance factor info */
    (*priv_handle->copy_node_func)( cur, prev );

    cur->bf = saveBf;
    cur-> right = saveRight;
    cur->left = saveLeft;

    /* copy address of prev into the original parameter node */
    *node = prev;

  }
  else {

    if ( cur == priv_handle->root ) {

      /* delete root */
      if ( cur->right != NULL )
        priv_handle->root = priv_handle->root->right;
      else
        priv_handle->root = priv_handle->root->left;

    }
    else {

      /* delete cur */
      stat = avl___delete( &priv_handle->root, cur,
       priv_handle->comp_node_func, &shorter );
      if ( stat != AVL_SUCCESS ) return stat;

    }

  }

  return AVL_SUCCESS;

}

extern int avl_get_match(
  AVL_HANDLE handle,
  void *item,
  void **node
) {

PRIV_AVL_HANDLE priv_handle;
AVL_NODE_PTR found_node;
int stat;

  priv_handle = (PRIV_AVL_HANDLE) handle;
  if ( priv_handle->root != NULL ) {
    stat = avl___get_match( priv_handle->root, item, priv_handle->comp_item_func,
     &found_node );
    if ( stat != AVL_SUCCESS ) return stat;
    if ( found_node ) priv_handle->cur_node = found_node;
  }
  else {
    found_node = NULL;
  }

  *node = (void *) found_node;

  return AVL_SUCCESS;

}

extern int avl_get_first(
  AVL_HANDLE handle,
  void **node
) {

PRIV_AVL_HANDLE priv_handle;
AVL_NODE_PTR first_node;
int stat;

  priv_handle = (PRIV_AVL_HANDLE) handle;
  if ( priv_handle->root != NULL ) {
    stat = avl___get_first( priv_handle->root, &first_node );
    if ( stat != AVL_SUCCESS ) return stat;
    priv_handle->cur_node = first_node;
  }
  else {
    first_node = NULL;
    priv_handle->cur_node = NULL;
  }

  *node = (void *) first_node;

  return AVL_SUCCESS;

}

extern int avl_get_last(
  AVL_HANDLE handle,
  void **node
) {

PRIV_AVL_HANDLE priv_handle;
AVL_NODE_PTR last_node;
int stat;

  priv_handle = (PRIV_AVL_HANDLE) handle;
  if ( priv_handle->root != NULL ) {
    stat = avl___get_last( priv_handle->root, &last_node );
    if ( stat != AVL_SUCCESS ) return stat;
    priv_handle->cur_node = last_node;
  }
  else {
    last_node = NULL;
    priv_handle->cur_node = NULL;
  }

  *node = (void *) last_node;

  return AVL_SUCCESS;

}

extern int avl_get_next(
  AVL_HANDLE handle,
  void **node
) {

PRIV_AVL_HANDLE priv_handle;
AVL_NODE_PTR next_node;
int state, stat;

  priv_handle = (PRIV_AVL_HANDLE) handle;
  if ( priv_handle->cur_node == NULL ) {

    next_node = NULL;
    *node = (void *) NULL;
    return AVL_SUCCESS;

  }
  else if ( priv_handle->cur_node->right != NULL ) {

    stat = avl___get_next_from_branch( priv_handle->cur_node->right,
     &next_node );
    if ( stat != AVL_SUCCESS ) return stat;
    priv_handle->cur_node = next_node;
    *node = (void *) next_node;
    return AVL_SUCCESS;

  }
  else { /* from leaf */

    state = FIND_CUR_NODE;
    stat = avl___get_next_from_leaf( &state, &priv_handle->stack,
     priv_handle->root, priv_handle->cur_node, priv_handle->comp_node_func,
     &next_node );
    if ( stat != AVL_SUCCESS ) return stat;

    if ( state == NODE_FOUND ) {
      priv_handle->cur_node = next_node;
      *node = (void *) next_node;
      return AVL_SUCCESS;
    }
    else {
      *node = (void *) NULL;
      return AVL_SUCCESS;
    }

  }

}

extern int avl_get_prev(
  AVL_HANDLE handle,
  void **node
) {

PRIV_AVL_HANDLE priv_handle;
AVL_NODE_PTR prev_node;
int state, stat;

  priv_handle = (PRIV_AVL_HANDLE) handle;
  if ( priv_handle->cur_node == NULL ) {

    prev_node = NULL;
    *node = (void *) NULL;
    return AVL_SUCCESS;

  }
  else if ( priv_handle->cur_node->left != NULL ) {

    stat = avl___get_prev_from_branch( priv_handle->cur_node->left,
     &prev_node );
    if ( stat != AVL_SUCCESS ) return stat;
    priv_handle->cur_node = prev_node;
    *node = (void *) prev_node;
    return AVL_SUCCESS;

  }
  else { /* from leaf */

    state = FIND_CUR_NODE;
    stat = avl___get_prev_from_leaf( &state, &priv_handle->stack,
     priv_handle->root, priv_handle->cur_node, priv_handle->comp_node_func,
     &prev_node );
    if ( stat != AVL_SUCCESS ) return stat;

    if ( state == NODE_FOUND ) {
      priv_handle->cur_node = prev_node;
      *node = (void *) prev_node;
      return AVL_SUCCESS;
    }
    else {
      *node = (void *) NULL;
      return AVL_SUCCESS;
    }

  }

}

extern void avl_find_depth(
  AVL_HANDLE handle,
  int *depth,
  int *shortest_branch
) {

PRIV_AVL_HANDLE priv_handle;
unsigned short n;
int cur_depth;
int cur_shortest_branch;

  priv_handle = (PRIV_AVL_HANDLE) handle;
  cur_depth = 0;
  n = 1;
  if ( priv_handle->root != NULL ) {
    cur_shortest_branch = 0x7fffffff;
    avl___find_depth( priv_handle->root, n, &cur_depth, &cur_shortest_branch );
  }
  else
    cur_shortest_branch = 0;

  *shortest_branch = cur_shortest_branch;
  *depth = cur_depth;

}

extern int avl_get_root(
  AVL_HANDLE handle,
  void **node
) {

PRIV_AVL_HANDLE priv_handle;

  priv_handle = (PRIV_AVL_HANDLE) handle;

  *node = (void *) priv_handle->root;

  return AVL_SUCCESS;

}

extern int avl_get_left(
  AVL_HANDLE handle,
  void **node
) {

PRIV_AVL_HANDLE priv_handle;

  priv_handle = (PRIV_AVL_HANDLE) handle;

  *node = (void *) priv_handle->cur_node->left;

  return AVL_SUCCESS;

}

extern int avl_get_right(
  AVL_HANDLE handle,
  void **node
) {

PRIV_AVL_HANDLE priv_handle;

  priv_handle = (PRIV_AVL_HANDLE) handle;

  *node = (void *) priv_handle->cur_node->right;

  return AVL_SUCCESS;

}
