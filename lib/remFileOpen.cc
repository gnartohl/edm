#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include "sys_types.h"
#include <sys/stat.h>
#include <unistd.h>
#include "avl.h"
#include "utility.h"
#include "environment.str"

#ifdef USECURL
#include <sys/types.h>
#include <sys/stat.h>
static int gGetUserUmask = 1;
static int gUseUmask = 0;
static mode_t gUmask = 0;
#endif

int debugMode ( void );

typedef struct nameListTag {
  AVL_FIELDS(nameListTag)
  char *ext;
  char *cmd;
} nameListType, *nameListPtr;

static int gInitList = 1;
static AVL_HANDLE gFilterH = NULL;
static int gNumFilters = 0;

static int compare_nodes (
  void *node1,
  void *node2
) {

nameListPtr p1, p2;

  p1 = (nameListPtr) node1;
  p2 = (nameListPtr) node2;

  return ( strcmp( p1->ext, p2->ext ) );

}

static int compare_key (
  void *key,
  void *node
) {

nameListPtr p;
char *one;

  p = (nameListPtr) node;
  one = (char *) key;

  return ( strcmp( one, p->ext ) );

}

static int copy_node (
  void *node1,
  void *node2
) {

nameListPtr p1, p2;

  p1 = (nameListPtr) node1;
  p2 = (nameListPtr) node2;

  *p1 = *p2;

  return 1;

}

static void initFilters ( void ) {

char *ptr, *tk, *ctx, file[255+1], line[255+1];
int l, status, dup;
FILE *f;
nameListPtr cur;

  status = avl_init_tree( compare_nodes,
   compare_key, copy_node, &gFilterH );

  ptr = getenv( environment_str12 );
  if ( !ptr ) return;

  strncpy( file, ptr, 255 );
  file[255] = 0;

  l = strlen(ptr);
  if ( ptr[l-1] != '/' ) {
    Strncat( file, (char *) "/edmFilters", 255 );
  }
  else {
    Strncat( file, (char *) "edmFilters", 255 );
  }

  f = fopen( file, "r" );
  if ( !f ) return;

  ptr = fgets( line, 255, f );
  while ( ptr ) {

    cur = new nameListType;

    ctx = NULL;
    tk = strtok_r( line, " \t\n", &ctx );
    if ( tk ) {

      l = strlen(tk);
      cur->ext = new char[l+1];
      strcpy( cur->ext, tk );

      tk = strtok_r( NULL, "\n", &ctx );
      if ( tk ) {

        l = strlen(tk);
        cur->cmd = new char[l+1];
        strcpy( cur->cmd, tk );

        status = avl_insert_node( gFilterH, (void *) cur, &dup );
	if ( !( status & 1 ) ) goto errRet;

        if ( dup ) {
          delete[] cur->ext;
	  delete[] cur->cmd;
	  delete cur;
	}
	else {
	  gNumFilters++;
	}

      }

    }

    ptr = fgets( line, 255, f );

  }

  status = avl_get_first( gFilterH, (void **) &cur );
  if ( !( status & 1 ) ) goto errRet;

  while ( cur ) {

    status = avl_get_next( gFilterH, (void **) &cur );
    if ( !( status & 1 ) ) goto errRet;

  }

errRet:

  fclose( f );

}

static char *findFilter (
  char *oneExt
) {

int status;
nameListPtr cur;

  if ( !gFilterH ) return NULL;

  status = avl_get_match( gFilterH, (void *) oneExt, (void **) &cur );
  if ( !( status & 1 ) ) return NULL;
  if ( !cur ) return NULL;
  if ( !cur->cmd ) return NULL;

  return cur->cmd;

}

static int buildCommand (
  char *cmd,
  int max,
  char *prog,
  int progMax,
  char *filterCmd,
  char *filename
) {

char *ptr;
int i, l, n, nProg, gettingProg = 1;

  l = strlen( filterCmd );
  ptr = filterCmd;
  n = nProg = 0;

  for ( i=0; i<l; i++ ) {

    if ( ( ptr[i] == ' ' ) || ( ptr[i] == '\t' ) || ( ptr[i] == '\n' ) ) {
      gettingProg = 0;
      prog[nProg] = 0;
    }

    if ( gettingProg ) {

      if ( nProg >= progMax ) {
        strcpy( cmd, "" );
        return 2;
      }
      prog[nProg++] = ptr[i];

    }

    if ( ( ptr[i] == '%' ) && ( ptr[i+1] == 'f' ) ) {

      if ( n >= max ) {
        strcpy( cmd, "" );
        return 2;
      }
      cmd[n] = 0;
      Strncat( cmd, filename, 255 );
      n = strlen( cmd );
      i += 2;

    }
    else {

      if ( n >= max ) {
        strcpy( cmd, "" );
        return 2;
      }
      cmd[n++] = ptr[i];

    }

  }

  cmd[n] = 0;
  return 1;

}

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

