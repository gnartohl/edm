#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>

extern int main (
  int argc,
  char **argv )
{

Display *testDisplay;
char displayName[63+1];

  if ( argc < 2 ) {
    return 1;
  }

  strncpy( displayName, argv[1], 63 );
  displayName[63] = 0;

  testDisplay = XOpenDisplay( displayName );
  if ( !testDisplay ) {
    //fprintf( stderr, "cannot open display\n" );
    return 2;
  }

  XCloseDisplay( testDisplay );

  return 0;

}
