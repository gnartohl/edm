#include stdlib
#include stdio

#define MAX 255

/***
/* private globals
*/
static int g_num_args = 0;
static char g_arg[10+3][MAX+1];


void param_init(
  int argc,
  char **argv
) {

int i, l;

  if ( argc <= 10 )
    g_num_args = argc;
  else
    g_num_args = 10;

  for ( i=0; i<3; i++ ) {
    strcpy( g_arg[i], "" );
  }

  for ( i=1; i<g_num_args; i++ ) {	/* disgard surrounding quotes */
    l = strlen( argv[i] ) - 1;
    if ( argv[i][0] == '\"' && argv[i][l] == '\"' ) {
      strncpy( g_arg[i+2], &argv[i][1], MAX );
      l = strlen( g_arg[i+2] ) - 1;
      g_arg[i+2][l] = 0;
    }
    else {
      strncpy( g_arg[i+2], argv[i], MAX );
    }
  }

  g_num_args += 2;

}

int param_program_argument_count( void ) {

  return g_num_args;

}

void param_program_argument(
  char *arg,
  int index
) {

  index--;

  if ( index >= g_num_args )
    strcpy( arg, "" );
  else
    strcpy( arg, g_arg[index] );

}