static int filterInstalled (
  char *name
) {

FILE *f;
char *tk, *ctx, line[255+1];
int found = 1;

  strcpy( line, "which 2>&1 " );
  Strncat( line, name, 255 );

  // which will return 1 token, the full path to the executable or
  // more than 1 token if the executable was not found

  f = popen( line, "r" );
  if ( f ) {

    while ( fgets( line, 255, f ) ) {

      line[255] = 0;

      ctx = NULL;
      tk = strtok_r( line, " \t\n", &ctx );
      if ( tk ) {
        tk = strtok_r( NULL, " \t\n", &ctx );
	if ( tk ) {
	  found = 0;
	}
      }

    }

  }

  pclose( f );

  return found;

}

int fileClose (
  FILE *f
) {

struct stat buf;
int status;

  if ( diagnosticMode() ) {
    logDiagnostic( "close file\n" );
  }

  status = fstat( fileno(f), &buf );
  if ( status == -1 ) {
    if ( debugMode() ) {
      perror( "in fileClose " );
    }
    return status;
  }

  if ( buf.st_mode & S_IFIFO ) {
    return pclose( f );
  }

  return fclose( f );

}

static int checkForHttp (
  char *fullName,
  char *name
) {

unsigned int i;
int status;
char buf[255+1], namePart[255+1], postPart[255+1], *tk, *context;

  strncpy( buf, fullName, 255 );
  buf[255] = 0;

  strcpy( name, "" );

  context = NULL;
  tk = strtok_r( buf, ":", &context );

  if ( !tk ) return 0;

  for ( i=0; i<strlen(tk); i++ ) {
    tk[i] = tolower( tk[i] );
  }

  if ( ( strcmp( tk, "http" ) == 0 ) ||
       ( strcmp( tk, "HTTP" ) == 0 ) ||
       ( strcmp( tk, "https" ) == 0 ) ||
       ( strcmp( tk, "HTTPS" ) == 0 ) 
     ) {

    status = getFileName( namePart, fullName, 255 );
    if ( status & 1 ) {

      strncpy( name, namePart, 255 );
      name[255] = 0;

      status = getFilePostfix( postPart, fullName, 255 );
      if ( status & 1 ) Strncat( name, postPart, 255 );

      return 1;

    }

  }

  return 0;

}

static int fileReadable (
  char *fname )
{

FILE *f;

char nameToCheck[255+1];

int result, len1, len2, remain;

char *first, *last;

  // if filename is of the form name[parm].ext,
  // use name.ext in the "is readable check".
  first = index( fname, (int) '[' );
  if ( first ) {
    last = rindex( fname, (int) ']' );
  }
  if ( first && last && ( first < last ) ) {
    len1 = (long) first - (long) fname;
    if ( len1 > 255 ) len1 = 255;
    strncpy( nameToCheck, fname, len1 );
    nameToCheck[len1] = 0;
    remain = 255 - len1;
    len2 = strlen( fname ) - (long) last + (long) fname - 1;
    if ( len2 > remain ) len2 = remain;
    strncat( nameToCheck, last+1, len2 );
    nameToCheck[len1+len2] = 0;
  }
  else {

    strncpy( nameToCheck, fname, 255 );
    nameToCheck[255] = 0;

    // else if filename is of the form name.ext?params,
    // use name.ext in the "is readable check".
    first = index( nameToCheck, (int) '?' );
    if ( first ) {
      *first = (char) 0;
    }

    // else use fname without any changes

  }

  f = fopen( nameToCheck, "r" );
  if ( f ) {
    result = 1;
    fileClose( f );
  }
  else
    result = 0;

  return result;

}

