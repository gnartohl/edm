#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "avl.h"

typedef struct allocListTag {
  AVL_FIELDS(allocListTag)
  unsigned int addr;
  unsigned int size;
  unsigned int seq;
  char fname[31+1];
  int line;
} allocListType, *allocListPtr;

static int g_seq = 0;
static int g_seqX = 0;
static int g_init = 1;
static AVL_HANDLE g_tree;
static AVL_HANDLE g_treeX;
static int g_memTrackOn = 0;

static int compare_nodes (
  void *node1,
  void *node2
) {

allocListPtr p1, p2;

  p1 = (allocListPtr) node1;
  p2 = (allocListPtr) node2;

  if ( p1->addr > p2->addr )
    return 1;
  else if ( p1->addr < p2->addr )
    return -1;

  return 0;

}

static int compare_key (
  void *key,
  void *node
) {

allocListPtr p;
unsigned int one;

  p = (allocListPtr) node;
  one = (unsigned int) key;

  if ( one > p->addr )
    return 1;
  else if ( one < p->addr )
    return -1;

  return 0;

}

static int copy_nodes (
  void *node1,
  void *node2
) {

allocListPtr p1, p2;

  p1 = (allocListPtr) node1;
  p2 = (allocListPtr) node2;

  *p1 = *p2;

  return 1;

}

#define MY_ALLOC 1
#if MY_ALLOC

void zFunc ( void ) {

int i;

  i = 0;


}

//=======================================================================

char* zXtMalloc ( size_t size )
{

  int dup, stat;
  allocListPtr cur;
  char msg[256];

  char *ptr;

  //printf( "my XtMalloc\n" );

  if ( g_init ) {
    g_init = 0;
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_tree );
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_treeX );
  }

  ptr = (char *) malloc( size );

  if ( g_memTrackOn ) {

    if ( (size == 12) && ( g_seqX > 0 ) ) { // for diagnostics

      zFunc();
      sprintf( msg, "zXtMalloc [%-x] [%-d]\n", (int) ptr, (int) size );
      write( 1, msg, strlen(msg) );

    }

    cur = (allocListPtr) calloc( 1, sizeof(allocListType) );
    cur->addr = (unsigned int) ptr;
    cur->size = (unsigned int) size;
    cur->seq = g_seqX++;
    strncpy( cur->fname, "XtMalloc", 31 );
    cur->line = 0;

    stat = avl_insert_node( g_treeX, (void *) cur, &dup );
    if ( !( stat & 1 ) ) {
      printf( "error [%-d] from avl_insert_node\n", stat );
      exit(0);
    }
    if ( dup ) {
      printf( "dup addr at [%-x]\n", cur->addr );
    }

  }

  return ptr;

}

char* zlocXtMalloc ( size_t size, char* _fname, int _line )
{

  int dup, stat;
  allocListPtr cur;
  char msg[256];

  char *ptr;

  if ( g_init ) {
    g_init = 0;
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_tree );
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_treeX );
  }

  ptr = (char *) malloc( size );

  if ( g_memTrackOn ) {

    if ( (size == 12) && ( g_seqX > 0 ) ) { // for diagnostics

      zFunc();
      sprintf( msg, "zXtMalloc [%-x] [%-d]\n", (int) ptr, (int) size );
      write( 1, msg, strlen(msg) );

    }

    cur = (allocListPtr) calloc( 1, sizeof(allocListType) );
    cur->addr = (unsigned int) ptr;
    cur->size = (unsigned int) size;
    cur->seq = g_seqX++;
    strncpy( cur->fname, _fname, 31 );
    cur->line = _line;

    stat = avl_insert_node( g_treeX, (void *) cur, &dup );
    if ( !( stat & 1 ) ) {
      printf( "error [%-d] from avl_insert_node\n", stat );
      exit(0);
    }
    if ( dup ) {
      printf( "dup addr at [%-x]\n", cur->addr );
    }

  }

  return ptr;

}

//=======================================================================

