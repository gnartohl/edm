#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "sys_types.h"
#include "thread.h"

typedef struct node_tag { /* locked queue node */
  void *flink;
  void *blink;
  char msg[255+1];
} NODE_TYPE, *NODE_PTR;

typedef struct queue_tag { /* locked queue header */
  void *flink;
  void *blink;
  void *lock;
  char msg[255+1];
} QUEUE_TYPE, *QUEUE_PTR;

#define REMQHI( queue, buf, flag )\
  sys_remqh( (void *) (queue), (void **) (buf), (int) (flag) )

#define INSQTI( buf, queue, flag )\
  sys_insqt( (void *) (buf), (void *) (queue), (int) (flag) )

#define QUEUE_SIZE 10

QUEUE_TYPE freeQueue, activeQueue;
NODE_TYPE nodes[QUEUE_SIZE];

void *test (
  THREAD_HANDLE h )
{

int stat, id, q_stat_r, q_stat_i;
NODE_PTR node;

 id = (int) thread_get_app_data( h );

  printf( "in test %-d\n", id );

  do {

    do {

      q_stat_r = REMQHI( (void *) &activeQueue, (void **) &node, 0 );
      if ( q_stat_r & 1 ) {

        if ( strcmp( node->msg, "exit" ) == 0 ) {
          printf( "exiting...\n" );
          stat = 1;
          thread_exit( h, (void *) &stat );
        }

        printf( "message: %s\n", node->msg );

        q_stat_i = INSQTI( (void *) node, (void *) &freeQueue, 0 );
        if ( !( q_stat_i & 1 ) ) {
          printf( "Cannot reinsert node into free queue\n" );
        }

      }
      else if ( q_stat_r != SYS_QUEWASEMP ) {
        printf( "Active queue error, stat = %-d\n", q_stat_r );
      }

    } while ( q_stat_r & 1 );

    stat = thread_timed_wait_for_signal( h, 2.0 );
    if ( stat == THR_TIMEOUT ) {
      printf( "timeout\n" );
    }
    else {
      printf( "got signal\n" );
    }

  } while ( 1 );

  return NULL;

}

main() {

THREAD_HANDLE th, delayH;
NODE_PTR node;
int stat, q_stat_r, q_stat_i, i;
char msg[255+1];

  /* init queues */

  stat = sys_iniq( &freeQueue );
  if ( !( stat & 1 ) ) {
    printf( "Cannot initialize free queue - abort\n" );
    exit(0);
  }
  stat = sys_iniq( &activeQueue );
  if ( !( stat & 1 ) ) {
    printf( "Cannot initialize active queue - abort\n" );
    exit(0);
  }

  freeQueue.flink = NULL;
  freeQueue.blink = NULL;
  activeQueue.flink = NULL;
  activeQueue.blink = NULL;

  for ( i=0; i<QUEUE_SIZE; i++ ) {

    q_stat_i = INSQTI( (void *) &nodes[i], (void *) &freeQueue, 0 );
    if ( !( q_stat_i & 1 ) ) {
      printf( "Cannot initialize queue - abort\n" );
      exit(0);
    }

  }

  /* init thread package */

  stat = thread_init();

  stat = thread_create_handle( &delayH, NULL );

  th = (THREAD_HANDLE *) calloc( 1, sizeof(THREAD_HANDLE) );
  stat = thread_create_handle( &th, (void *) 1 );
  stat = thread_create_proc( th, test );

  for ( i=0; i<10; i++ ) {

    sprintf( msg, "this is a message %-d-1", i );

    q_stat_r = REMQHI( (void *) &freeQueue, (void **) &node, 0 );
    if ( q_stat_r & 1 ) {

      strncpy( node->msg, msg, 255 );

      q_stat_i = INSQTI( (void *) node, (void *) &activeQueue, 0 );
      if ( !( q_stat_i & 1 ) ) {
        printf( "Cannot insert node into active queue\n" );
      }
      else if ( q_stat_i == SYS_ONEENTQUE ) {
        stat = thread_signal( th );
        // printf( "first entry on empty queue: [%s]\n", node->msg );
      }

    }
    else {

      printf( "Cannot remove node from free queue\n" );

    }

    sprintf( msg, "this is a message %-d-2", i );

    q_stat_r = REMQHI( (void *) &freeQueue, (void **) &node, 0 );
    if ( q_stat_r & 1 ) {

      strncpy( node->msg, msg, 255 );

      q_stat_i = INSQTI( (void *) node, (void *) &activeQueue, 0 );
      if ( !( q_stat_i & 1 ) ) {
        printf( "Cannot insert node into active queue\n" );
      }
      else if ( q_stat_i == SYS_ONEENTQUE ) {
        stat = thread_signal( th );
        // printf( "first entry on empty queue: [%s]\n", node->msg );
      }

    }
    else {

      printf( "Cannot remove node from free queue\n" );

    }

    stat = thread_delay( delayH, 1.5 );

  }


  strcpy( msg, "exit" );

  q_stat_r = REMQHI( (void *) &freeQueue, (void **) &node, 0 );
  if ( q_stat_r & 1 ) {

    strncpy( node->msg, msg, 255 );

    q_stat_i = INSQTI( (void *) node, (void *) &activeQueue, 0 );
    if ( !( q_stat_i & 1 ) ) {
      printf( "Cannot insert node into active queue\n" );
    }
    else if ( q_stat_i == SYS_ONEENTQUE ) {
      stat = thread_signal( th );
      // printf( "first entry on empty queue: [%s]\n", node->msg );
    }

  }
  else {

    printf( "Cannot remove node from free queue\n" );

  }


  stat = thread_wait_til_complete( th );
  if ( !(stat & 1) ) {
    printf( "stat = %-X\n", stat );
  }

}