static void discardParams (
  char *name,
  int maxlen
) {

char *first, *last;
int len1, len2, remain;
char buf[255+1];

 if ( maxlen > 255 ) maxlen = 255;
  strcpy( buf, "" );

  first = index( name, (int) '[' );
  if ( first ) {
    last = rindex( name, (int) ']' );
  }
  if ( first && last && ( first < last ) ) {

    // filename is of the form name[parm].ext

    len1 = (long) first - (long) name;
    if ( len1 > maxlen ) len1 = maxlen;
    strncpy( buf, name, len1 );
    buf[len1] = 0;
    remain = maxlen - len1;
    len2 = strlen( name ) - (long) last + (long) name - 1;
    if ( len2 > remain ) len2 = remain;
    strncat( buf, last+1, len2 );
    buf[len1+len2] = 0;

    strncpy( name, buf, maxlen );
    name[maxlen] = 0;

  }
  else {

    first = index( name, (int) '?' );

    if ( first ) {
      // filename is of the form name.ext?params
      *first = (char) 0;
    }

  }

}

static void getParams (
  char *params,
  char *fullNameWithParams,
  int maxlen
) {

char *first, *last, *ptr;
int len, i;

  strcpy( params, "" );

  first = index( fullNameWithParams, (int) '[' );
  if ( first ) {
    last = rindex( fullNameWithParams, (int) ']' );
  }
  if ( first && last && ( first < last ) ) {

    // filename is of the form name[parm].ext

    len = last - first + 1;
    if ( len > maxlen ) len = maxlen;
    if ( len < 0 ) len = 0;

    for ( i=0, ptr=first; i<len; i++, ptr++ ) {
      params[i] = *ptr;
    }
    params[i] = 0;

  }
  else {

    first = index( fullNameWithParams, (int) '?' );

    if ( first ) {
      // filename is of the form name.ext?params
      Strncat( params, first, maxlen );
    }

  }

}


static void reassemble (
  char *fullNameWithParams,
  char *fullName,
  char *params,
  int maxlen
) {

char *first;
int len, i, loc=0;

  first = index( params, (int) '[' );
  if ( first ) {

    first = 0;
    len = strlen( fullName );
    for ( i=len; i>=0; i-- ) {
      if ( fullName[i] == '.' ) {
        loc = i;
        break;
      }
    }

    for ( i=0; i<loc; i++ ) {
      fullNameWithParams[i] = fullName[i];
    }
    fullNameWithParams[i] = 0;
    Strncat( fullNameWithParams, params, maxlen );
    Strncat( fullNameWithParams, &fullName[loc], maxlen );

  }
  else {

    strncpy( fullNameWithParams, fullName, maxlen );
    Strncat( fullNameWithParams, params, maxlen );
    fullNameWithParams[maxlen] = 0;

  }

}


