#include "pv_action.h"

int debugMode ( void );

pvActionClass::pvActionClass ( void ) {

char *ptr, *tk, *ctx, file[255+1], line[255+1];
int i, l, numLines, comment;
FILE *f;

  name = NULL;
  action = NULL;
  expAction = NULL;

  n = 0;

  ptr = getenv( environment_str15 );
  if ( !ptr ) return;

  strncpy( file, ptr, 255 );
  file[255] = 0;

  l = strlen(ptr);
  if ( ptr[l-1] != '/' ) {
    Strncat( file, (char *) "/edmActions", 255 );
  }
  else {
    Strncat( file, (char *) "edmActions", 255 );
  }

  if ( debugMode() ) fprintf( stderr, "Action file is [%s]\n", file );

  f = fopen( file, "r" );
  if ( !f ) return;

  numLines = comment = 0;
  ptr = fgets( line, 255, f );
  while ( ptr ) {

    ctx = NULL;
    tk = strtok_r( line, " \t\n", &ctx );
    if ( !tk ) {
      comment = 1;
    }
    else if ( tk[0] == '#' ) {
      comment = 1;
    }

    if ( !comment ) { /* not an empty line or comment */
      numLines++;
    }

    comment = 0;
    ptr = fgets( line, 255, f );

  }

  fclose( f );

  name = new char *[numLines+1];
  action = new char *[numLines+1];
  expAction = new expStringClass[numLines+1];

  f = fopen( file, "r" );
  if ( !f ) return;

  i = comment = 0;
  ptr = fgets( line, 255, f );
  while ( ptr ) {

    ctx = NULL;

    // ---------------------------------------

    tk = strtok_r( line, " \t\n", &ctx );

    if ( !tk ) {
      comment = 1;
    }
    else if ( tk[0] == '#' ) {
      comment = 1;
    }

    if ( !comment ) { /* not an empty line or comment */

      l = strlen( tk ) + 1;
      name[i] = new char [l];
      strcpy( name[i], tk );

      // ---------------------------------------

      tk = strtok_r( NULL, "\n", &ctx );
      if ( tk ) {

        l = strlen( tk ) + 1;
        action[i] = new char [l];
        strcpy( action[i], tk );

      }
      else {

        l = strlen( "Unknown" ) + 1;
        action[i] = new char [l];
        strcpy( action[i], "Unknown" );

      }

      // ---------------------------------------

      i++;

    }

    comment = 0;
    ptr = fgets( line, 255, f );

  }

  fclose( f );

  if ( debugMode() ) {
    for ( i=0; i<numLines; i++ ) {
      fprintf( stderr, "name[%-d] = [%s], action[%-d] = [%s]\n", i, name[i],
       i, action[i] );
    }
  }

  n = numLines;

  for ( i=0; i<n; i++ ) {
    expAction[i].setRaw( action[i] );
  }

}

pvActionClass::~pvActionClass ( void ) {

int i;

  for ( i=0; i<n; i++ ) {

    if ( name[i] ) {
      delete[] name[i];
    }

    if ( action[i] ) {
      delete[] action[i];
    }

  }

  if ( name ) delete[] name;
  if ( action ) delete[] action;
  if ( expAction ) delete[] expAction;

}

#ifdef __linux__
void *pvActionClass::executeThread (
  THREAD_HANDLE h )
{
#endif

#ifdef darwin
void *pvActionClass::executeThread (
  THREAD_HANDLE h )
{
#endif

#ifdef __solaris__
void *pvActionClass::executeThread (
  THREAD_HANDLE h )
{
#endif

#ifdef __osf__
void pvActionClass::executeThread (
  THREAD_HANDLE h )
{
#endif

#ifdef HP_UX
void *pvActionClass::executeThread (
  THREAD_HANDLE h )
{
#endif

int stat;
threadParamBlockPtr threadParamBlock =
 (threadParamBlockPtr) thread_get_app_data( h );

  stat = executeCmd( threadParamBlock->cmd );

  stat = thread_request_free_ptr( (void *) threadParamBlock->cmd );
  stat = thread_request_free_ptr( (void *) threadParamBlock );
  stat = thread_detached_exit( h, NULL ); // this call deallocates h

#ifdef __linux__
  return (void *) NULL;
#endif

#ifdef darwin
  return (void *) NULL;
#endif
  
#ifdef __solaris__
  return (void *) NULL;
#endif

}

int pvActionClass::numActions ( void ) {

  return n;

}

void pvActionClass::setInfo (
  char *pvName,
  char *displayName
) {

int i;
char mArray[3][255+1], vArray[3][255+1];
char *macros[3], *values[3];

  strcpy( mArray[0], "pv" );
  strncpy( vArray[0], pvName, 255 );
  vArray[0][255] = 0;

  strcpy( mArray[1], "display" );
  strncpy( vArray[1], displayName, 255 );
  vArray[1][255] = 0;

  for ( i=0; i<2; i++ ) {
    macros[i] = mArray[i];
    values[i] = vArray[i];
  }

  for ( i=0; i<n; i++ ) {
    expAction[i].expand1st( 2, macros, values );
  }

  if ( debugMode() ) {
    for ( i=0; i<n; i++ ) {
      fprintf( stderr,
       "name[%-d] = [%s], expanded action[%-d] = [%s]\n", i, name[i],
       i, expAction[i].getExpanded() );
    }
  }

}

char *pvActionClass::getActionName (
  int index
) {

  return name[index];

}

char *pvActionClass::getAction (
  int index
) {

  return expAction[index].getExpanded();

}

void pvActionClass::executeAction (
  int index
) {

int stat;

  threadParamBlock =
   (threadParamBlockPtr) calloc( 1, sizeof(threadParamBlockType) );

  threadParamBlock->cmd =
   (char *) calloc( strlen(expAction[index].getExpanded())+1, 1 );

  strcpy( threadParamBlock->cmd, expAction[index].getExpanded() );

  stat = thread_create_handle( &thread, threadParamBlock );

  stat = thread_create_proc( thread, pvActionClass::executeThread );

  stat = thread_detach( thread );

}
