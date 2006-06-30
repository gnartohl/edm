#include "pv_action.h"

#ifdef __linux__
static void *executeThread (
  THREAD_HANDLE h )
{
#endif

#ifdef darwin
static void *executeThread (
  THREAD_HANDLE h )
{
#endif

#ifdef __solaris__
static void *executeThread (
  THREAD_HANDLE h )
{
#endif

#ifdef __osf__
static void executeThread (
  THREAD_HANDLE h )
{
#endif

#ifdef HP_UX
static void *executeThread (
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

pvActionClass::pvActionClass ( void ) {

char *ptr, *tk, *ctx, file[255+1], line[255+1];
int i, l, stat, numLines;
FILE *f;

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

  f = fopen( file, "r" );
  if ( !f ) return;

  numLines = 0;
  ptr = fgets( line, 255, f );
  while ( ptr ) {

    numLines++;

    ptr = fgets( line, 255, f );

  }

  fclose( f );

  name = new char *[numLines+1];
  action = new char *[numLines+1];
  expAction = new expStringClass[numLines+1];

  f = fopen( file, "r" );
  if ( !f ) return;

  i = 0;
  ptr = fgets( line, 255, f );
  while ( ptr ) {

    ctx = NULL;

    // ---------------------------------------

    tk = strtok_r( line, " \t\n", &ctx );
    if ( tk ) {

      l = strlen( tk ) + 1;
      name[i] = new char [l];
      strcpy( name[i], tk );

    }
    else {

      l = strlen( "Unknown" ) + 1;
      name[i] = new char [l];
      strcpy( name[i], "Unknown" );

    }

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

    ptr = fgets( line, 255, f );

  }

  fclose( f );

  //for ( i=0; i<numLines; i++ ) {
  //  printf( "name[%-d] = [%s], action[%-d] = [%s]\n", i, name[i],
  //   i, action[i] );
  //}

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

  delete[] name;
  delete[] action;
  delete[] expAction;

}

int pvActionClass::numActions ( void ) {

  return n;

}

void pvActionClass::setPv (
  char *pvName
) {

int i;
char mArray[255+1][2], vArray[255+1][2];
char *macros[2], *values[2];

  strcpy( mArray[0], "pv" );
  strncpy( vArray[0], pvName, 255 );
  vArray[0][255] = 0;

  macros[0] = mArray[0];
  values[0] = vArray[0];

  for ( i=0; i<n; i++ ) {
    expAction[i].expand1st( 1, macros, values );
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

  stat = thread_create_proc( thread, executeThread );

  stat = thread_detach( thread );

}
