#include <stdio.h>
#include <stdlib.h>
#include "thread.h"

void *test (
  THREAD_HANDLE h )
{

int i, stat, id;

 id = (int) thread_get_app_data( h );

  printf( "in test %-d\n", id );

  for ( i=0; i<10; i++ ) {
    stat = system( "ls -l /home/sinclair/.bash*" );
    stat = thread_delay( h, 1.0 );
  }

  stat = 1;
  thread_exit( h, (void *) &stat );

  return NULL;

}

main() {

int i, stat, notAllDone, notDone[10];
THREAD_HANDLE th[10], delayH;

  stat = thread_init();

  stat = thread_create_handle( &delayH, NULL );

  for ( i=0; i<10; i++ ) {

    notDone[i] = 1;

    th[i] = (THREAD_HANDLE *) calloc( 1, sizeof(THREAD_HANDLE) );
    stat = thread_create_handle( &th[i], (void *) i );
    stat = thread_create_proc( th[i], test );

//      stat = thread_detach( th[i] );

  }

  do {

    notAllDone = 0;
    for ( i=0; i<10; i++ ) {
      if ( notDone[i] ) {
        stat = thread_wait_til_complete_no_block( th[i] );
        if ( stat & 1 ) {
          notDone[i] = 0;
        }
        else if ( stat == THR_NOJOIN ) {
          notAllDone++;
	}
        else {
          printf( "stat = %-X\n", stat );
	}
      }
    }

    if ( notAllDone ) {
      printf( "in main, waiting...\n" );
      stat = thread_delay( delayH, 1.0 );
    }

  } while ( notAllDone );

}
