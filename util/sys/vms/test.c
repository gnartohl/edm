#include stdlib
#include stdio
#include descrip
#include ssdef
#include iodef

#include "sys_types.h"

main() {

int i, n, next_key, key;
float delay = 3.0;

  do {

    key = sys_get_char();

    if ( key < 32 )
      printf( "^%c\n", key+64 );
    else
      printf( "%c\n", key );

  } while ( key != '\\' );

  exit(1);

  printf( "start\n" );
  lib$wait( &delay );

  n = sys_num_chars();
  printf( "n = %d\n", n );
  for ( i=0; i<n; i++ ) {
    next_key = sys_next_char();
    key = sys_get_char();
    if ( key > 31 )
      printf( "next_key = %c,   key = %c\n", next_key, key );
    else
      printf( "next_key = %d,   key = %d\n", next_key, key );
  };
}
