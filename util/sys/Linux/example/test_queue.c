#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "sys_types.h"

/* fixed interlocked queue example */

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

int main ( void ) {

QUEUE_TYPE freeQueue, activeQueue;
NODE_TYPE nodes[QUEUE_SIZE];
NODE_PTR node;
int stat, q_stat_r, q_stat_i, i, n;
char msg[255+1];

  /* init */

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

  /* at this point, all nodes are on the free queue */

  // goto z;

  /* queue 2 messages */

  strcpy( msg, "this is a message 1" );

  q_stat_r = REMQHI( (void *) &freeQueue, (void **) &node, 0 );
  if ( q_stat_r & 1 ) {

    strncpy( node->msg, msg, 255 );

    q_stat_i = INSQTI( (void *) node, (void *) &activeQueue, 0 );
    if ( !( q_stat_i & 1 ) ) {
      printf( "Cannot insert node into active queue\n" );
    }
    else if ( q_stat_i == SYS_ONEENTQUE ) {
      /* usually we signal a worker thread here */
      printf( "first entry on empty queue: [%s]\n", node->msg );
    }

  }
  else {

    printf( "Cannot remove node from free queue\n" );

  }

  strcpy( msg, "this is a message 2" );

  q_stat_r = REMQHI( (void *) &freeQueue, (void **) &node, 0 );
  if ( q_stat_r & 1 ) {

    strncpy( node->msg, msg, 255 );

    q_stat_i = INSQTI( (void *) node, (void *) &activeQueue, 0 );
    if ( !( q_stat_i & 1 ) ) {
      printf( "Cannot insert node into active queue\n" );
    }
    else if ( q_stat_i == SYS_ONEENTQUE ) {
      /* usually we signal a worker thread here */
      printf( "first entry on empty queue: [%s]\n", node->msg );
    }

  }
  else {

    printf( "Cannot remove node from free queue\n" );

  }

 z:

  /* retrieve all messages */

  n = 0;
  do {

    q_stat_r = REMQHI( (void *) &activeQueue, (void **) &node, 0 );
    if ( q_stat_r & 1 ) {

      n++;

      printf( "message: %s\n", node->msg );

      q_stat_i = INSQTI( (void *) node, (void *) &freeQueue, 0 );
      if ( !( q_stat_i & 1 ) ) {
        printf( "Cannot reinsert node into free queue\n" );
      }

    }
    else if ( q_stat_r == SYS_QUEWASEMP ) {
      if ( !n ) printf( "Active queue is empty\n" );
    }

  } while ( q_stat_r & 1 );

}
