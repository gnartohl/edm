#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "sys_types.h"
#include "environment.str"

int debugMode ( void );

#ifdef USECURL

#include <curl/curl.h>

static int g_init = 1;
static CURL *curlH;
static char *tmpDir = NULL;

static int getFileNameAndExt (
  char *name,
  char *fullName,
  int maxSize )
{

int start, end, i, ii, l, ret_stat;

 if ( !fullName || !name ) {
   ret_stat = 0;
   goto err_return;
 }

  l = strlen(fullName);

  start = 0;

  for ( i=l-1; i>=0; i-- ) {

    if ( fullName[i] == '/' ) {
      start = i+1;
      break;

    }

  }

  end = l-1;

#if 0
  for ( i=l-1; i>=start; i-- ) {

    if ( fullName[i] == '.' ) {
      end = i-1;
      break;

    }

  }
#endif

  strcpy( name, "" );
  for ( i=start, ii=0; (i<=end) && (ii<maxSize); i++, ii++ ) {
    name[ii] = fullName[i];
  }

  if ( ii >= maxSize ) ii = maxSize-1;
  name[ii] = 0;

  return 1;

err_return:

  if ( name ) strcpy( name, "" );

  return ret_stat;

}

#endif

FILE *fileOpen (
  char *fullName,
  char *mode
) {

#ifdef USECURL
FILE *f;
char name[255+1], buf[255+1], allPaths[10239+1], *urlList, *tk, *context;
int stat, gotFile, l;
char errBuf[CURL_ERROR_SIZE+1];
CURLcode result;
#endif

#ifndef USECURL
  if ( debugMode() ) printf( "Using local access only\n" );
  return fopen( fullName, mode );
#endif

#ifdef USECURL

  if ( debugMode() ) printf( "Using curl for URL-based access\n" );

  urlList = getenv( environment_str9 );
  if ( !urlList ) return fopen( fullName, mode );

  if ( g_init ) {
    g_init = 0;
    curlH = curl_easy_init();
    tk = getenv( environment_str8 );
    if ( tk ) {
      l = strlen(tk);
      if ( tk[l-1] != '/' ) {
	tmpDir = new char[l+2];
        strcpy( tmpDir, tk );
        strcat( tmpDir, "/" );
      }
      else {
        tmpDir = strdup( tk );
      }
    }
    else {
      tmpDir = strdup( "/tmp/" );
    }
  }

  strncpy( name, fullName, 255 );
  name[255] = 0;

  if ( debugMode() ) printf( "Open file [%s] [%s]\n", name, mode );

  if ( !strcmp( mode, "r" ) || !strcmp( mode, "rb" ) ) {

    stat = getFileNameAndExt( name, fullName, 255 );
    if ( !(stat & 1 ) ) {
      strncpy( name, fullName, 255 );
      name[255] = 0;
    }

    strncpy( allPaths, urlList, 10239 );
    allPaths[10239] = 0;
    context = NULL;
    tk = strtok_r( allPaths, "|", &context );

    gotFile = 0;

    while ( tk && !gotFile ) {

      l = strlen(tk);
      if ( tk[l-1] == '/' ) {
        tk[l-1] = 0;
      }

      strncpy( buf, tmpDir, 255 );
      Strncat( buf, name, 255 );
      if ( debugMode() ) printf( "open [%s]\n", buf );
      f = fopen( buf, "w" );
      if ( !f ) return NULL;

      strcpy( buf, tk );
      Strncat( buf, fullName, 255 );

      if ( debugMode() ) printf( "get [%s]\n", buf );

      curl_easy_setopt( curlH, CURLOPT_URL, buf );
      curl_easy_setopt( curlH, CURLOPT_FILE, f );
      curl_easy_setopt( curlH, CURLOPT_ERRORBUFFER, errBuf );
      curl_easy_setopt( curlH, CURLOPT_FAILONERROR, 1 );
      curl_easy_setopt( curlH, CURLOPT_SSL_VERIFYPEER, 0 );
      curl_easy_setopt( curlH, CURLOPT_SSL_VERIFYHOST, 0 );
      strcpy( errBuf, "" );
      result = curl_easy_perform( curlH );
      if ( debugMode() ) printf( "result = %-d, errno = %-d\n",
       (int) result, errno );
      if ( debugMode() ) printf( "errBuf = [%s]\n", errBuf );

      fclose( f );

      gotFile = !result;

      tk = strtok_r( NULL, "|", &context );

    }

    f = NULL;

    if ( result ) return NULL;

    strncpy( buf, tmpDir, 255 );
    Strncat( buf, name, 255 );
    f = fopen( buf, "r" );

  }
  else {

    f = fopen( name, mode );

  }

  return f;

#endif

}
