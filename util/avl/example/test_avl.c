#include <stdio.h>
#include <stdlib.h>

#include "avl.h"

typedef struct listTag {
  AVL_FIELDS(listTag)
  int key;
  int value;
} listType, *listPtr;

static int compare_nodes (
  void *node1,
  void *node2
) {

listPtr p1, p2;

  p1 = (listPtr) node1;
  p2 = (listPtr) node2;

  if ( p1->key > p2->key )
    return 1;
  else if ( p1->key < p2->key )
    return -1;

  return 0;

}

static int compare_key (
  void *key,
  void *node
) {

listPtr p;
int one;

  p = (listPtr) node;
  one = (int) key;

  if ( one > p->key )
    return 1;
  else if ( one < p->key )
    return -1;

  return 0;

}

static int copy_nodes (
  void *node1,
  void *node2
) {

listPtr p1, p2;

  p1 = (listPtr) node1;
  p2 = (listPtr) node2;

  *p1 = *p2;

  return 1;

}

main() {

AVL_HANDLE tree;
int stat, dup, key;
listPtr cur;

  /* init */

  stat = avl_init_tree( compare_nodes,
   compare_key, copy_nodes, &tree );


  /* build list */

  cur = (listPtr) calloc( 1, sizeof(listType) );
  cur->key = 1;
  cur->value = 100;

  stat = avl_insert_node( tree, (void *) cur, &dup );
  if ( !( stat & 1 ) ) {
    printf( "error [%-d] from avl_insert_node\n", stat );
    exit(0);
  }
  if ( dup ) {
    printf( "dup key: %-d\n", cur->key );
  }

  cur = (listPtr) calloc( 1, sizeof(listType) );
  cur->key = 2;
  cur->value = 20;

  stat = avl_insert_node( tree, (void *) cur, &dup );
  if ( !( stat & 1 ) ) {
    printf( "error [%-d] from avl_insert_node\n", stat );
    exit(0);
  }
  if ( dup ) {
    printf( "dup key: %-d\n", cur->key );
  }

  cur = (listPtr) calloc( 1, sizeof(listType) );
  cur->key = 3;
  cur->value = 3000;

  stat = avl_insert_node( tree, (void *) cur, &dup );
  if ( !( stat & 1 ) ) {
    printf( "error [%-d] from avl_insert_node\n", stat );
    exit(0);
  }
  if ( dup ) {
    printf( "dup key: %-d\n", cur->key );
  }


  /* get match */

{
int i, keys[3] = { 2, 1, 3 };

  printf( "\n\nmatch keys\n" );

  for ( i=0; i<3; i++ ) {

    key = keys[i];

    stat = avl_get_match( tree, (void *) key, (void **) &cur );
    if ( !( stat & 1 ) ) {
      printf( "error [%-d] from avl_get_match\n", stat );
      exit(0);
    }
    if ( cur ) {
      printf( "key: %-d, value found: %-d\n", key, cur->value );
    }
    else {
      printf( "key: %-d, value not found\n", key );
    }

  }

}

  /* traverse list forward */

  printf( "\n\nforward list traversal\n" );

  stat = avl_get_first( tree, (void **) &cur );
  if ( !( stat & 1 ) ) {
    printf( "error [%-d] from avl_get_first\n", stat );
    exit(0);
  }

  while ( cur ) {

    printf( "key: %-d, value: %-d\n", cur->key, cur->value );

    stat = avl_get_next( tree, (void **) &cur );
    if ( !( stat & 1 ) ) {
      printf( "error [%-d] from avl_get_first\n", stat );
      exit(0);
    }

  }


  /* traverse list backward */

  printf( "\n\nbackward list traversal\n" );

  stat = avl_get_last( tree, (void **) &cur );
  if ( !( stat & 1 ) ) {
    printf( "error [%-d] from avl_get_first\n", stat );
    exit(0);
  }

  while ( cur ) {

    printf( "key: %-d, value: %-d\n", cur->key, cur->value );

    stat = avl_get_prev( tree, (void **) &cur );
    if ( !( stat & 1 ) ) {
      printf( "error [%-d] from avl_get_first\n", stat );
      exit(0);
    }

  }

}
