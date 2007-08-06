#include "os_objs.h"
#include "os_priv.h"


#ifdef FUTURE_USE
static void os___file_show_error(
  void *private_data,
  int code
) {

char *name;

  name = (char *) private_data;
  if ( name ) {
    printf( "  [%s]\n", name );
  }

}
#endif

static int os___prefix(
  char *logical_file_name,
  char *prefix
) {

/***
* return prefix for file
*/

char temp[127+1];
int i, l, ret_stat;

  l = strlen(logical_file_name);
  if ( l > 127 ) return OS_BADLGNAM;

  for ( i=0; i<l; i++ )
    temp[i] = toupper( logical_file_name[i] );

  if ( strcmp( temp, "UTIL_ERROR_MSG_FILE" ) ) {

     strcpy( prefix, "/msg_files/" );
     ret_stat = OS_SUCCESS;

  }
  else {

    ret_stat = OS_BADLGNAM;

  }

  return ret_stat;

}

static int os___postfix(
  char *logical_file_name,
  char *postfix
) {

/***
* return postfix for file
*/

char temp[127+1];
int i, l, ret_stat;

  l = strlen(logical_file_name);
  if ( l > 127 ) return OS_BADLGNAM;

  for ( i=0; i<l; i++ )
    temp[i] = toupper( logical_file_name[i] );

  if ( strcmp( temp, "UTIL_ERROR_MSG_FILE" ) ) {

     strcpy( postfix, ".mtx" );
     ret_stat = OS_SUCCESS;

  }
  else {

    ret_stat = OS_BADLGNAM;

  }

  return ret_stat;

}

static int os___filename(
  char *logical_file_name,
  char *filename
) {

/***
* return filename for file
*/

char temp[127+1];
int i, l, ret_stat;

  l = strlen(logical_file_name);
  if ( l > 127 ) return OS_BADLGNAM;

  for ( i=0; i<l; i++ )
    temp[i] = toupper( logical_file_name[i] );

  if ( strcmp( temp, "UTIL_ERROR_MSG_FILE" ) ) {

     strcpy( filename, MSG_MAIN_FACILITY );
     ret_stat = OS_SUCCESS;

  }
  else {

    ret_stat = OS_BADLGNAM;

  }

  return ret_stat;

}

#ifdef FUTURE_USE
static void os___error_message(
  void *private_data,
  int error_code
) {

  printf( "Error %d in os package\n", error_code );
  if ( g_list_file )
   fprintf( g_list_file, "Error %d in os package\n", error_code );

}
#endif

int os_get_filespec(
  char *logical_file_name,
  char *filespec
) {

int stat;
char temp[127+1];

  stat = os___prefix( logical_file_name, filespec );
  if ( stat != OS_SUCCESS ) return stat;

  stat = os___filename( logical_file_name, temp );
  if ( stat != OS_SUCCESS ) return stat;

  strcat( filespec, temp );

  stat = os___postfix( logical_file_name, temp );
  if ( stat != OS_SUCCESS ) return stat;

  strcat( filespec, temp );

  return OS_SUCCESS;

}