char* zXtCalloc ( size_t num, size_t size )
{

  int dup, stat;
  allocListPtr cur;
  char msg[256];

  char *ptr;

  //printf( "my XtCalloc\n" );

  if ( g_init ) {
    g_init = 0;
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_tree );
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_treeX );
  }

  ptr = (char *) calloc( num, size );

  if ( g_memTrackOn ) {

    if ( (size == 12) && ( g_seqX > 200 ) ) { // for diagnostics

      zFunc();
      sprintf( msg, "zXtCalloc[%-x] [%-d]\n", (int) ptr, (int) size );
      //write( 1, msg, strlen(msg) );

    }

    cur = (allocListPtr) calloc( 1, sizeof(allocListType) );
    cur->addr = (unsigned int) ptr;
    cur->size = (unsigned int) size;
    cur->seq = g_seqX++;
    strncpy( cur->fname, "XtCalloc", 31 );
    cur->line = 0;

    stat = avl_insert_node( g_treeX, (void *) cur, &dup );
    if ( !( stat & 1 ) ) {
      //printf( "error [%-d] from avl_insert_node\n", stat );
      exit(0);
    }
    if ( dup ) {
      //printf( "dup addr at [%-x]\n", cur->addr );
    }

  }

  return ptr;

}

char* zlocXtCalloc ( size_t num, size_t size, char* _fname, int _line )
{

  int dup, stat;
  allocListPtr cur;
  char msg[256];

  char *ptr;

  if ( g_init ) {
    g_init = 0;
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_tree );
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_treeX );
  }

  ptr = (char *) calloc( num, size );

  if ( g_memTrackOn ) {

    if ( (size == 12) && ( g_seqX > 200 ) ) { // for diagnostics

      zFunc();
      sprintf( msg, "zXtCalloc[%-x] [%-d]\n", (int) ptr, (int) size );
      //write( 1, msg, strlen(msg) );

    }

    cur = (allocListPtr) calloc( 1, sizeof(allocListType) );
    cur->addr = (unsigned int) ptr;
    cur->size = (unsigned int) size;
    cur->seq = g_seqX++;
    strncpy( cur->fname, _fname, 31 );
    cur->line = _line;

    stat = avl_insert_node( g_treeX, (void *) cur, &dup );
    if ( !( stat & 1 ) ) {
      //printf( "error [%-d] from avl_insert_node\n", stat );
      exit(0);
    }
    if ( dup ) {
      //printf( "dup addr at [%-x]\n", cur->addr );
    }

  }

  return ptr;

}

//=======================================================================

void zXtFree ( char *obj )
{

  int stat;
  allocListPtr cur;
  char msg[256];

  //printf( "my XtFree\n" );

  if ( g_init ) {
    g_init = 0;
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_tree );
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_treeX );
  }

  if ( g_memTrackOn ) {

    stat = avl_get_match( g_treeX, obj, (void **) &cur );
    if ( !( stat & 1 ) ) {
      //printf( "error [%-d] from avl_get_match\n", stat );
      exit(0);
    }

    if ( cur ) {

      if ( (cur->size > 170) && (cur->size < 200) ) {

        sprintf( msg, "zXtFree [%-x] [%-d] [%s line %-d]\n", (int) cur->addr,
         (int) cur->size, cur->fname, cur->line );
        //write( 1, msg, strlen(msg) );

      }

      stat = avl_delete_node( g_treeX, (void **) &cur );
      if ( !( stat & 1 ) ) {
        //printf( "error [%-d] from avl_delete_node\n", stat );
        exit(0);
      }

      free( cur );

    }
    else {

      //printf( " (not in list)" );

    }

    //printf( "\n" );

  }

  free( obj );

}

void zlocXtFree ( char *obj, char* _fname, int _line )
{

  int stat;
  allocListPtr cur;
  char msg[256];

   sprintf( msg, "zlocXtFree called in %s line %-d\n", _fname, _line );
   //write( 1, msg, strlen(msg) );

  if ( g_init ) {
    g_init = 0;
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_tree );
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_treeX );
  }

  if ( g_memTrackOn ) {

    stat = avl_get_match( g_treeX, obj, (void **) &cur );
    if ( !( stat & 1 ) ) {
      //printf( "error [%-d] from avl_get_match\n", stat );
      exit(0);
    }

    if ( cur ) {

      if ( (cur->size > 170) && (cur->size < 200) ) {

        sprintf( msg, "zXtFree [%-x] [%-d] [%s line %-d]\n", (int) cur->addr,
         (int) cur->size, cur->fname, cur->line );
        //write( 1, msg, strlen(msg) );

      }

      stat = avl_delete_node( g_treeX, (void **) &cur );
      if ( !( stat & 1 ) ) {
        //printf( "error [%-d] from avl_delete_node\n", stat );
        exit(0);
      }

      free( cur );

    }
    else {

      //printf( " (not in list)" );

    }

    //printf( "\n" );

  }

  free( obj );

}

