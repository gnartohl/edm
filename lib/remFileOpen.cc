#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include "sys_types.h"
#include "avl.h"
#include "environment.str"

int debugMode ( void );

typedef struct nameListTag {
  AVL_FIELDS(nameListTag)
  char *ext;
  char *cmd;
} nameListType, *nameListPtr;

static int gPipeIsOpen = 0;
static FILE *gPipeF = NULL;
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
int l, stat, dup;
FILE *f;
nameListPtr cur;

  stat = avl_init_tree( compare_nodes,
   compare_key, copy_node, &gFilterH );

  ptr = getenv( "EDMFILTERS" );
  if ( !ptr ) return;

  strncpy( file, ptr, 255 );
  file[255] = 0;

  l = strlen(ptr);
  if ( ptr[l-1] != '/' ) {
    Strncat( file, "/edmFilters", 255 );
  }
  else {
    Strncat( file, "edmFilters", 255 );
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

        stat = avl_insert_node( gFilterH, (void *) cur, &dup );
	if ( !( stat & 1 ) ) goto errRet;

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

  stat = avl_get_first( gFilterH, (void **) &cur );
  if ( !( stat & 1 ) ) goto errRet;

  while ( cur ) {

    stat = avl_get_next( gFilterH, (void **) &cur );
    if ( !( stat & 1 ) ) goto errRet;

  }

errRet:

  fclose( f );

}

static char *findFilter (
  char *oneExt
) {

int stat;
nameListPtr cur;

  if ( !gFilterH ) return NULL;

  stat = avl_get_match( gFilterH, (void *) oneExt, (void **) &cur );
  if ( !( stat & 1 ) ) return NULL;
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

  if ( gPipeIsOpen ) {

    if ( f == gPipeF ) {

      gPipeF = NULL;
      gPipeIsOpen = 0;
      return pclose( f );

    }

  }

  return fclose( f );

}

void _edmDebug ( void );

FILE *fileOpen (
  char *fullName,
  char *mode
) {

char cmd[255+1], prog[255+1];
char oneExt[32], *oneExtPtr, *filterCmd, *ptr1, *ptr2, *ptr3;
int gotExt, i, l, startPos, stat;

#ifdef USECURL
FILE *f;
char buf[255+1], name[255+1], allPaths[10239+1], *urlList, *tk, *context;
int gotFile;
char errBuf[CURL_ERROR_SIZE+1];
CURLcode result;
#endif

  _edmDebug();

  if ( gInitList ) {
    gInitList = 0;
    initFilters();
  }

#ifndef USECURL
  if ( debugMode() ) printf( "Using local access only\n" );

  if ( strcmp( mode, "r" ) != 0 ) {

    return fopen( fullName, mode );

  }

  // First find last "/"
  l = strlen( fullName );
  startPos = l;
  for ( i=l; (i>=0) && (fullName[i] != '/'); i-- ) {
    startPos = i;
  }

  gotExt = 0;
  ptr1 = strstr( &fullName[startPos], "." );
  if ( ptr1 ) {
    ptr2 = strstr( &ptr1[1], "."  );
    if ( ptr2 ) {
      l = strlen( ptr2 );
      if ( ( l < 25 ) && ( strcmp( ptr2, ".edl" ) == 0 ) ) {
        oneExtPtr = oneExt;
        for ( ptr3=ptr1; ptr3<ptr2; ptr3++ ) {
          *oneExtPtr++ = *ptr3;
        }
        *oneExtPtr = 0;
        gotExt = 1;
        *ptr2 = 0;
      }
    }
  }

  if ( gotExt ) {

    filterCmd = findFilter( oneExt );
    if ( filterCmd ) {

      stat = buildCommand( cmd, 255, prog, 255, filterCmd, fullName );
      if ( stat & 1 ) {

        if ( !filterInstalled( prog ) ) {
          printf( "Filter %s (%s) is not installed\n", prog, oneExt );
          return NULL;
        }

        if ( gPipeIsOpen ) {
          printf( "Pipe is already open\n" );
          return NULL;
        }

        gPipeIsOpen = 1;

	if ( debugMode() ) printf( "Filter cmd is [%s]\n", cmd );

        // change extension, if any, to .edl
        ptr1 = ptr2 = strstr( fullName, oneExt );
        while ( ptr2 ) {
          ptr2 = strstr( &ptr1[1], oneExt );
          if ( ptr2 ) {
            ptr1 = ptr2;
          }
        }
        if ( ptr1 ) {
          *ptr1 = 0;
          strcat( fullName, ".edl" );
        }

        gPipeF = popen( cmd, "r" );

        return gPipeF;

      }
      else {

        return NULL;

      }

    }

  }

  return fopen( fullName, mode );

#endif

#ifdef USECURL

  if ( debugMode() ) printf( "Using curl for URL-based access\n" );

  urlList = getenv( environment_str9 );
  if ( !urlList ) {

    if ( strcmp( mode, "r" ) != 0 ) {

      return fopen( fullName, mode );

    }

    // First find last "/"
    l = strlen( fullName );
    startPos = l;
    for ( i=l; (i>=0) && (fullName[i] != '/'); i-- ) {
      startPos = i;
    }

    gotExt = 0;
    ptr1 = strstr( &fullName[startPos], "." );
    if ( ptr1 ) {
      ptr2 = strstr( &ptr1[1], "."  );
      if ( ptr2 ) {
        l = strlen( ptr2 );
        if ( ( l < 25 ) && ( strcmp( ptr2, ".edl" ) == 0 ) ) {
          oneExtPtr = oneExt;
          for ( ptr3=ptr1; ptr3<ptr2; ptr3++ ) {
            *oneExtPtr++ = *ptr3;
          }
          *oneExtPtr = 0;
          gotExt = 1;
          *ptr2 = 0;
        }
      }
    }

    if ( gotExt ) {

      filterCmd = findFilter( oneExt );
      if ( filterCmd ) {

        stat = buildCommand( cmd, 255, prog, 255, filterCmd, fullName );
        if ( stat & 1 ) {

          if ( !filterInstalled( prog ) ) {
            printf( "Filter %s (%s) is not installed\n", prog, oneExt );
            return NULL;
          }

          if ( gPipeIsOpen ) {
            printf( "Pipe is already open\n" );
            return NULL;
          }

          gPipeIsOpen = 1;

          if ( debugMode() ) printf( "Filter cmd is [%s]\n", cmd );

          // change extension, if any, to .edl
          ptr1 = ptr2 = strstr( fullName, oneExt );
          while ( ptr2 ) {
            ptr2 = strstr( &ptr1[1], oneExt );
            if ( ptr2 ) {
              ptr1 = ptr2;
            }
          }
          if ( ptr1 ) {
            *ptr1 = 0;
            strcat( fullName, ".edl" );
          }

          gPipeF = popen( cmd, "r" );

          return gPipeF;

        }
        else {

          return NULL;

        }

      }

    }

    return fopen( fullName, mode );

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