FILE *fileOpen (
  char *fullNameBuf,
  char *mode
) {

char fullName[255+1], params[255+1], fullNameWithParams[255+1],
 cmd[255+1], prog[255+1], oneExt[32], *filterCmd, *more;
int gotExt, i, l, startPos, status, ii, iii;
FILE *f;

#ifdef USECURL
char buf[255+1], name[255+1], allPaths[10239+1], plainName[255+1],
 tmpName[255+1], *urlList, *tk, *context;
int gotFile, useHttp, f_open_successful;
char errBuf[CURL_ERROR_SIZE+1];
CURLcode result;
mode_t curMode=0, newMode=0;
struct stat sbuf;
time_t tsDiff;
static time_t expireSeconds = 0;
char *nonInt, *expireString;
static int disableCache = 0;
#endif


#ifdef USECURL
  if ( expireSeconds == 0 ) {
    expireString = getenv( environment_str33 );
    if ( expireString ) {
      if ( strcmp( expireString, "DISABLE" ) == 0 ) {
	disableCache = 1;
      }
      expireSeconds = (time_t) strtol( expireString, &nonInt, 10 );
      if ( nonInt && strcmp( nonInt, "" ) ) {
        expireSeconds = 5;
      }
    }
    else {
      expireSeconds = 5;
    }
    if ( debugMode() ) fprintf( stderr, "disableCache = %-d\n", disableCache );
    if ( debugMode() ) fprintf( stderr, "expireSeconds = %-d\n", (int) expireSeconds );
  }
#endif

  strncpy( fullName, fullNameBuf, 255 );
  fullName[255] = 0;

  discardParams( fullName, 255 );

  getParams( params, fullNameBuf, 255 );

  if ( gInitList ) {
    gInitList = 0;
    initFilters();
  }

#ifndef USECURL
  if ( debugMode() ) fprintf( stderr, "Using local access only\n" );

  if ( strcmp( mode, "r" ) && strcmp( mode, "rb" ) ) {

    return fopen( fullName, mode );

  }

  // First find last "/"
  l = strlen( fullName );
  startPos = l;
  for ( i=l; (i>=0) && (fullName[i] != '/'); i-- ) {
    startPos = i;
  }



  gotExt = 0;
  more = strstr( &fullName[startPos], "." );
  for ( i=l; (i>=startPos) && more; i-- ) {
    if ( fullName[i] == '.' ) {
      more = NULL;
      gotExt = 1;
      for ( iii=0, ii=i; ii<l; ii++, iii++ ) {
        oneExt[iii] = fullName[ii];
      }
      oneExt[iii] = 0;
    }
  }

  if ( gotExt ) {

    filterCmd = findFilter( oneExt );
    if ( filterCmd ) {

      if ( !fileReadable( fullName ) ) return NULL;

      reassemble( fullNameWithParams, fullName, params, 255 );

      status = buildCommand( cmd, 255, prog, 255, filterCmd, fullNameWithParams );
      if ( status & 1 ) {

        if ( !filterInstalled( prog ) ) {
          fprintf( stderr, "Filter %s (%s) is not installed\n", prog, oneExt );
          return NULL;
        }

        if ( debugMode() ) fprintf( stderr, "1 Filter cmd is [%s]\n", cmd );

        f = popen( cmd, "r" );

        return f;

      }
      else {

        return NULL;

      }

    }

  }

  f = fopen( fullName, mode );

  if ( diagnosticMode() ) {
    char diagBuf[255+1];
    if ( f ) {
      snprintf( diagBuf, 255, "open [%s]\n", fullName );
      logDiagnostic( diagBuf );
    }
  }

  return f;

#endif

#ifdef USECURL

  if ( debugMode() ) fprintf( stderr, "Using curl for URL-based access\n" );

  if ( gGetUserUmask ) {
    gGetUserUmask = 0;
    tk = getenv( environment_str13 );
    if ( tk ) {
      gUseUmask = 1;
      context = NULL;
      gUmask = (mode_t) strtol( tk, &context, 0 );
      if ( debugMode() ) fprintf( stderr, "Temp dir umask = 0%-o\n", (int) gUmask );
    }
  }

  // Explicit http access (http:// embedded in name)
  useHttp = checkForHttp( fullName, plainName );
  if ( useHttp ) {

    // First find last "/"
    l = strlen( fullName );
    startPos = l;
    for ( i=l; (i>=0) && (fullName[i] != '/'); i-- ) {
      startPos = i;
    }



    gotExt = 0;
    more = strstr( &fullName[startPos], "." );
    for ( i=l; (i>=startPos) && more; i-- ) {
      if ( fullName[i] == '.' ) {
        more = NULL;
        gotExt = 1;
        for ( iii=0, ii=i; ii<l; ii++, iii++ ) {
          oneExt[iii] = fullName[ii];
        }
        oneExt[iii] = 0;
      }
    }

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

    strncpy( buf, tmpDir, 255 );
    Strncat( buf, plainName, 255 );

    if ( disableCache ) {
      status = 0;
      tsDiff = 0;
    }
    else {
      status = stat( buf, &sbuf );
      tsDiff = time( NULL ) - sbuf.st_mtime;
    }

    if ( disableCache || status || ( tsDiff > expireSeconds ) ) {

      if ( debugMode() ) fprintf( stderr, "open [%s]\n", buf );
      if ( gUseUmask ) {
        newMode = gUmask;
        curMode = umask( newMode );
      }
      f = fopen( buf, "w" );
      if ( gUseUmask ) {
        umask( curMode );
      }
      if ( !f ) return NULL;
      strncpy( tmpName, buf, 255 );
      tmpName[255] = 0;

      strncpy( buf, fullName, 255 );

      if ( debugMode() ) fprintf( stderr, "get [%s]\n", buf );

      curl_easy_setopt( curlH, CURLOPT_URL, buf );
      curl_easy_setopt( curlH, CURLOPT_FILE, f );
      curl_easy_setopt( curlH, CURLOPT_ERRORBUFFER, errBuf );
      curl_easy_setopt( curlH, CURLOPT_FAILONERROR, 1 );
      curl_easy_setopt( curlH, CURLOPT_SSL_VERIFYPEER, 0 );
      curl_easy_setopt( curlH, CURLOPT_SSL_VERIFYHOST, 0 );
      strcpy( errBuf, "" );
      result = curl_easy_perform( curlH );
      if ( debugMode() ) fprintf( stderr, "1 result = %-d, errno = %-d\n",
       (int) result, errno );
      if ( debugMode() ) fprintf( stderr, "1 errBuf = [%s]\n", errBuf );

      fclose( f );

      if ( result ) {
        if ( debugMode() ) fprintf( stderr, "unlink [%s]\n", tmpName );
        unlink( tmpName );
        return NULL;
      }

    }

    strncpy( buf, tmpDir, 255 );
    Strncat( buf, plainName, 255 );

    if ( gotExt ) {

      filterCmd = findFilter( oneExt );
      if ( filterCmd ) {

        reassemble( fullNameWithParams, buf, params, 255 );

        status = buildCommand( cmd, 255, prog, 255, filterCmd, fullNameWithParams );
        if ( status & 1 ) {

          if ( !filterInstalled( prog ) ) {
            fprintf( stderr, "Filter %s (%s) is not installed\n", prog, oneExt );
            return NULL;
          }

          if ( debugMode() ) fprintf( stderr, "2 Filter cmd is [%s]\n", cmd );

          f = popen( cmd, "r" );

          return f;

        }
        else {

          return NULL;

        }

      }

    }

    f = fopen( buf, "r" );

    if ( diagnosticMode() ) {
      char diagBuf[255+1];
      if ( f ) {
        snprintf( diagBuf, 255, "open [%s]\n", buf );
        logDiagnostic( diagBuf );
      }
    }

    return f;

  }

  urlList = getenv( environment_str9 );
  if ( !urlList ) {

    if ( strcmp( mode, "r" ) && strcmp( mode, "rb" ) ) {

      return fopen( fullName, mode );

    }

    // First find last "/"
    l = strlen( fullName );
    startPos = l;
    for ( i=l; (i>=0) && (fullName[i] != '/'); i-- ) {
      startPos = i;
    }

    gotExt = 0;
    more = strstr( &fullName[startPos], "." );
    for ( i=l; (i>=startPos) && more; i-- ) {
      if ( fullName[i] == '.' ) {
        more = NULL;
        gotExt = 1;
        for ( iii=0, ii=i; ii<l; ii++, iii++ ) {
          oneExt[iii] = fullName[ii];
        }
        oneExt[iii] = 0;
      }
    }

    if ( gotExt ) {

      filterCmd = findFilter( oneExt );
      if ( filterCmd ) {

        if ( !fileReadable( fullName ) ) return NULL;

        reassemble( fullNameWithParams, fullName, params, 255 );

        status = buildCommand( cmd, 255, prog, 255, filterCmd, fullNameWithParams );
        if ( status & 1 ) {

          if ( !filterInstalled( prog ) ) {
            fprintf( stderr, "Filter %s (%s) is not installed\n", prog, oneExt );
            return NULL;
          }

          if ( debugMode() ) fprintf( stderr, "3 Filter cmd is [%s]\n", cmd );

          f = popen( cmd, "r" );

          return f;

        }
        else {

          return NULL;

        }

      }

    }

    f = fopen( fullName, mode );

    if ( diagnosticMode() ) {
      char diagBuf[255+1];
      if ( f ) {
        snprintf( diagBuf, 255, "open [%s]\n", fullName );
        logDiagnostic( diagBuf );
      }
    }

    return f;

  }

  // First find last "/"
  l = strlen( fullName );
  startPos = l;
  for ( i=l; (i>=0) && (fullName[i] != '/'); i-- ) {
    startPos = i;
  }


  gotExt = 0;
  more = strstr( &fullName[startPos], "." );
  for ( i=l; (i>=startPos) && more; i-- ) {
    if ( fullName[i] == '.' ) {
      more = NULL;
      gotExt = 1;
      for ( iii=0, ii=i; ii<l; ii++, iii++ ) {
        oneExt[iii] = fullName[ii];
      }
      oneExt[iii] = 0;
    }
  }

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

  if ( debugMode() ) fprintf( stderr, "Open file [%s] [%s]\n", name, mode );

  if ( !strcmp( mode, "r" ) || !strcmp( mode, "rb" ) ) {

    status = getFileNameAndExt( name, fullName, 255 );
    if ( !(status & 1 ) ) {
      strncpy( name, fullName, 255 );
      name[255] = 0;
    }

    strncpy( allPaths, urlList, 10239 );
    allPaths[10239] = 0;
    context = NULL;
    tk = strtok_r( allPaths, "|", &context );

    gotFile = 0;
    result = (CURLcode) -1;
    f_open_successful = 0;

    while ( tk && !gotFile ) {

      l = strlen(tk);
      if ( tk[l-1] == '/' ) {
        tk[l-1] = 0;
      }

      strncpy( buf, tmpDir, 255 );
      Strncat( buf, name, 255 );

      if ( disableCache ) {
        status = 0;
        tsDiff = 0;
      }
      else {
        status = stat( buf, &sbuf );
        tsDiff = time( NULL ) - sbuf.st_mtime;
      }

      if ( disableCache || status || ( tsDiff > expireSeconds ) ) {

        if ( debugMode() ) fprintf( stderr, "open [%s]\n", buf );
        if ( gUseUmask ) {
          newMode = gUmask;
          curMode = umask( newMode );
        }
        f = fopen( buf, "w" );
        if ( gUseUmask ) {
          umask( curMode );
        }
        if ( !f ) return NULL;
        f_open_successful = 1;
        strncpy( tmpName, buf, 255 );
        tmpName[255] = 0;

        strcpy( buf, tk );
        Strncat( buf, fullName, 255 );

        if ( debugMode() ) fprintf( stderr, "get [%s]\n", buf );

        curl_easy_setopt( curlH, CURLOPT_URL, buf );
        curl_easy_setopt( curlH, CURLOPT_FILE, f );
        curl_easy_setopt( curlH, CURLOPT_ERRORBUFFER, errBuf );
        curl_easy_setopt( curlH, CURLOPT_FAILONERROR, 1 );
        curl_easy_setopt( curlH, CURLOPT_SSL_VERIFYPEER, 0 );
        curl_easy_setopt( curlH, CURLOPT_SSL_VERIFYHOST, 0 );
        strcpy( errBuf, "" );
        result = curl_easy_perform( curlH );
        if ( debugMode() ) fprintf( stderr, "2 result = %-d, errno = %-d\n",
         (int) result, errno );
        if ( debugMode() ) fprintf( stderr, "2 errBuf = [%s]\n", errBuf );

        fclose( f );

        if ( result ) {
          if ( debugMode() ) fprintf( stderr, "2 unlink [%s]\n", tmpName );
          unlink( tmpName );
        }

        gotFile = !result;

        tk = strtok_r( NULL, "|", &context );

      }
      else {

        result = (CURLcode) 0; // reuse existing file
	tk = NULL;
	gotFile = 1;

      }

    }

    f = NULL;

    if ( result ) {
      return NULL;
    }

    strncpy( buf, tmpDir, 255 );
    Strncat( buf, name, 255 );

    if ( gotExt ) {

      filterCmd = findFilter( oneExt );
      if ( filterCmd ) {

        reassemble( fullNameWithParams, buf, params, 255 );

        status = buildCommand( cmd, 255, prog, 255, filterCmd, fullNameWithParams );
        if ( status & 1 ) {

          if ( !filterInstalled( prog ) ) {
            fprintf( stderr, "Filter %s (%s) is not installed\n", prog, oneExt );
            return NULL;
          }

          if ( debugMode() ) fprintf( stderr, "4 Filter cmd is [%s]\n", cmd );

          f = popen( cmd, "r" );

          return f;

        }
        else {

          return NULL;

        }

      }

    }

    f = fopen( buf, "r" );

    if ( diagnosticMode() ) {
      char diagBuf[255+1];
      if ( f ) {
        snprintf( diagBuf, 255, "open [%s]\n", buf );
        logDiagnostic( diagBuf );
      }
    }

  }
  else {

    f = fopen( name, mode );

  }

  return f;

#endif

}