//=======================================================================

char* zXtRealloc ( char *oldPtr, size_t size )
{

  int dup, stat;
  allocListPtr cur;
  char msg[256];

  char *ptr;

  //printf( "my XtRealloc\n" );

  if ( g_init ) {
    g_init = 0;
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_tree );
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_treeX );
  }

  if ( g_memTrackOn ) {

    if ( oldPtr ) {

      stat = avl_get_match( g_treeX, oldPtr, (void **) &cur );
      if ( !( stat & 1 ) ) {
        //printf( "error [%-d] from avl_get_match\n", stat );
        exit(0);
      }

      if ( cur ) {

        sprintf( msg, "zXtRealloc free [%-x] [%-d] [%s line %-d]\n",
         (int) cur->addr, (int) cur->size, cur->fname, cur->line );
        //write( 1, msg, strlen(msg) );

        stat = avl_delete_node( g_treeX, (void **) &cur );
        if ( !( stat & 1 ) ) {
          //printf( "error [%-d] from avl_delete_node\n", stat );
          exit(0);
        }

        free( cur );

      }

    }

    ptr = (char *) realloc( oldPtr, size );

    if ( (size == 12) && ( g_seqX > 200 ) ) { // for diagnostics

      sprintf( msg, "zXtRealloc[%-x] [%-d]\n", (int) ptr, (int) size );
      //write( 1, msg, strlen(msg) );

    }

    cur = (allocListPtr) calloc( 1, sizeof(allocListType) );
    cur->addr = (unsigned int) ptr;
    cur->size = (unsigned int) size;
    cur->seq = g_seqX++;
    strncpy( cur->fname, "XtRealloc", 31 );
    cur->line = 0;

    stat = avl_insert_node( g_treeX, (void *) cur, &dup );
    if ( !( stat & 1 ) ) {
      //printf( "error [%-d] from avl_insert_node\n", stat );
      exit(0);
    }
    if ( dup ) {
      //printf( "dup addr at [%-x]\n", cur->addr );
    }

  }
  else {

    ptr = (char *) realloc( oldPtr, size );

  }

  return ptr;

}

char* zlocXtRealloc ( char *oldPtr, size_t size, char* _fname, int _line )
{

  int dup, stat;
  allocListPtr cur;
  char msg[256];

  char *ptr;

  if ( g_init ) {
    g_init = 0;
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_tree );
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_treeX );
  }

  if ( g_memTrackOn ) {

    if ( oldPtr ) {

      stat = avl_get_match( g_treeX, oldPtr, (void **) &cur );
      if ( !( stat & 1 ) ) {
        //printf( "error [%-d] from avl_get_match\n", stat );
        exit(0);
      }

      if ( cur ) {

        sprintf( msg, "zXtRealloc free [%-x] [%-d] [%s line %-d]\n",
         (int) cur->addr, (int) cur->size, cur->fname, cur->line );
        //write( 1, msg, strlen(msg) );

        stat = avl_delete_node( g_treeX, (void **) &cur );
        if ( !( stat & 1 ) ) {
          //printf( "error [%-d] from avl_delete_node\n", stat );
          exit(0);
        }

        free( cur );

      }

    }

    ptr = (char *) realloc( oldPtr, size );

    if ( (size == 12) && ( g_seqX > 200 ) ) { // for diagnostics

      sprintf( msg, "zXtRealloc[%-x] [%-d]\n", (int) ptr, (int) size );
      //write( 1, msg, strlen(msg) );

    }

    cur = (allocListPtr) calloc( 1, sizeof(allocListType) );
    cur->addr = (unsigned int) ptr;
    cur->size = (unsigned int) size;
    cur->seq = g_seqX++;
    strncpy( cur->fname, _fname, 31 );
    cur->line = _line;

    stat = avl_insert_node( g_treeX, (void *) cur, &dup );
    if ( !( stat & 1 ) ) {
      //printf( "error [%-d] from avl_insert_node\n", stat );
      exit(0);
    }
    if ( dup ) {
      //printf( "dup addr at [%-x]\n", cur->addr );
    }

  }
  else {

    ptr = (char *) realloc( oldPtr, size );

  }

  return ptr;

}

