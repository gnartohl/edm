#include <stdio.h>
#include <stdlib.h>

#include "thread.h"

THREAD_LOCK_HANDLE g_lock;
THREAD_LOCK_ARRAY_HANDLE g_lock_array;

void one (
  THREAD_HANDLE handle
) {

int stat, i, ii;

  for ( i=0; i<10; i++ ) {

    for ( ii=0; ii<3; ii++ ) {
      stat = thread_lock_array_element ( g_lock_array, ii );
      printf( "one - take lock %-d\n", ii );
      stat = thread_delay( handle, 1.0 );
      stat = thread_unlock_array_element ( g_lock_array, ii );
      printf( "one - unlock %-d\n", ii );
    }

    stat = thread_delay( handle, 1.0 );

  }

}

void two (
  THREAD_HANDLE handle
) {

int stat, i, ii;

  for ( i=0; i<10; i++ ) {

    for ( ii=0; ii<3; ii++ ) {
      stat = thread_lock_array_element ( g_lock_array, ii );
      printf( "two - take lock %-d\n", ii );
      stat = thread_delay( handle, 1.0 );
      stat = thread_unlock_array_element ( g_lock_array, ii );
      printf( "two - unlock %-d\n", ii );
    }

    stat = thread_delay( handle, 3.0 );

  }

}

main() {

THREAD_HANDLE one_h, two_h;
int stat, i;

  stat = thread_init();

/*
  stat = thread_create_lock_handle( &g_lock );
  stat = thread_create_lock_array_handle( &g_lock_array, 10 );
*/

  stat = thread_create_handle( &one_h, NULL );

/*
  stat = thread_create_handle( &two_h, NULL );
*/

  for ( i=0; i<10; i++ ) {
    stat = thread_delay( one_h, 1.0 );
    printf( "tick\n" );
  }

  stat = thread_destroy_handle( one_h );

  exit(0);



  stat = thread_create_proc( one_h, one );
  stat = thread_create_proc( two_h, two );

  stat = thread_wait_til_complete( one_h );
  printf( "main - one has completed\n" );

  stat = thread_wait_til_complete( two_h );
  printf( "main - two has completed\n" );

}
