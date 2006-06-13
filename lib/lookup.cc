#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <glob.h>
#include "lookup.h"

static int g_allocated = 0;
static unsigned int g_index = 0;
static glob_t g_glob;

typedef int (*glob_err_func_t)( const char *, int );

static int globErrFunc (
  char *epath,
  int globErr
) {

  //fprintf( stderr, "globErrFunc\n" );
  return 0;

}

static void extractComponents (
  char *path,
  int maxName,
  char *name,
  int maxExt,
  char *ext
) {

int i, l, outlen, first, last;

  l = strlen(path);

  // ---------------------------------------------------------------
  // get name

  first = 0;

  // find 1st char after last '/'
  for ( i=l; i>=0; i-- ) {
    if ( path[i] == '/' ) {
      first = i+1;
      break;
    }
  }

  last = l-1;

  // find last char before '.'
  for ( i=l; i>=0; i-- ) {
    if ( path[i] == '.' ) {
      last = i-1;
      break;
    }
  }
  if ( last < first ) last = first;

  outlen = last - first + 1;
  if ( outlen > maxName ) outlen = maxName;

  strncpy( name, &path[first], outlen );
  name[outlen] = 0;

  // ----------------------------------------------------------
  // get ext

  first = last + 2;
  if ( first > l-1 ) {

    strcpy( ext, "" );

  }
  else {

    outlen = l - first;
    if ( outlen > maxExt ) outlen = maxExt;

    strncpy( ext, &path[first], outlen );
    ext[outlen] = 0;

  }

}

void getFirstFileNameExt (
  char *spec,
  int maxName,
  char *name,
  int maxExt,
  char *ext,
  int *found
) {

int st;

  if ( g_allocated ) {
    g_allocated = 0;
    globfree( &g_glob );
  }

  st = glob( spec, 0, (glob_err_func_t) globErrFunc, &g_glob );
  if ( st ) {
    *found = 0;
    strcpy( name, "" );
    strcpy( ext, "" );
    return;
  }

  g_index = 0;
  g_allocated = 1;

  if ( g_glob.gl_pathc == 0 ) {

    *found = 0;
    strcpy( name, "" );
    strcpy( ext, "" );

  }
  else {

    *found = 1;

    extractComponents( g_glob.gl_pathv[g_index], maxName, name, maxExt, ext );
    g_index++;

  }

}

void getNextFileNameExt (
  char *spec,
  int maxName,
  char *name,
  int maxExt,
  char *ext,
  int *found
) {

  if ( !g_allocated ) {
    *found = 0;
    strcpy( name, "" );
    strcpy( ext, "" );
    return;
  }

  if ( g_index > g_glob.gl_pathc-1 ) {
    *found = 0;
    strcpy( name, "" );
    strcpy( ext, "" );
    return;
  }

  *found = 1;

  extractComponents( g_glob.gl_pathv[g_index], maxName, name, maxExt, ext );
  g_index++;

}

void getFirstFile (
  char *spec,
  int maxName,
  char *name,
  int *found
) {

int st;

  if ( g_allocated ) {
    g_allocated = 0;
    globfree( &g_glob );
  }

  st = glob( spec, 0, (glob_err_func_t) globErrFunc, &g_glob );
  if ( st ) {
    *found = 0;
    strcpy( name, "" );
    return;
  }

  g_index = 0;
  g_allocated = 1;

  if ( g_glob.gl_pathc == 0 ) {

    *found = 0;
    strcpy( name, "" );

  }
  else {

    *found = 1;

    strncpy( name, g_glob.gl_pathv[g_index], maxName );
    g_index++;

  }

}

void getNextFile (
  char *spec,
  int maxName,
  char *name,
  int *found
) {

  if ( !g_allocated ) {
    *found = 0;
    strcpy( name, "" );
    return;
  }

  if ( g_index > g_glob.gl_pathc-1 ) {
    *found = 0;
    strcpy( name, "" );
    return;
  }

  *found = 1;

  strncpy( name, g_glob.gl_pathv[g_index], maxName );
  g_index++;

}