//=======================================================================

void* znew ( size_t size )
{

  int dup, stat;
  allocListPtr cur;
  char msg[256];

  void *ptr;

  //printf( "my new\n" );

  if ( g_init ) {
    g_init = 0;
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_tree );
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_treeX );
  }

  ptr = malloc( size );

  if ( g_memTrackOn ) {

#if 1
    if ( (size == 1) && ( g_seq > 200 ) ) { // for diagnostics

      zFunc();
      sprintf( msg, "[%-x] [%-d]\n", (int) ptr, (int) size );
      write( 1, msg, strlen(msg) );

    }
#endif

    cur = (allocListPtr) calloc( 1, sizeof(allocListType) );
    cur->addr = (unsigned int) ptr;
    cur->size = (unsigned int) size;
    cur->seq = g_seq++;
    strncpy( cur->fname, "new", 31 );
    cur->line = 0;

    stat = avl_insert_node( g_tree, (void *) cur, &dup );
    if ( !( stat & 1 ) ) {
      //printf( "error [%-d] from avl_insert_node\n", stat );
      exit(0);
    }
    if ( dup ) {
      //printf( "dup addr at [%-x]\n", cur->addr );
    }

  }

  return ptr;

}

void* zlocnew ( size_t size, char* _fname, int _line )
{

  int dup, stat;
  allocListPtr cur;
  char msg[256];

  void *ptr;

  if ( g_init ) {
    g_init = 0;
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_tree );
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_treeX );
  }

  ptr = malloc( size );

  if ( g_memTrackOn ) {

#if 1
    if ( (size == 1) && ( g_seq > 200 ) ) { // for diagnostics

      zFunc();
      sprintf( msg, "[%-x] [%-d]\n", (int) ptr, (int) size );
      write( 1, msg, strlen(msg) );

    }
#endif

    cur = (allocListPtr) calloc( 1, sizeof(allocListType) );
    cur->addr = (unsigned int) ptr;
    cur->size = (unsigned int) size;
    cur->seq = g_seq++;
    strncpy( cur->fname, _fname, 31 );
    cur->line = _line;

    stat = avl_insert_node( g_tree, (void *) cur, &dup );
    if ( !( stat & 1 ) ) {
      //printf( "error [%-d] from avl_insert_node\n", stat );
      exit(0);
    }
    if ( dup ) {
      //printf( "dup addr at [%-x]\n", cur->addr );
    }

  }

  return ptr;

}

//=======================================================================

void showMem ( void ) {

  int stat, n, total;
  unsigned int i;
  allocListPtr cur;
  unsigned char *cptr, text[100];

  if ( g_init ) {
    g_init = 0;
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_tree );
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_treeX );
  }

  n = 0;
  total = 0;

  stat = avl_get_first( g_tree, (void **) &cur );
  if ( !( stat & 1 ) ) {
    printf( "error [%-d] from avl_get_first\n", stat );
    exit(0);
  }

  while ( cur ) {

    n++;
    total += cur->size;

    printf( "%-d: addr = %-x\t\tsize = %-d [%s line %-d]\n",
     cur->seq, cur->addr, cur->size, cur->fname, cur->line );

    stat = avl_get_next( g_tree, (void **) &cur );
    if ( !( stat & 1 ) ) {
      printf( "error [%-d] from avl_get_first\n", stat );
      exit(0);
    }

  }

  if ( n ) {
    printf( "total = %-d\n", total );
  }
  else {
    //printf( "no problems\n" );
  }

  // now, for X

  n = 0;
  total = 0;

  stat = avl_get_first( g_treeX, (void **) &cur );
  if ( !( stat & 1 ) ) {
    printf( "error [%-d] from avl_get_first\n", stat );
    exit(0);
  }

  while ( cur ) {

    n++;
    total += cur->size;

    printf( "%-d: addr = %-x\t\tsize = %-d [%s line %-d]\n",
     cur->seq, cur->addr, cur->size, cur->fname, cur->line );

#if 0
    if ( cur->size < 80 ) {
      cptr = (unsigned char *) cur->addr;
      for ( i=0; i<cur->size; i++ ) {
      if ( ( cptr[i] < 32 ) || ( cptr[i] > '~' ) )
        text[i] = ' ';
      else
        text[i] = cptr[i];
      }
      text[cur->size] = 0;
      printf( "[%s]\n", text );
    }
#endif

    stat = avl_get_next( g_treeX, (void **) &cur );
    if ( !( stat & 1 ) ) {
      printf( "error [%-d] from avl_get_first\n", stat );
      exit(0);
    }

  }

  if ( n ) {
    printf( "for X, total = %-d\n", total );
  }
  else {
    //printf( "for X, no problems\n" );
  }

}

//=======================================================================

void zdelete ( void *obj )
{

  int stat;
  allocListPtr cur;

  //printf( "my delete\n" );

  if ( g_init ) {
    g_init = 0;
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_tree );
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_treeX );
  }

  if ( g_memTrackOn ) {

    stat = avl_get_match( g_tree, obj, (void **) &cur );
    if ( !( stat & 1 ) ) {
      //printf( "error [%-d] from avl_get_match\n", stat );
      exit(0);
    }

    if ( cur ) {

      stat = avl_delete_node( g_tree, (void **) &cur );
      if ( !( stat & 1 ) ) {
        //printf( "error [%-d] from avl_delete_node\n", stat );
        exit(0);
      }

      free( cur );

    }

  }

  free( obj );

}

void zlocdelete ( void *obj, char* _fname, int _line )
{

  int stat;
  allocListPtr cur;
  char msg[256];

   sprintf( msg, "zlocdelete called in %s line %-d\n", _fname, _line );
   //write( 1, msg, strlen(msg) );

  if ( g_init ) {
    g_init = 0;
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_tree );
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_treeX );
  }

  if ( g_memTrackOn ) {

    stat = avl_get_match( g_tree, obj, (void **) &cur );
    if ( !( stat & 1 ) ) {
      //printf( "error [%-d] from avl_get_match\n", stat );
      exit(0);
    }

    if ( cur ) {

      stat = avl_delete_node( g_tree, (void **) &cur );
      if ( !( stat & 1 ) ) {
        //printf( "error [%-d] from avl_delete_node\n", stat );
        exit(0);
      }

      free( cur );

    }

  }

  free( obj );

}

//=======================================================================

void memTrackOn ( void ) {

  g_memTrackOn = 1;

}

void memTrackOff ( void ) {

  g_memTrackOn = 0;

}

void memTrackReset ( void ) {

  int stat;
  allocListPtr cur, more;

  if ( g_init ) {
    g_init = 0;
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_tree );
    stat = avl_init_tree( compare_nodes,
     compare_key, copy_nodes, &g_treeX );
  }

  g_seqX = 0;
  g_seq = 0;

  do {

    stat = avl_get_first( g_tree, (void **) &cur );
    if ( !( stat & 1 ) ) {
      printf( "error [%-d] from avl_get_first\n", stat );
      exit(0);
    }

    more = cur;

    if ( cur ) {

      stat = avl_delete_node( g_tree, (void **) &cur );
      if ( !( stat & 1 ) ) {
        printf( "error [%-d] from avl_delete_node\n", stat );
        exit(0);
      }

      free( cur );

    }

  } while ( more );

  // for X

  do {

    stat = avl_get_first( g_treeX, (void **) &cur );
    if ( !( stat & 1 ) ) {
      printf( "error [%-d] from avl_get_first\n", stat );
      exit(0);
    }

    more = cur;

    if ( cur ) {

      stat = avl_delete_node( g_treeX, (void **) &cur );
      if ( !( stat & 1 ) ) {
        printf( "error [%-d] from avl_delete_node\n", stat );
        exit(0);
      }

      free( cur );

    }

  } while ( more );

}

#endif
