//  edm - extensible display manager

//  Copyright (C) 1999-2016 John W. Sinclair

//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "app_pkg.h"

#include "thread.h"
#include "crc.h"
#include "edm.version"
#include <unistd.h>

#include <X11/Intrinsic.h>
#include <sys/wait.h>

#define SMALL_SYM_ARRAY_SIZE 10
#define SMALL_SYM_ARRAY_LEN 31

typedef struct libRecTag {
  struct libRecTag *flink;
  char *className;
  char *fileName;
  char *typeName;
  char *text;
} libRecType, *libRecPtr;

static int g_needXtInit = 1;

static int checkDisplay (
  char *dspName
) {

char *envPtr;
char cmd[1023+1];
int result;

  result = 0; // success

  envPtr = getenv( environment_str37 );
  if ( envPtr ) {

    if ( !dspName ) return result;
    if ( strcmp( dspName, "" ) == 0 ) return result;

    if ( debugMode() ) {
      fprintf( stderr, "checking display %s with %s\n", dspName, envPtr );
    }

    snprintf( cmd, 1023, "%s %s", envPtr, dspName );

    result = system( cmd );

    if ( result ) {
      result = result >> 8;
    }

    if ( debugMode() ) {
      fprintf( stderr, "return value = %-d\n", result );
    }

  }

  return result;

}

static int ignoreIconic ( void ) {

char *envPtr;
static int flag = -1;

  if ( flag == -1 ) {
    envPtr = getenv( environment_str19 );
    if ( envPtr ) {
      flag = 1;
    }
    else {
      flag = 0;
    }
  }

  return flag;

}

static void extractPosition (
  char *str,
  char *filePart,
  int max,
  int *gotPosition,
  int *posx,
  int *posy
) {

char buf[1023+1], *tk, *ctx, *err;
int ok;

  strncpy( buf, str, 1023 );
  buf[1023] = 0;

  ok = 1;

  ctx = NULL;
  tk = strtok_r( buf, "?", &ctx );

  if ( tk ) {

    strncpy( filePart, tk, max );
    filePart[max] = 0;

    tk = strtok_r( NULL, "?", &ctx );

    if ( tk ) {

      err = NULL;
      *posx = strtol( tk, &err, 0 );
      if ( err ) {
	if ( strcmp( err, "" ) != 0 ) {
	  ok = 0;
	}
      }

      tk = strtok_r( NULL, "?", &ctx );

      if ( tk ) {

        err = NULL;
        *posy = strtol( tk, &err, 0 );
        if ( err ) {
	  if ( strcmp( err, "" ) != 0 ) {
	    ok = 0;
	  }
        }

      }
      else {

        ok = 0;

      }

    }
    else {

      ok = 0;

    }

  }
  else {

    ok = 0;

  }

  if ( ok ) {
    *gotPosition = 1;
  }
  else {
    strncpy( filePart, str, max );
    filePart[max] = 0;
    *gotPosition = 0;
    *posx = 0;
    *posy = 0;
  }

}

static void extractWinName (
  char *str,
  int max,
  char *name,
  int maxName
) {

char buf[1023+1], *tk, *ctx;

  if ( strstr( str, "=" ) ) {

    strncpy( buf, str, 1023 );
    buf[1023] = 0;

    ctx = NULL;
    tk = strtok_r( buf, "= 	", &ctx ); // =, space, tab

    if ( tk ) {

      strncpy( name, tk, maxName );
      name[maxName] = 0;

      tk = strtok_r( NULL, "= 	", &ctx ); // =, space, tab

      if ( tk ) {

        strncpy( str, tk, max );
        str[max] = 0;

      }
      else {

        strcpy( str, "" );

      }

    }

  } // else, no name given

}

static int httpPath (
  char *path
 ) {

  if ( strstr( path, "http://" ) ) return 1;
  if ( strstr( path, "HTTP://" ) ) return 1;
  if ( strstr( path, "https://" ) ) return 1;
  if ( strstr( path, "HTTPs://" ) ) return 1;

  return 0;

}

static void fixupHttpPart (
  char *path
 ) {

char *ptr;
int more, count = 100;

  do {

    more = 0;

    ptr = strstr( path, "http://" );
    if ( ptr ) {
      more = 1;
      ptr[4] = '|';
    }
    else {
      ptr = strstr( path, "HTTP://" );
      if ( ptr ) {
        more = 1;
        ptr[4] = '|';
      }
      else {
        ptr = strstr( path, "https://" );
        if ( ptr ) {
          more = 1;
          ptr[5] = '|';
        }
	else {
          ptr = strstr( path, "HTTPS://" );
          if ( ptr ) {
            more = 1;
            ptr[5] = '|';
          }
	}
      }
    }

    count--;

  } while ( more && count );

}

static void undoFixupHttpPart (
  char *path
 ) {

char *ptr;
int more, count = 100;

  do {

    more = 0;

    ptr = strstr( path, "http|//" );
    if ( ptr ) {
      more = 1;
      ptr[4] = ':';
    }
    else {
      ptr = strstr( path, "HTTP|//" );
      if ( ptr ) {
        more = 1;
        ptr[4] = ':';
      }
      else {
        ptr = strstr( path, "https|//" );
        if ( ptr ) {
          more = 1;
          ptr[5] = ':';
        }
	else {
          ptr = strstr( path, "HTTPS|//" );
          if ( ptr ) {
            more = 1;
            ptr[5] = ':';
          }
	}
      }
    }

    count--;

  } while ( more && count );

}

static int compare_nodes (
  void *node1,
  void *node2
) {

schemeListPtr p1, p2;

  p1 = (schemeListPtr) node1;
  p2 = (schemeListPtr) node2;

  return strcmp( p1->objName, p2->objName );

}

static int compare_key (
  void *key,
  void *node
) {

schemeListPtr p;
char *oneIndex;

  p = (schemeListPtr) node;
  oneIndex = (char *) key;

  return strcmp( oneIndex, p->objName );

}

static int copy_nodes (
  void *node1,
  void *node2
) {

schemeListPtr p1, p2;

  p1 = (schemeListPtr) node1;
  p2 = (schemeListPtr) node2;

  *p1 = *p2;

  // give p1 a copy of the object name
  if ( p2->objName ) {
    p1->objName = new char[ strlen(p2->objName) + 1 ];
    strcpy( p1->objName, p2->objName );
  }

  // give p1 a copy of the file name
  if ( p2->fileName ) {
    p1->fileName = new char[ strlen(p2->fileName) + 1 ];
    strcpy( p1->fileName, p2->fileName );
  }

  return 1;

}

static void manageComponents (
  char *op,
  char *libFile )
{

typedef int (*REGFUNC)( char **, char **, char ** );
REGFUNC func;
typedef char *(*CHARFUNC)( void );
CHARFUNC cfunc;

int stat, index, comment, fileExists, fileEmpty, doAdd, alreadyExists;
char *classNamePtr, *typeNamePtr, *textPtr, *error;
void *dllHandle;
char fileName[255+1], prefix[255+1], line[255+1], buf[255+1];
int numComponents = 0;
int numToAdd = 0;
int numToRemove = 0;
char *envPtr, *tk, *more;
FILE *f;
libRecPtr head, tail, cur, prev, next;
//char *author;

  head = new libRecType;
  tail = head;
  tail->flink = NULL;

  envPtr = getenv(environment_str3);
  if ( envPtr ) {
    strncpy( prefix, envPtr, 255 );
    if ( prefix[strlen(prefix)-1] != '/' ) Strncat( prefix, "/", 255 );
  }
  else {
    strcpy( prefix, "/etc/edm/" );
  }

  strncpy( fileName, prefix, 255 );
  Strncat( fileName, "edmObjects", 255 );

  f = fopen( fileName, "r" );
  if ( f ) {

    fprintf( stderr, appContextClass_str1, fileName );

    fileExists = 1;
    fileEmpty = 0;

    // read in existing components

    more = fgets( line, 255, f );
    if ( more ) {
      tk = strtok( line, "\n" );
      numComponents = atol( tk );
      if ( numComponents <= 0 ) {
        fprintf( stderr, appContextClass_str2, fileName );
        fileEmpty = 1;
        fclose( f );
      }
    }
    else {
      fprintf( stderr, appContextClass_str2, fileName );
      fileEmpty = 1;
      fclose( f );
    }

    if ( !fileEmpty ) {

      index = 0;
      do {

        more = fgets( line, 255, f );
        if ( more ) {

          cur = new libRecType;

          strncpy( buf, line, 255 );

          comment = 0;
          tk = strtok( buf, " \t\n" );

          if ( !tk ) {
            comment = 1;
          }
          else if ( tk[0] == '#' ) {
            comment = 1;
          }

          if ( !comment ) { /* not an empty line or comment */

            tail->flink = cur;
            tail = cur;
            tail->flink = NULL;

            tk = strtok( line, " \t\n" );
            if ( !tk ) {
              fprintf( stderr, appContextClass_str3 );
              return;
            }
            cur->className = new char[strlen(tk)+1];
            strcpy( cur->className, tk );

            tk = strtok( NULL, " \t\n" );
            if ( !tk ) {
              fprintf( stderr, appContextClass_str3 );
              return;
            }
            cur->fileName = new char[strlen(tk)+1];
            strcpy( cur->fileName, tk );

            tk = strtok( NULL, " \t\n" );
            if ( !tk ) {
              fprintf( stderr, appContextClass_str3 );
              return;
            }
            cur->typeName = new char[strlen(tk)+1];
            strcpy( cur->typeName, tk );

            tk = strtok( NULL, "\n" );
            if ( !tk ) {
              fprintf( stderr, appContextClass_str3 );
              return;
            }
            cur->text = new char[strlen(tk)+1];
            strcpy( cur->text, tk );

            index++;

          }

        }

      } while ( more );

      fclose( f );

      if ( index != numComponents ) {
        fprintf( stderr, appContextClass_str4, fileName );
        return;
      }

    }
    else {

      fileExists = 0; // file was empty so behave as if file does not exist

    }

  }
  else {
    fileExists = 0;
  }

  if ( libFile[0] != '/' ) {
    fprintf( stderr, appContextClass_str5 );
    fprintf( stderr, appContextClass_str6 );
    return;
  }

  dllHandle = dlopen( libFile, RTLD_LAZY );
  if ((error = dlerror()) != NULL)  {
    fputs(error, stderr);
    fputs( "\n", stderr );
    return;
  }

  if ( strcmp( op, global_str8 ) == 0 ) {  // show

    fprintf( stderr, "\n" );

    strcpy( line, "version" );
    cfunc = (CHARFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) == NULL)  {
      fprintf( stderr, "Built with edm version: %s\n", (*cfunc)() );
    }
    else {
      fprintf( stderr, "edm version not registered\n" );
    }

    fprintf( stderr, "\n" );

    strcpy( line, "author" );
    cfunc = (CHARFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) == NULL)  {
      fprintf( stderr, "Author: %s\n", (*cfunc)() );
    }
    else {
      fprintf( stderr, "Author name not registered\n" );
    }

    fprintf( stderr, "\n" );

    strcpy( line, "firstRegRecord" );
    func = (REGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str8, stderr );
      return;
    }

    stat = (*func)( &classNamePtr, &typeNamePtr, &textPtr );

    strcpy( line, "nextRegRecord" );
    func = (REGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str9, stderr );
      return;
    }

    if ( !stat ) {
      strncpy( line, appContextClass_str10, 255 );
      Strncat( line, appContextClass_str11, 255 );
      fprintf( stderr, "%s\n\n", line );
    }

    while ( !stat ) {

      strncpy( line, classNamePtr, 255 );

      if ( strlen(line) < 37 )
        index = 42;
      else
        index = strlen(line) + 5;
      Strncat( line, "                                             ", 255 );
      strncpy( &line[index], typeNamePtr, 255-index );

      if ( strlen(line) < 50 )
        index = 55;
      else
        index = strlen(line) + 5;
      Strncat( line, "                                                       ", 255 );
      strncpy( &line[index], textPtr, 255-index );

      fprintf( stderr, "%s\n", line );

      stat = (*func)( &classNamePtr, &typeNamePtr, &textPtr );

    }

    fprintf( stderr, "\n\n" );

  }
  else if ( strcmp( op, global_str6 ) == 0 ) {  // add

    if ( !fileExists ) {
      fprintf( stderr, appContextClass_str13, fileName );
    }

    fprintf( stderr, "\n" );

    strcpy( line, "firstRegRecord" );
    func = (REGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str14, stderr );
      return;
    }

    stat = (*func)( &classNamePtr, &typeNamePtr, &textPtr );

    strcpy( line, "nextRegRecord" );
    func = (REGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str9, stderr );
      return;
    }

    numToAdd = 0;
    alreadyExists = 0;
    while ( !stat ) {

      doAdd = 1;

      cur = head->flink;
      while ( cur ) {
        if ( strcmp( classNamePtr, cur->className ) == 0 ) {
          fprintf( stderr, appContextClass_str15, classNamePtr );
          doAdd = 0;
          alreadyExists = 1;
          break;
	}
        cur = cur->flink;
      }

      if ( doAdd ) {

        numToAdd++;

        fprintf( stderr, appContextClass_str16,
         classNamePtr, typeNamePtr, textPtr );

        cur = new libRecType;
        tail->flink = cur;
        tail = cur;
        tail->flink = NULL;

        cur->className = new char[strlen(classNamePtr)+1];
        strcpy( cur->className, classNamePtr );
        cur->fileName = new char[strlen(libFile)+1];
        strcpy( cur->fileName, libFile );
        cur->typeName = new char[strlen(typeNamePtr)+1];
        strcpy( cur->typeName, typeNamePtr );
        cur->text = new char[strlen(textPtr)+1];
        strcpy( cur->text, textPtr );

        numComponents++;

      }

      stat = (*func)( &classNamePtr, &typeNamePtr, &textPtr );

    }

    if ( numToAdd == 0 ) {
      fprintf( stderr, "\n" );
      if ( alreadyExists ) {
        fprintf( stderr, appContextClass_str122, fileName );
      }
      else {
        fprintf( stderr,
         appContextClass_str17, fileName );
      }
      return;
    }

    fprintf( stderr, "\n" );

    strncpy( line, fileName, 255 );
    Strncat( line, "~", 255 );

    if ( fileExists ) {

      stat = unlink( line );

      fprintf( stderr, appContextClass_str18, line );
      stat = rename( fileName, line );
      if ( stat ) {
        perror( appContextClass_str19 );
        return;
      }

    }

    f = fopen( fileName, "w" );
    if ( f ) {

      fprintf( f, "%-d\n", numComponents );
      cur = head->flink;
      while ( cur ) {
        fprintf( f, "%s %s %s %s\n", cur->className, cur->fileName,
         cur->typeName, cur->text );
        cur = cur->flink;
      }

    }
    else {
      perror( fileName );
      return;
    }

    if ( numToAdd == 1 )
      fprintf( stderr, appContextClass_str20 );
    else if ( numToAdd > 1 )
      fprintf( stderr, appContextClass_str21 );

  }
  else if ( strcmp( op, global_str7 ) == 0 ) {  // remove

    if ( !fileExists ) {
      fprintf( stderr, appContextClass_str123 );
      return;
    }

    fprintf( stderr, "\n" );

    strcpy( line, "firstRegRecord" );
    func = (REGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str14, stderr );
      return;
    }

    stat = (*func)( &classNamePtr, &typeNamePtr, &textPtr );

    strcpy( line, "nextRegRecord" );
    func = (REGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str9, stderr );
      return;
    }

    numToRemove = 0;
    while ( !stat ) {

      cur = head->flink;
      prev = head;
      while ( cur ) {
        next = cur->flink;
        if ( strcmp( classNamePtr, cur->className ) == 0 ) {
          fprintf( stderr, appContextClass_str124, classNamePtr );
          numToRemove++;
          prev->flink = next;
          delete cur;
          break;
	}
        else {
          prev = cur;
	}
        cur = next;
      }

      stat = (*func)( &classNamePtr, &typeNamePtr, &textPtr );

    }

    if ( numToRemove == 0 ) {
      fprintf( stderr, appContextClass_str125, fileName );
      return;
    }

    // count remaining components
    numComponents = 0;
    cur = head->flink;
    while ( cur ) {
      numComponents++;
      cur = cur->flink;
    }

    fprintf( stderr, "\n" );

    strncpy( line, fileName, 255 );
    Strncat( line, "~", 255 );

    if ( fileExists ) {

      stat = unlink( line );

      fprintf( stderr, appContextClass_str18, line );
      stat = rename( fileName, line );
      if ( stat ) {
        perror( appContextClass_str19 );
        return;
      }

    }

    f = fopen( fileName, "w" );
    if ( f ) {

      fprintf( f, "%-d\n", numComponents );
      cur = head->flink;
      while ( cur ) {
        fprintf( f, "%s %s %s %s\n", cur->className, cur->fileName,
         cur->typeName, cur->text );
        cur = cur->flink;
      }

    }
    else {
      perror( fileName );
      return;
    }

    if ( numToRemove == 1 )
      fprintf( stderr, appContextClass_str126 );
    else if ( numToRemove > 1 )
      fprintf( stderr, appContextClass_str127 );

    if ( numComponents == 0 ) {
      fprintf( stderr, appContextClass_str128 );
    }

  }

}

static void managePvComponents (
  char *op,
  char *libFile )
{

typedef int (*PVREGFUNC)( char **, char ** );
PVREGFUNC func;
typedef char *(*CHARFUNC)( void );
CHARFUNC cfunc;

int stat, index, comment, fileExists, fileEmpty, doAdd, alreadyExists;
char *classNamePtr, *textPtr, *error;
void *dllHandle;
char fileName[255+1], prefix[255+1], line[255+1], buf[255+1];
int numComponents = 0;
int numToAdd = 0;
int numToRemove = 0;
char *envPtr, *tk, *more;
FILE *f;
libRecPtr head, tail, cur, prev, next;

  head = new libRecType;
  tail = head;
  tail->flink = NULL;

  envPtr = getenv(environment_str3);
  if ( envPtr ) {
    strncpy( prefix, envPtr, 255 );
    if ( prefix[strlen(prefix)-1] != '/' ) Strncat( prefix, "/", 255 );
  }
  else {
    strcpy( prefix, "/etc/edm/" );
  }

  strncpy( fileName, prefix, 255 );
  Strncat( fileName, "edmPvObjects", 255 );

  f = fopen( fileName, "r" );
  if ( f ) {

    fprintf( stderr, appContextClass_str1, fileName );

    fileExists = 1;
    fileEmpty = 0;

    // read in existing components

    more = fgets( line, 255, f );
    if ( more ) {
      tk = strtok( line, "\n" );
      numComponents = atol( tk );
      if ( numComponents <= 0 ) {
        fprintf( stderr, appContextClass_str2, fileName );
        fileEmpty = 1;
        fclose( f );
      }
    }
    else {
      fprintf( stderr, appContextClass_str2, fileName );
      fileEmpty = 1;
      fclose( f );
    }

    if ( !fileEmpty ) {

      index = 0;
      do {

        more = fgets( line, 255, f );
        if ( more ) {

          cur = new libRecType;

          strncpy( buf, line, 255 );

          comment = 0;
          tk = strtok( buf, " \t\n" );

          if ( !tk ) {
            comment = 1;
          }
          else if ( tk[0] == '#' ) {
            comment = 1;
          }

          if ( !comment ) { /* not an empty line or comment */

            tail->flink = cur;
            tail = cur;
            tail->flink = NULL;

            tk = strtok( line, " \t\n" );
            if ( !tk ) {
              fprintf( stderr, appContextClass_str3 );
              return;
            }
            cur->className = new char[strlen(tk)+1];
            strcpy( cur->className, tk );

            tk = strtok( NULL, " \t\n" );
            if ( !tk ) {
              fprintf( stderr, appContextClass_str3 );
              return;
            }
            cur->fileName = new char[strlen(tk)+1];
            strcpy( cur->fileName, tk );

            tk = strtok( NULL, "\n" );
            if ( !tk ) {
              fprintf( stderr, appContextClass_str3 );
              return;
            }
            cur->text = new char[strlen(tk)+1];
            strcpy( cur->text, tk );

            index++;

          }

        }

      } while ( more );

      fclose( f );

      if ( index != numComponents ) {
        fprintf( stderr, appContextClass_str4, fileName );
        return;
      }

    }
    else {

      fileExists = 0; // file was empty so behave as if file does not exist

    }

  }
  else {
    fileExists = 0;
  }

  if ( libFile[0] != '/' ) {
    fprintf( stderr, appContextClass_str5 );
    fprintf( stderr, appContextClass_str6 );
    return;
  }

  dllHandle = dlopen( libFile, RTLD_LAZY );
  if ((error = dlerror()) != NULL)  {
    fputs(error, stderr);
    fputs( "\n", stderr );
    return;
  }

  if ( strcmp( op, global_str65 ) == 0 ) {  // showpv

    fprintf( stderr, "\n" );

    strcpy( line, "version" );
    cfunc = (CHARFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) == NULL)  {
      fprintf( stderr, "Built with edm version: %s\n", (*cfunc)() );
    }
    else {
      fprintf( stderr, "edm version not registered\n" );
    }

    fprintf( stderr, "\n" );

    strcpy( line, "firstPvRegRecord" );
    func = (PVREGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str8, stderr );
      return;
    }

    stat = (*func)( &classNamePtr, &textPtr );

    strcpy( line, "nextPvRegRecord" );
    func = (PVREGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str9, stderr );
      return;
    }

    if ( !stat ) {
      strncpy( line, appContextClass_str110, 255 );
      Strncat( line, appContextClass_str111, 255 );
      fprintf( stderr, "%s\n\n", line );
    }

    while ( !stat ) {

      strncpy( line, classNamePtr, 255 );

      if ( strlen(line) < 45 )
        index = 45;
      else
        index = strlen(line) + 5;
      Strncat( line, "                                             ", 255 );
      strncpy( &line[index], textPtr, 255-index );

      fprintf( stderr, "%s\n", line );

      // get next
      stat = (*func)( &classNamePtr, &textPtr );

    }

    fprintf( stderr, "\n\n" );

  }
  else if ( strcmp( op, global_str63 ) == 0 ) {  // addpv

    if ( !fileExists ) {
      fprintf( stderr, appContextClass_str13, fileName );
    }

    fprintf( stderr, "\n" );

    strcpy( line, "firstPvRegRecord" );
    func = (PVREGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str14, stderr );
      return;
    }

    stat = (*func)( &classNamePtr, &textPtr );

    strcpy( line, "nextPvRegRecord" );
    func = (PVREGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str9, stderr );
      return;
    }

    numToAdd = 0;
    alreadyExists = 0;
    while ( !stat ) {

      doAdd = 1;

      cur = head->flink;
      while ( cur ) {
        if ( strcmp( classNamePtr, cur->className ) == 0 ) {
          fprintf( stderr, appContextClass_str15, classNamePtr );
          doAdd = 0;
          alreadyExists = 1;
	}
        cur = cur->flink;
      }

      if ( doAdd ) {

        numToAdd++;

        fprintf( stderr, appContextClass_str109, classNamePtr, textPtr );

        cur = new libRecType;
        tail->flink = cur;
        tail = cur;
        tail->flink = NULL;

        cur->className = new char[strlen(classNamePtr)+1];
        strcpy( cur->className, classNamePtr );
        cur->fileName = new char[strlen(libFile)+1];
        strcpy( cur->fileName, libFile );
        cur->text = new char[strlen(textPtr)+1];
        strcpy( cur->text, textPtr );

        numComponents++;

      }

      stat = (*func)( &classNamePtr, &textPtr );

    }

    if ( numToAdd == 0 ) {
      fprintf( stderr, "\n" );
      if ( alreadyExists ) {
        fprintf( stderr, appContextClass_str122, fileName );
      }
      else {
        fprintf( stderr,
         appContextClass_str17, fileName );
      }
      return;
    }

    fprintf( stderr, "\n" );

    strncpy( line, fileName, 255 );
    Strncat( line, "~", 255 );

    if ( fileExists ) {

      stat = unlink( line );

      fprintf( stderr, appContextClass_str18, line );
      stat = rename( fileName, line );
      if ( stat ) {
        perror( appContextClass_str19 );
        return;
      }

    }

    f = fopen( fileName, "w" );
    if ( f ) {

      fprintf( f, "%-d\n", numComponents );
      cur = head->flink;
      while ( cur ) {
        fprintf( f, "%s %s %s\n", cur->className, cur->fileName,
         cur->text );
        cur = cur->flink;
      }

    }
    else {
      perror( fileName );
      return;
    }

    if ( numToAdd == 1 )
      fprintf( stderr, appContextClass_str20 );
    else if ( numToAdd > 1 )
      fprintf( stderr, appContextClass_str21 );

  }
  else if ( strcmp( op, global_str64 ) == 0 ) {  // removepv

    if ( !fileExists ) {
      fprintf( stderr, appContextClass_str123 );
      return;
    }

    fprintf( stderr, "\n" );

    strcpy( line, "firstPvRegRecord" );
    func = (PVREGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str14, stderr );
      return;
    }

    stat = (*func)( &classNamePtr, &textPtr );

    strcpy( line, "nextPvRegRecord" );
    func = (PVREGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str9, stderr );
      return;
    }

    numToRemove = 0;
    while ( !stat ) {

      cur = head->flink;
      prev = head;
      while ( cur ) {
        next = cur->flink;
        if ( strcmp( classNamePtr, cur->className ) == 0 ) {
          fprintf( stderr, appContextClass_str124, classNamePtr );
          numToRemove++;
          prev->flink = next;
          delete cur;
          break;
	}
        else {
          prev = cur;
	}
        cur = next;
      }

      stat = (*func)( &classNamePtr, &textPtr );

    }

    if ( numToRemove == 0 ) {
      fprintf( stderr, appContextClass_str125, fileName );
      return;
    }

    // count remaining components
    numComponents = 0;
    cur = head->flink;
    while ( cur ) {
      numComponents++;
      cur = cur->flink;
    }

    fprintf( stderr, "\n" );

    strncpy( line, fileName, 255 );
    Strncat( line, "~", 255 );

    if ( fileExists ) {

      stat = unlink( line );

      fprintf( stderr, appContextClass_str18, line );
      stat = rename( fileName, line );
      if ( stat ) {
        perror( appContextClass_str19 );
        return;
      }

    }

    f = fopen( fileName, "w" );
    if ( f ) {

      fprintf( f, "%-d\n", numComponents );
      cur = head->flink;
      while ( cur ) {
        fprintf( f, "%s %s %s\n", cur->className, cur->fileName,
         cur->text );
        cur = cur->flink;
      }

    }
    else {
      perror( fileName );
      return;
    }

    if ( numToRemove == 1 )
      fprintf( stderr, appContextClass_str126 );
    else if ( numToRemove > 1 )
      fprintf( stderr, appContextClass_str127 );

    if ( numComponents == 0 ) {
      fprintf( stderr, appContextClass_str128 );
    }

  }

}

void ctlPvMonitorConnection (
  ProcessVariable *pv,
  void *userarg )
{

}

void ctlPvUpdate (
  ProcessVariable *pv,
  void *userarg )
{

appContextClass *apco = (appContextClass *) userarg;
char str[41], name[127+1];
activeWindowListPtr cur;
SYS_PROC_ID_TYPE procId;

  pv->get_string( name, 127 );
  name[127] = 0;

  if ( apco->initialConnection ) {
    apco->initialConnection = 0;
    pv->putText( "" );
    return;
  }

  if ( blank(name) ) {
    return;
  }

  if ( apco->shutdownFlag ) {
    return;
  }

  if ( strcmp( name, "* SHUTDOWN *" ) == 0 ) {
    apco->shutdownFlag = 1;
    sys_get_proc_id( &procId );
    sprintf( str, "%-d", (int) procId.id );
    pv->putText( str );
    return;
  }
  else if ( strcmp( name, "* RELOAD *" ) == 0 ) {
    apco->reloadFlag = 1;
    strcpy( str, "" );
    pv->putText( str );
    return;
  }
  else if ( strcmp( name, "* READONLY *" ) == 0 ) {
    setReadOnly();
    strcpy( str, "" );
    pv->putText( str );
    return;
  }

  if ( name[0] == ' ' ) {
    return;
  }

  cur = apco->head->flink;
  while ( cur != apco->head ) {
    if ( strcmp( name, cur->node.displayName ) == 0 ) {
      // deiconify
      XMapWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
      // raise
      XRaiseWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
      snprintf( str, 40, " %-lu", XtWindow(cur->node.topWidgetId()) );
      //strcpy( str, "" );
      pv->putText( str );
      return;  // display is already open; don't open another instance
    }
    cur = cur->flink;
  }

  cur = new activeWindowListType;
  //strcpy( cur->winName, "" );
  cur->requestDelete = 0;
  cur->requestActivate = 0;
  cur->requestActivateClear = 0;
  cur->requestReactivate = 0;
  cur->requestOpen = 0;
  cur->requestPosition = 0;
  cur->requestImport = 0;
  cur->requestRefresh = 0;
  cur->requestActiveRedraw = 0;
  cur->requestIconize = 0;
  cur->requestConvertAndExit = 0;

  cur->node.create( apco, NULL, 0, 0, 0, 0, apco->numMacros, apco->macros,
   apco->expansions );
  cur->node.realize();
  cur->node.setGraphicEnvironment( &apco->ci, &apco->fi );

  cur->blink = apco->head->blink;
  apco->head->blink->flink = cur;
  apco->head->blink = cur;
  cur->flink = apco->head;

  cur->node.storeFileName( name );

  cur->requestOpen = 1;
  (apco->requestFlag)++;

  cur->requestActivate = 1;
  (apco->requestFlag)++;

  strcpy( str, "" );
  pv->putText( str );

}

void setPath_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

callbackBlockPtr cbPtr = (callbackBlockPtr) client;
char *item = (char *) cbPtr->ptr;
appContextClass *apco = (appContextClass *) cbPtr->apco;

  //fprintf( stderr, "setPath_cb, item = [%s]\n", item );

  strncpy( apco->curPath, item, 127 );

}

void selectPath_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

callbackBlockPtr cbPtr = (callbackBlockPtr) client;
//char *item = (char *) cbPtr->ptr;
appContextClass *apco = (appContextClass *) cbPtr->apco;

  apco->pathList.popup();

}

void app_fileSelectFromPathOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

XmFileSelectionBoxCallbackStruct *cbs =
 (XmFileSelectionBoxCallbackStruct *) call;
appContextClass *apco = (appContextClass *) client;
activeWindowListPtr cur;
char *fName;

  if ( !XmStringGetLtoR( cbs->value, XmFONTLIST_DEFAULT_TAG, &fName ) ) {
    goto done;
  }

  if ( !*fName ) {
    XtFree( fName );
    goto done;
  }

  cur = new activeWindowListType;

  //strcpy( cur->winName, "" );
  cur->requestDelete = 0;
  cur->requestActivate = 0;
  cur->requestActivateClear = 0;
  cur->requestReactivate = 0;
  cur->requestOpen = 0;
  cur->requestPosition = 0;
  cur->requestImport = 0;
  cur->requestRefresh = 0;
  cur->requestActiveRedraw = 0;
  cur->requestIconize = 0;
  cur->requestConvertAndExit = 0;

  cur->node.create( apco, NULL, 0, 0, 0, 0, apco->numMacros, apco->macros,
   apco->expansions );
  cur->node.realize();
  cur->node.setGraphicEnvironment( &apco->ci, &apco->fi );

  cur->blink = apco->head->blink;
  apco->head->blink->flink = cur;
  apco->head->blink = cur;
  cur->flink = apco->head;

  cur->node.storeFileName( fName );

  XtFree( fName );

  cur->requestOpen = 1;
  (apco->requestFlag)++;

  if ( apco->executeOnOpen ) {
    cur->requestActivate = 1;
    (apco->requestFlag)++;
  }

done:

  XtUnmanageChild( w );

}

void app_fileSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

XmFileSelectionBoxCallbackStruct *cbs =
 (XmFileSelectionBoxCallbackStruct *) call;
appContextClass *apco = (appContextClass *) client;
activeWindowListPtr cur;
char *fName;

  if ( !XmStringGetLtoR( cbs->value, XmFONTLIST_DEFAULT_TAG, &fName ) ) {
    goto done;
  }

  if ( !*fName ) {
    XtFree( fName );
    goto done;
  }

  cur = new activeWindowListType;

  //strcpy( cur->winName, "" );
  cur->requestDelete = 0;
  cur->requestActivate = 0;
  cur->requestActivateClear = 0;
  cur->requestReactivate = 0;
  cur->requestOpen = 0;
  cur->requestPosition = 0;
  cur->requestImport = 0;
  cur->requestRefresh = 0;
  cur->requestActiveRedraw = 0;
  cur->requestIconize = 0;
  cur->requestConvertAndExit = 0;

  cur->node.create( apco, NULL, 0, 0, 0, 0, apco->numMacros, apco->macros,
   apco->expansions );
  cur->node.realize();
  cur->node.setGraphicEnvironment( &apco->ci, &apco->fi );

  cur->blink = apco->head->blink;
  apco->head->blink->flink = cur;
  apco->head->blink = cur;
  cur->flink = apco->head;

  cur->node.storeFileName( fName );

  XtFree( fName );

  cur->requestOpen = 1;
  (apco->requestFlag)++;

  if ( apco->executeOnOpen ) {
    cur->requestActivate = 1;
    (apco->requestFlag)++;
  }

done:

  XtUnmanageChild( w );

}

void app_fileSelectFromPathCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  XtUnmanageChild( w );

}

void app_fileSelectCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  XtUnmanageChild( w );

}

void app_importSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

XmFileSelectionBoxCallbackStruct *cbs =
 (XmFileSelectionBoxCallbackStruct *) call;
appContextClass *apco = (appContextClass *) client;
activeWindowListPtr cur;
char *fName;

  if ( !XmStringGetLtoR( cbs->value, XmFONTLIST_DEFAULT_TAG, &fName ) ) {
    goto done;
  }

  if ( !*fName ) {
    XtFree( fName );
    goto done;
  }

  cur = new activeWindowListType;

  //strcpy( cur->winName, "" );
  cur->requestDelete = 0;
  cur->requestActivate = 0;
  cur->requestActivateClear = 0;
  cur->requestReactivate = 0;
  cur->requestOpen = 0;
  cur->requestPosition = 0;
  cur->requestImport = 0;
  cur->requestRefresh = 0;
  cur->requestActiveRedraw = 0;
  cur->requestIconize = 0;
  cur->requestConvertAndExit = 0;

  cur->node.create( apco, NULL, 0, 0, 0, 0, apco->numMacros, apco->macros,
   apco->expansions );
  cur->node.realize();
  cur->node.setGraphicEnvironment( &apco->ci, &apco->fi );

  cur->blink = apco->head->blink;
  apco->head->blink->flink = cur;
  apco->head->blink = cur;
  cur->flink = apco->head;

  cur->node.storeFileName( fName );

  XtFree( fName );

  cur->requestOpen = 1;
  cur->requestImport = 1;
  (apco->requestFlag)++;

done:

  XtUnmanageChild( w );

}

void app_importSelectCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  XtUnmanageChild( w );

}

void new_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

static int oneFileNum = 1;

appContextClass *apco = (appContextClass *) client;
activeWindowListPtr cur, next;
char oneFileName[127+1];

  // traverse list and delete nodes so marked
  cur = apco->head->flink;
  while ( cur != apco->head ) {
    next = cur->flink;
    if ( cur->requestDelete ) {
      cur->blink->flink = cur->flink;
      cur->flink->blink = cur->blink;
      apco->removeAllDeferredExecutionQueueNode( &cur->node );
      if ( cur->node.numChildren == 0 ) {
        if ( cur->node.parent ) {
          if ( cur->node.parent->numChildren ) {
            (cur->node.parent->numChildren)--;
          }
        }
        delete cur;
      }
    }
    cur = next;
  }

  cur = new activeWindowListType;
  //strcpy( cur->winName, "" );
  cur->requestDelete = 0;
  cur->requestActivate = 0;
  cur->requestActivateClear = 0;
  cur->requestReactivate = 0;
  cur->requestOpen = 0;
  cur->requestPosition = 0;
  cur->requestImport = 0;
  cur->requestRefresh = 0;
  cur->requestActiveRedraw = 0;
  cur->requestIconize = 0;
  cur->requestConvertAndExit = 0;
  cur->node.create( apco, NULL, 100, 100, 500, 600, apco->numMacros,
   apco->macros, apco->expansions );
  cur->node.realize();
  cur->node.setGraphicEnvironment( &apco->ci, &apco->fi );
  cur->node.setDisplayScheme( &apco->displayScheme );

  sprintf( oneFileName, "%s%-d", appContextClass_str140, oneFileNum );

  cur->node.storeFileName( oneFileName );

  if ( oneFileNum > 0xfffffff )
    oneFileNum = 1;
  else
    oneFileNum++;

  //XtMapWidget( XtParent( cur->node.drawWidgetId() ) );
  XtMapWidget( cur->node.topWidgetId() );

  cur->blink = apco->head->blink;
  apco->head->blink->flink = cur;
  apco->head->blink = cur;
  cur->flink = apco->head;

  apco->atLeastOneOpen = 1;

}

void refreshUserLib_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;

  apco->reopenUserLib();

}

void open_from_path_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;
int n;
Arg args[10];
XmString xmStr = NULL;
char prefix[127+1];

  strncpy( prefix, apco->curPath, 127 );

  n = 0;

  if ( strcmp( prefix, "" ) != 0 ) {
    xmStr = XmStringCreateLocalized( prefix );
    XtSetArg( args[n], XmNdirectory, xmStr ); n++;
    XtSetValues( apco->fileSelectFromPathBox, args, n );
    XmStringFree( xmStr );
  }

  XtManageChild( apco->fileSelectFromPathBox );

  XSetWindowColormap( apco->display, XtWindow(XtParent(apco->fileSelectFromPathBox)),
   apco->ci.getColorMap() );

}

void open_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;
int n;
Arg args[10];
XmString xmStr = NULL;

  if ( apco->firstOpen ) {

    apco->firstOpen = 0;

    n = 0;
    xmStr = XmStringCreateLocalized( "./" );
    XtSetArg( args[n], XmNdirectory, xmStr ); n++;
    XtSetValues( apco->fileSelectBox, args, n );
    XmStringFree( xmStr );

  }

  XtManageChild( apco->fileSelectBox );

  XSetWindowColormap( apco->display, XtWindow(XtParent(apco->fileSelectBox)),
   apco->ci.getColorMap() );

}

void import_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;
int n;
Arg args[10];
XmString xmStr = NULL;
char prefix[127+1];

  strncpy( prefix, apco->curPath, 127 );

  n = 0;

  if ( strcmp( prefix, "" ) != 0 ) {
    xmStr = XmStringCreateLocalized( prefix );
    XtSetArg( args[n], XmNdirectory, xmStr ); n++;
    XtSetValues( apco->importSelectBox, args, n );
    XmStringFree( xmStr );
  }

  XtManageChild( apco->importSelectBox );

  XSetWindowColormap( apco->display, XtWindow(XtParent(apco->importSelectBox)),
   apco->ci.getColorMap() );

}

void continue_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;

  apco->confirm.popdown();

}

void abort_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;

  apco->confirm.popdown();

  apco->pathList.popdown();

  apco->exitFlag = 1;

  if ( diagnosticMode() ) logDiagnostic( "Program exit requested\n" );

}

void exit_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;
activeWindowListPtr cur;
int changes = 0;

Window root, child;
int rootX, rootY, winX, winY;
unsigned int mask;

  cur = apco->head->flink;
  while ( cur != apco->head ) {
    if ( cur->node.changed() ) changes = 1;
    cur = cur->flink;
  }

  if ( changes ) {
    XQueryPointer( apco->display, XtWindow(apco->appTop), &root, &child,
     &rootX, &rootY, &winX, &winY, &mask );
    apco->confirm.create( apco->appTop, "confirm", rootX, rootY, 2,
     appContextClass_str24, NULL, NULL );
    apco->confirm.addButton( appContextClass_str25, abort_cb, (void *) apco );
    apco->confirm.addButton( appContextClass_str26, continue_cb,
     (void *) apco );
    apco->confirm.finished();
    apco->confirm.popup();
    XSetWindowColormap( apco->display, XtWindow(apco->confirm.top()),
     apco->ci.getColorMap() );
    return;
  }

  apco->pathList.popdown();

  apco->exitFlag = 1;

  if ( diagnosticMode() ) logDiagnostic( "Program exit requested\n" );

}

void dont_shutdown_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;

  apco->confirm.popdown();

}

void do_shutdown_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;

  apco->confirm.popdown();
  apco->shutdownFlag = 1;

}

void shutdown_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;

activeWindowListPtr cur;
int changes = 0;

Window root, child;
int rootX, rootY, winX, winY;
unsigned int mask;

  cur = apco->head->flink;
  while ( cur != apco->head ) {
    if ( cur->node.changed() ) changes = 1;
    cur = cur->flink;
  }

  if ( changes ) {

    XQueryPointer( apco->display, XtWindow(apco->appTop), &root, &child,
     &rootX, &rootY, &winX, &winY, &mask );

    apco->confirm.create( apco->appTop, "confirm", rootX, rootY, 2,
     appContextClass_str24, NULL, NULL );
    apco->confirm.addButton( appContextClass_str137, do_shutdown_cb,
     (void *) apco );
    apco->confirm.addButton( appContextClass_str26, dont_shutdown_cb,
     (void *) apco );
    apco->confirm.finished();

    apco->confirm.popup();

    XSetWindowColormap( apco->display, XtWindow(apco->confirm.top()),
     apco->ci.getColorMap() );

    return;

  }

  apco->shutdownFlag = 1;

}

void reload_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;

  apco->reloadFlag = 1;

}

void save_screen_config (
  Widget w,
  XtPointer client,
  XtPointer call )
{
  Widget dialog;
  XmString t;
  Arg args[5];
  char filter[MAX_DIR];
  char msg[1001];
  void check_config_exists(Widget, XtPointer, XtPointer);
  void destroy_callback(Widget, XtPointer, XtPointer);
  
  appContextClass *apco = (appContextClass *) client;
  int err = apco->getCfgDirectory(filter, msg);
  if (err) {
    apco->postMessage(msg);
    XtDestroyWidget ( XtParent (w));
    return;
  }
  int n = 0;
  t = XmStringCreateLocalized (filter);
  XtSetArg (args[n], XmNdirMask, t); n++;
//  XtSetArg (args[0], XmNpathMode, XmPATH_MODE_RELATIVE);
  dialog = XmCreateFileSelectionDialog (w, "Select cfg:", args, 1);
  XmStringFree (t); // always destroy compound strings when done
  XtAddCallback (dialog, XmNokCallback, check_config_exists, (appContextClass *) client);
  XtAddCallback (dialog, XmNcancelCallback, destroy_callback, NULL);
  XtManageChild (dialog);
}

void destroy_callback(
  Widget w,
  XtPointer client,
  XtPointer call )
{
  XtDestroyWidget ( XtParent (w));
}

void check_config_exists(
  Widget w,         // the dialog widget
  XtPointer client,
  XtPointer call )

{
  Widget dialog;
  void exec_config_save(Widget, XtPointer, XtPointer);
  void destroy_callback(Widget, XtPointer, XtPointer);
  Arg args[5];
  char fname[MAX_FNAME];
  char msg[1001];
  char *xtext;
  
  XmSelectionBoxCallbackStruct *cbs = (XmSelectionBoxCallbackStruct *) call; 
  xtext = (char *) XmStringUnparse (cbs->value,
                                         XmFONTLIST_DEFAULT_TAG,
                                         XmCHARSET_TEXT,
                                         XmCHARSET_TEXT,
                                         NULL, 0, XmOUTPUT_ALL);
  appContextClass *apco = (appContextClass *) client;
  
  if (strlen(xtext) >= MAX_NAME) {
      sprintf(msg, appContextClass_str100, "name");  // string too long: %s
      apco->postMessage(msg);
      XtFree(xtext);
      XtDestroyWidget ( XtParent (w));
      return;
  }
  int ltext = strlen(xtext);
  if ( ltext  && ltext < MAX_FNAME) {
    strcpy(fname, xtext);
    XtFree(xtext);
  } else {
    if (ltext) {
      sprintf(msg, appContextClass_str100, "file name");  // String too long: %s
    } else {
      sprintf(msg, appContextClass_str101, "file name");  // String empty: %s
    }
    apco->postMessage(msg);
    return;
  }
  apco->checkCfgName(fname);
//  sprintf(msg, "checking file %s\n", fname);
//  printf(msg);
  FILE *fp = fopen(fname, "r");
  if (fp) {
    fclose(fp);
//    printf("file exists\n");
    sprintf(msg, appContextClass_str53, fname);          // File already exists:\n%s
    apco->setCfgName(fname);                     // remember fname for after confirm dialog
    XmString xms = XmStringCreateLocalized (msg);
    int n = 0;
    XtSetArg (args[n], XmNmessageString, xms); n++;
    dialog = XmCreateMessageDialog (w, "message", args, n);
    XmStringFree (xms);
    XtAddCallback (dialog, XmNokCallback, exec_config_save, (appContextClass *) client);
    XtAddCallback (dialog, XmNcancelCallback, destroy_callback, NULL);
    XtManageChild (dialog);
    return;
  } else {
    apco->setCfgName("");                     // indicate that we did not have confirm dialog
    XtDestroyWidget ( XtParent (w));
    int err = apco->writeConfig(fname); 
    if (err) {
      sprintf(msg, appContextClass_str78, fname);   // Error writing file: %s
      apco->postMessage(msg);
      return;
    }
  } 
}

void exec_config_save(
  Widget w,         // the dialog widget
  XtPointer client,
  XtPointer call )

{
  char *fname;
  char msg[1001];
  char *xtext;
  
  appContextClass *apco = (appContextClass *) client;
  if (*apco->getCfgName()) {          // we had a confirmation dialog
    XtDestroyWidget ( XtParent(XtParent(w)));
  }
  XtDestroyWidget ( XtParent (w));
  
  fname = apco->getCfgName();
  int err = apco->writeConfig(fname); 
  if (err) {
    sprintf(msg, appContextClass_str78, fname);   // Error writing file: %s
    apco->postMessage(msg);
    return;
  }
}

// This function does not touch apco->screenAdd nor apco->screenAddAll
void load_screen_config (
  Widget w,
  XtPointer client,
  XtPointer call )
{
  char filter[MAX_DIR];
  char line[MAX_LINE];
  char *home;
  Widget dialog;
  XmString t;
  void exec_config_load(Widget, XtPointer, XtPointer);
  void destroy_callback(Widget, XtPointer, XtPointer);
  Arg args[2];
  
  appContextClass *apco = (appContextClass *) client;
  int err = apco->getCfgDirectory(filter, line);
  if (err) {
    apco->postMessage(line);
    XtDestroyWidget ( XtParent (w));
    return;
  }
  
  int n = 0;
  t = XmStringCreateLocalized (filter);
  XtSetArg (args[n], XmNdirMask, t); n++;
//  XtSetArg (args[0], XmNpathMode, XmPATH_MODE_RELATIVE);
  dialog = XmCreateFileSelectionDialog (w, "Select cfg:", args, 1);
  XmStringFree (t); // always destroy compound strings when done
  XtAddCallback (dialog, XmNokCallback, exec_config_load, (appContextClass *) client);
  XtAddCallback (dialog, XmNcancelCallback, destroy_callback, NULL);
  XtManageChild (dialog);
}

// This function sets apco->screenAdd
void add_screen_config (
  Widget w,
  XtPointer client,
  XtPointer call )
{
  appContextClass *apco = (appContextClass *) client;
  apco->setScreenAdd(1);
  load_screen_config(w, client, call);
}

// This function sets apco->screenAddAll
void addall_screen_config (
  Widget w,
  XtPointer client,
  XtPointer call )
{
  appContextClass *apco = (appContextClass *) client;
  apco->setScreenAddAll(1);
  load_screen_config(w, client, call);
}

void switch_screen_config (
  Widget w,
  XtPointer client,
  XtPointer call )
{
  Widget dialog;
  Arg arg[5];
  XmString xms;
  void exec_config_load(Widget, XtPointer, XtPointer);
  void destroy_callback(Widget, XtPointer, XtPointer);
  int  n = 0;

  xms = XmStringCreateLocalized ("This will close all screeens.\nAre you sure?");
  XtSetArg (arg[n], XmNmessageString, xms); n++;
  dialog = XmCreateMessageDialog (w, "message", arg, n);
  XmStringFree (xms);
  XtAddCallback (dialog, XmNokCallback, load_screen_config, (appContextClass *) client);
  XtAddCallback (dialog, XmNcancelCallback, destroy_callback, NULL);
  XtManageChild (dialog);
}

// This function must reset appco->screenAdd and appco-screenAddAll
void exec_config_load (
  Widget w,
  XtPointer client,
  XtPointer call )
{
  char *newMacros[MAX_MACROS];
  char *newValues[MAX_MACROS];

  char *home;
  char fname[MAX_FNAME];
  char line[MAX_LINE];
  char edlname[MAX_NAME];
  char macros[MAX_LINE];
  int nmacros = 0;
  char **msymbols = 0;
  char **mvalues = 0;
  int x, y;
  float scale = 1.0;   // to be added to display node later
  char *xfname;

  XmFileSelectionBoxCallbackStruct *cbs =
                        (XmFileSelectionBoxCallbackStruct *) call; 
  xfname = (char *) XmStringUnparse (cbs->value,
                                         XmFONTLIST_DEFAULT_TAG,
                                         XmCHARSET_TEXT,
                                         XmCHARSET_TEXT,
                                         NULL, 0, XmOUTPUT_ALL); 
//  printf("calling exec_config_load - name %s\n", xfname);
  appContextClass *apco = (appContextClass *) client;

  // load the screen configuration file from the user's home directory
  if (strlen(xfname) >= MAX_NAME) {
    sprintf(line, appContextClass_str100, "name");  // string too long: %s
    apco->postMessage(line);
    XtDestroyWidget ( XtParent (w));
    apco->setScreenAdd(0);
    apco->setScreenAddAll(0);
    return;
  }
  strcpy(fname, xfname);
  XtFree(xfname);
 if (diagnosticMode()) {
    sprintf(line, "reading file %s\n", fname);
    logDiagnostic(line);
  }
  FILE *fp = fopen(fname, "r");
  if (!fp) {
    sprintf(line, appContextClass_str108, fname);   // Error loading file: %s
    apco->postMessage(line);
    XtDestroyWidget ( XtParent (w));
    apco->setScreenAdd(0);
    apco->setScreenAddAll(0);
    return;
  } 
  XtDestroyWidget ( XtParent (w));
  
  if (!apco->screenConfigOk(fp)) {
    sprintf(line, appContextClass_str51, fname);   // Invalid screen configuration: %s
    apco->postMessage(line);
    XtDestroyWidget ( XtParent (w));
    apco->setScreenAdd(0);
    apco->setScreenAddAll(0);
    return;
  }
  int i = 0;
  activeWindowListPtr cur;
  unordered_map<string, string> sigs;
  // if we switch configurations, remove existing files except the head
  // (in order not to get caught by eolc
  if (!apco->getScreenAdd() && !apco->getScreenAddAll()) apco->closeAllButHead();
  else apco->getScreenSignatures(sigs);
  
/*  printf("Existing screens\n");
  for(auto iter=sigs.begin(); iter!=sigs.end(); ++iter) {
    printf("%s --> %s\n", iter->first.c_str(), iter->second.c_str());
  }
*/
  while (fgets(line, MAX_LINE, fp) != NULL) { 
    if (*line == '#') continue;
    int len = strlen(line);
    *(line+len) = '\0';
    //int n = sscanf(line, "%s x=%d y=%d scale=%f %s", edlname, &x, &y, &scale, macros );
    int n = sscanf(line, "%s x=%d y=%d scale=%f %[^\n]", edlname, &x, &y, &scale, macros );
    if (n < 4) continue;      // not enough info in file
    parseSymbolsAndValues( macros, 100, newMacros, newValues, &nmacros );
    if (!apco->getScreenAdd()) {
      apco->addActWin(edlname, x, y, nmacros, newMacros, newValues );
      cur = apco->head->blink;
      cur->x = x;
      cur->y = y;
      cur->requestPosition = 1;
    } else {
      if (apco->getScreenAddAll() || !apco->screenAlreadyUp(sigs, edlname, newMacros, newValues, nmacros)) {
        apco->addActWin(edlname, x, y, nmacros, newMacros, newValues );
        cur = apco->head->blink;
        cur->x = x;
        cur->y = y;
        cur->requestPosition = 1;
      }
    }   
  }
  fclose(fp);
  apco->setScreenAdd(0);
  apco->setScreenAddAll(0);
}



void view_pvList_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;

  apco->pvList.popup();

}

void renderImages_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;
XmString str;

  if ( apco->renderImagesFlag ) {
    apco->renderImagesFlag = 0;
    str = XmStringCreateLocalized( appContextClass_str134 );
    XtVaSetValues( apco->renderImagesB,
     XmNlabelString, str,
     NULL );
    XmStringFree( str );
  }
  else {
    apco->renderImagesFlag = 1;
    str = XmStringCreateLocalized( appContextClass_str135 );
    XtVaSetValues( apco->renderImagesB,
     XmNlabelString, str,
     NULL );
    XmStringFree( str );
  }

}

void view_xy_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;
XmString str;

  if ( apco->viewXy ) {
    apco->viewXy = 0;
    str = XmStringCreateLocalized( appContextClass_str112 );
    XtVaSetValues( apco->viewXyB,
     XmNlabelString, str,
     NULL );
    XmStringFree( str );
  }
  else {
    apco->viewXy = 1;
    str = XmStringCreateLocalized( appContextClass_str113 );
    XtVaSetValues( apco->viewXyB,
     XmNlabelString, str,
     NULL );
    XmStringFree( str );
  }

}

void view_screens_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;
char string[79+1];
int i, nodeCount = 0;
activeWindowListPtr cur;

  cur = apco->head->flink;
  while ( cur != apco->head ) {
    if ( blank( cur->node.displayName ) ) {
      apco->postMessage( appContextClass_str27 );
    }
    else {
      apco->postMessage( cur->node.displayName );
      for ( i=0; i<cur->node.numMacros; i++ ) {
        snprintf( string, 79, "  %s=%s", cur->node.macros[i],
         cur->node.expansions[i] );
        apco->postMessage( string );
      }
    }
    nodeCount++;
    cur = cur->flink;
  }

  if ( nodeCount ) {

    sprintf( string, appContextClass_str28, nodeCount );
    for( i=0; string[i]; i++ ) string[i] = '-';
    apco->postMessage( string );

    sprintf( string, appContextClass_str28, nodeCount );
    apco->postMessage( string );

    strcpy( string, " " );
    apco->postMessage( string );

  }
  else {

    strcpy( string, appContextClass_str29 );
    apco->postMessage( string );

    strcpy( string, " " );
    apco->postMessage( string );

  }

}

void view_msgBox_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;

  apco->msgBox.popup();

  XSetWindowColormap( apco->display, XtWindow(apco->msgBox.top()),
   apco->ci.getColorMap() );

}

void checkpointPid_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;
char procIdName[31+1];
SYS_PROC_ID_TYPE procId;

  sys_get_proc_id( &procId );
  sprintf( procIdName, "PID = %-d", (int) procId.id );

  apco->postMessage( procIdName );

}

void viewFontMapping_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;
char tag[63+1], spec[255+1], buf[419+1];
int found;

  found = apco->fi.getFirstFontMapping( tag, 63, spec, 255 );
  while ( found ) {

    snprintf( buf, 419, "%s = %s", tag, spec );
    apco->postMessage( buf );

    found = apco->fi.getNextFontMapping( tag, 63, spec, 255 );

  }

}

void viewEnv_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;

  apco->showEnv();

}

void help_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;
activeWindowListPtr cur;
char buf[255+1], *envPtr;
int i, numMacros;
char *sysValues[5], *ptr;

char *fName = "helpMain";

char *sysMacros[] = {
  "help"
};

  // is help file already open?
  cur = apco->head->flink;
  while ( cur != apco->head ) {
    if ( strcmp( fName, cur->node.displayName ) == 0 ) {
      // deiconify
      XMapWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
      // raise
      XRaiseWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
      return;  // display is already open; don't open another instance
    }
    cur = cur->flink;
  }

  envPtr = getenv( environment_str5 );
  if ( envPtr ) {

    strncpy( buf, envPtr, 255 );

    if ( buf[strlen(buf)-1] != '/' ) {
      Strncat( buf, "/", 255 );
    }

  }
  else {

    strcpy( buf, "/etc/edm/" );

  }

  // build system macros

  numMacros = 0;

  ptr = new char[strlen(buf)+1];
  strcpy( ptr, buf );
  sysValues[0] = ptr;

  numMacros++;

  // ============

  Strncat( buf, fName, 255 );
  //Strncat( buf, ".edl", 255 );
  Strncat( buf, activeWindowClass::defExt(), 255 );

  cur = new activeWindowListType;
  //strcpy( cur->winName, "" );
  cur->requestDelete = 0;
  cur->requestActivate = 0;
  cur->requestActivateClear = 0;
  cur->requestReactivate = 0;
  cur->requestOpen = 0;
  cur->requestPosition = 0;
  cur->requestImport = 0;
  cur->requestRefresh = 0;
  cur->requestActiveRedraw = 0;
  cur->requestIconize = 0;
  cur->requestConvertAndExit = 0;

  cur->node.createNoEdit( apco, NULL, 0, 0, 0, 0, numMacros,
   sysMacros, sysValues );

  for ( i=0; i<numMacros; i++ ) {
    delete sysValues[i];
  }

  cur->node.realize();
  cur->node.setGraphicEnvironment( &apco->ci, &apco->fi );

  cur->blink = apco->head->blink;
  apco->head->blink->flink = cur;
  apco->head->blink = cur;
  cur->flink = apco->head;

  cur->node.storeFileName( buf );

  cur->requestOpen = 1;
  (apco->requestFlag)++;

  cur->requestActivate = 1;
  (apco->requestFlag)++;

}

/* ugly -- but why does EDM not use/support toolkit resources !? */
typedef struct AppContextResourceRec_ {
	Position	efX,efY;
	int			efW,efH;
	int			llH;		
} AppContextResourceRec;

static XtResource resource_list[] = {
	{ "entryFormX", "EntryFormX", XtRPosition, sizeof(Position),
	  XtOffsetOf( AppContextResourceRec, efX ), XtRImmediate, (XtPointer) 0 },
	{ "entryFormY", "EntryFormY", XtRPosition, sizeof(Position),
	  XtOffsetOf( AppContextResourceRec, efX ), XtRImmediate, (XtPointer) 0 },
	{ "entryFormW", "EntryFormW", XtRInt,       sizeof(int),
	  XtOffsetOf( AppContextResourceRec, efW ), XtRImmediate, (XtPointer) 0 },
	{ "entryFormH", "EntryFormH", XtRInt,       sizeof(int),
	  XtOffsetOf( AppContextResourceRec, efH ), XtRImmediate, (XtPointer) 0 },
	{ "largestH",   "LargestH",   XtRInt,       sizeof(int),
	  XtOffsetOf( AppContextResourceRec, llH ), XtRImmediate, (XtPointer) 0 },
};

appContextClass::appContextClass (
  void )
{

  firstOpen = 1;
  executeCount = 0;
  isActive = 0;
  exitFlag = 0;
  objDelFlag = 0;
  shutdownFlag = 0;
  reloadFlag = 0;
  saveContextOnExit = 0;
  primaryServer = 0;
  oneInstance = 0;
  executeOnOpen = 0;
  noEdit = 0;
  requestFlag = 0;
  iconified = 0;
  usingControlPV = 0;
  ctlPvId = NULL;
  renderImagesFlag = 1;
  exitOnLastClose = 0;
  atLeastOneOpen = 0;
  useScrollBars = 1;

  screenAdd = 0;
  screenAddAll = 0;
  confOk = "# edm screen configuration";
  confOkCount = 26;
  entryFormX = 0;
  entryFormY = 0;
  entryFormW = 0;
  //entryFormH = 600;
  entryFormH = 0;
  largestH = 600;

  head = new activeWindowListType;
  //strcpy( head->winName, "" );
  head->flink = head;
  head->blink = head;

  numMacros = 0;
  macroHead = new macroListType;
  macroHead->flink = macroHead;
  macroHead->blink = macroHead;

  numFiles = 0;
  fileHead = new fileListType;
  fileHead->flink = fileHead;
  fileHead->blink = fileHead;

  strcpy( ctlPV, "" );
  strcpy( userLib, "" );

  cutHead1 = new activeGraphicListType;
  cutHead1->flink = cutHead1;
  cutHead1->blink = cutHead1;

  viewXy = 0;

  haveGroupVisInfo = 0;

  int defaultPos = getFilePaths();
  strncpy( curPath, dataFilePrefix[defaultPos], 127 );

  buildSchemeList();

  // sentinel node
  callbackBlockHead = new callbackBlockType;
  callbackBlockTail = callbackBlockHead;
  callbackBlockTail->flink = NULL;

  thread_create_lock_handle( &actionsLock );

  thread_lock( actionsLock );

  // sentinel node
  actHead = new actionsType;
  actTail = actHead;
  actTail->flink = NULL;

  thread_unlock( actionsLock );

  syncOnce = 1;

  ddgc = NULL;
  ddFixedFont = NULL;

  useStdErrFlag = 0;
  errMsgPrefix = NULL;

  msgDialogOpenCount = 0;

}

appContextClass::~appContextClass (
  void )
{

int i;
activeGraphicListPtr curCut, nextCut;
macroListPtr curMacro, nextMacro;
fileListPtr curFile, nextFile;
callbackBlockPtr curCbBlock, nextCbBlock;
actionsPtr curAct, nextAct;

  if ( usingControlPV ) {
    if ( ctlPvId ) {
      ctlPvId->remove_conn_state_callback( ctlPvMonitorConnection, this );
      ctlPvId->remove_value_callback( ctlPvUpdate, this );
      ctlPvId->release();
      ctlPvId = NULL;
      usingControlPV = 0;
    }
  }

  // empty cut list
  curCut = cutHead1->flink;
  while ( curCut != cutHead1 ) {
    nextCut = curCut->flink;
    delete curCut->node;
    delete curCut;
    curCut = nextCut;
  }
  delete cutHead1;

  // walk macroList and delete
  curMacro = macroHead->flink;
  while ( curMacro != macroHead ) {
    nextMacro = curMacro->flink;
    if ( curMacro->macro ) delete[] curMacro->macro;
    if ( curMacro->expansion ) delete[] curMacro->expansion;
    delete curMacro;
    curMacro = nextMacro;
  }
  delete macroHead;

  // walk fileList and delete
  curFile = fileHead->flink;
  while ( curFile != fileHead ) {
    nextFile = curFile->flink;
    if ( curFile->file ) delete[] curFile->file;
    delete curFile;
    curFile = nextFile;
  }
  delete fileHead;

  // delete active window list head
  delete head;

  // delete widgets

  XtRemoveCallback( importSelectBox, XmNcancelCallback,
   app_importSelectOk_cb, (void *) this );
  XtRemoveCallback( importSelectBox, XmNcancelCallback,
   app_importSelectCancel_cb, (void *) this );
  XtUnmanageChild( importSelectBox );
  XtDestroyWidget( importSelectBox );

  XtRemoveCallback( fileSelectFromPathBox, XmNcancelCallback,
   app_fileSelectFromPathOk_cb, (void *) this );
  XtRemoveCallback( fileSelectFromPathBox, XmNcancelCallback,
   app_fileSelectFromPathCancel_cb, (void *) this );
  XtUnmanageChild( fileSelectFromPathBox );
  XtDestroyWidget( fileSelectFromPathBox );

  XtRemoveCallback( fileSelectBox, XmNcancelCallback,
   app_fileSelectOk_cb, (void *) this );
  XtRemoveCallback( fileSelectBox, XmNcancelCallback,
   app_fileSelectCancel_cb, (void *) this );
  XtUnmanageChild( fileSelectBox );
  XtDestroyWidget( fileSelectBox );

  pvList.destroy();

  msgBox.destroy();

  XtDestroyWidget( mainWin );

  processAllEvents( app, display );

  if ( dataFilePrefix ) {
    for ( i=0; i<numPaths; i++ ) {
      delete[] dataFilePrefix[i];
      dataFilePrefix[i] = NULL;
    }
    delete[] dataFilePrefix;
    dataFilePrefix = NULL;
  }

  destroySchemeList();

  curCbBlock = callbackBlockHead->flink;
  while ( curCbBlock ) {
    nextCbBlock = curCbBlock->flink;
    delete curCbBlock;
    curCbBlock = nextCbBlock;
  }
  delete callbackBlockHead;

  // delete actions list

  thread_lock( actionsLock );
  curAct = actHead->flink;
  while ( curAct ) {
    nextAct = curAct->flink;
    delete curAct;
    curAct = nextAct;
  }
  delete actHead;
  thread_unlock( actionsLock );
  thread_destroy_lock_handle( actionsLock );

  termDeferredExecutionQueue();

  if ( ddgc ) {
    XFreeGC( display, ddgc );
    ddgc = NULL;
  }

  if ( ddFixedFont ) {
    XFreeFont( display, ddFixedFont );
    ddFixedFont = NULL;
  }

}

void appContextClass::closeDownAppCtx ( void ) {

int numOpenWindows, count, moreToDelete;
activeWindowListPtr cur, next;
macroListPtr curMacro, nextMacro;

  if ( saveContextOnExit ) {
    fprintf( shutdownFilePtr, "%-d\n", primaryServer | ( oneInstance << 8 ) );
    writeStringToFile( shutdownFilePtr, displayName );
    fprintf( shutdownFilePtr, "%-d\n", noEdit );
  }

  ci.closeColorWindow();

  if ( saveContextOnExit ) {
    fprintf( shutdownFilePtr, "%-d\n", numMacros );
  }
  curMacro = macroHead->flink;
  while ( curMacro != macroHead ) {
    nextMacro = curMacro->flink;
    if ( saveContextOnExit ) {
      fprintf( shutdownFilePtr, "%s=%s\n", curMacro->macro,
       curMacro->expansion );
    }
    curMacro = nextMacro;
  }

  if ( saveContextOnExit ) {
    // get number of open windows
    numOpenWindows = 0;
    cur = head->flink;
    while ( cur != head ) {
      numOpenWindows++;
      cur = cur->flink;
    }
    fprintf( shutdownFilePtr, "%-d\n", numOpenWindows );
  }

  count = 100000;
  moreToDelete = 0;
  do {

    // walk activeWindowList - close and delete
    cur = head->flink;
    while ( cur != head ) {

      next = cur->flink;

      if ( cur->node.mode == AWC_EXECUTE ) {
        cur->node.returnToEdit( 0 );
      }

      if ( cur->node.numChildren ) {

        moreToDelete = 1;

      }
      else {

        if ( !cur->node.parent && saveContextOnExit ) {
          cur->node.checkPoint( primaryServer, shutdownFilePtr );
        }

        if ( cur->node.parent ) {
          if ( cur->node.parent->numChildren ) {
            (cur->node.parent->numChildren)--;
          }
        }

        cur->blink->flink = cur->flink; // maintain list structure!
        cur->flink->blink = cur->blink;

        delete cur;

      }

      cur = next;

    }

    processAllEvents( app, display );

    count--;

  } while ( moreToDelete && count );

  XtUnmapWidget( appTop );

  processAllEvents( app, display );

}

void appContextClass::reloadAll ( void )
{

activeWindowListPtr cur;

  // walk activeWindowList and reload
  cur = head->flink;
  while ( cur != head ) {
    if ( !cur->requestDelete ) {
      cur->requestActivate = 0;
      cur->requestActivateClear = 0;
      cur->requestReactivate = 0;
      cur->requestOpen = 1;
      requestFlag++;
      cur->requestPosition = 1;
      cur->requestImport = 0;
      cur->requestRefresh = 0;
      cur->requestActiveRedraw = 0;
      cur->requestIconize = 0;
      cur->requestConvertAndExit = 0;
      cur->x = cur->node.x;
      cur->y = cur->node.y;
      if ( cur->node.mode == AWC_EXECUTE ) {
        cur->node.returnToEdit( 0 );
        cur->node.noRaise = 1;
        processAllEvents( app, display );
        cur->requestActivate = 1;
        cur->requestActivateClear = 1;
        requestFlag++;
      }
      cur->node.reloadSelf();
    }
    cur = cur->flink;
  }
  processAllEvents( app, display );

}

void appContextClass::requestSelectedReload ( void )
{

  reloadFlag = 3;

}

void appContextClass::reloadSelected ( void )
{

activeWindowListPtr cur;

  // walk activeWindowList and reload
  cur = head->flink;
  while ( cur != head ) {
    if ( !cur->requestDelete && cur->node.reloadRequestFlag ) {
      cur->requestActivate = 0;
      cur->requestActivateClear = 0;
      cur->requestReactivate = 0;
      cur->requestOpen = 1;
      requestFlag++;
      cur->requestPosition = 1;
      cur->requestImport = 0;
      cur->requestRefresh = 0;
      cur->requestActiveRedraw = 0;
      cur->requestIconize = 0;
      cur->requestConvertAndExit = 0;
      cur->x = cur->node.x;
      cur->y = cur->node.y;
      if ( cur->node.mode == AWC_EXECUTE ) {
        cur->node.returnToEdit( 0 );
        cur->node.noRaise = 1;
        processAllEvents( app, display );
        cur->requestActivate = 1;
        cur->requestActivateClear = 1;
        requestFlag++;
      }
      cur->node.reloadRequestFlag = 0;
      cur->node.reloadSelf();
    }
    cur = cur->flink;
  }
  processAllEvents( app, display );

}

void appContextClass::refreshAll ( void )
{

activeWindowListPtr cur;

  // walk activeWindowList and request refresh
  cur = head->flink;
  while ( cur != head ) {
    if ( !cur->requestDelete ) {
      if ( cur->node.mode == AWC_EXECUTE ) {
        cur->node.refreshActive();
      }
      else {
        cur->node.refresh();
      }
    }
    cur = cur->flink;
    processAllEvents( app, display );
  }

}

int appContextClass::getFilePaths ( void ) {

  int i, l, allocL, curLen, stat, defaultPos = 0;
char *envPtr, *gotIt, *buf, save[127+1], path[127+1], msg[127+1], *tk,
 *useHttp;

  curLen = -1;
  buf = NULL;

  useHttp = getenv( environment_str9 ); // EDMHTTPDOCROOT

  // EDMFILES
  envPtr = getenv( environment_str2 );
  if ( envPtr ) {

    l = strlen( envPtr ) + 1;
    allocL = l / 4096;
    allocL = allocL * 4096 + 4096;
    curLen = allocL;
    buf = new char[allocL];

    strncpy( buf, envPtr, l );
    buf[l] = 0;

    tk = strtok( buf, ":" );
    if ( tk ) {

      strncpy( colorPath, tk, 127 );
      if ( colorPath[strlen(colorPath)-1] != '/' ) {
        Strncat( colorPath, "/", 127 );
      }

    }
    else {

      strncpy( colorPath, "./", 127 );

    }

  }
  else {

    strncpy( colorPath, "./", 127 );

  }

  // EDMDATAFILES

  envPtr = getEnvironmentVar( environment_str1 );
  if ( envPtr ) {

    l = strlen( envPtr ) + 1;
    allocL = l / 4096;
    allocL = allocL * 4096 + 4096;

    if ( !buf ) {
      curLen = allocL;
      buf = new char[allocL];
    }
    else {
      if ( allocL > curLen ) {
        curLen = allocL;
        delete[] buf;
        buf = new char[allocL];
      }
    }

    // count number of search paths
    strncpy( buf, envPtr, l );
    buf[l] = 0;

    fixupHttpPart( buf );

    numPaths = 0;
    tk = strtok( buf, ":" );
    while ( tk ) {

      numPaths++;

      tk = strtok( NULL, ":" );

    }

    if ( numPaths == 0 ) {

      strcpy( path, "." );

      gotIt = getcwd( save, 127 );
      if ( !gotIt ) {
        fprintf( stderr, appContextClass_str118, __LINE__, __FILE__ );
        exit(0);
      }

      stat = chdir( path );
      if ( stat && !useHttp ) {
        snprintf( msg, 127, appContextClass_str119, path );
        perror( msg );
      }

      chdir( save );

      if ( path[strlen(path)-1] != '/' )
       Strncat( path, "/", 127 );

      numPaths = 1;
      dataFilePrefix = new char *[1];
      dataFilePrefix[0] = new char[strlen(path)+1];
      strcpy( dataFilePrefix[0], path );

      if ( buf ) delete[] buf;

      return defaultPos;

    }

    dataFilePrefix = new char *[numPaths];

    strncpy( buf, envPtr, l );
    buf[l] = 0;

    fixupHttpPart( buf );

    tk = strtok( buf, ":" );
    for ( i=0; i<numPaths; i++ ) {

      strncpy( path, tk, 127 );
      if ( path[strlen(path)-1] == '/' ) path[strlen(path)-1] = 0;

      undoFixupHttpPart( path );

      gotIt = getcwd( save, 127 );
      if ( !gotIt ) {
        fprintf( stderr, appContextClass_str118, __LINE__, __FILE__ );
        exit(0);
      }

      if ( !httpPath( path ) ) {

        if ( path[0] == '=' ) {
          stat = chdir( &path[1] );
          if ( stat && !useHttp ) {
            snprintf( msg, 127, appContextClass_str119, &path[1] );
            perror( msg );
          }
        }
        else {
          stat = chdir( path );
          if ( stat && !useHttp ) {
            snprintf( msg, 127, appContextClass_str119, path );
            perror( msg );
          }
        }

        chdir( save );

      }

      if ( path[strlen(path)-1] != '/' )
       Strncat( path, "/", 127 );

      if ( path[0] == '=' ) { // = means use this a default
        dataFilePrefix[i] = new char[strlen(path)];
        strcpy( dataFilePrefix[i], &path[1] );
        defaultPos = i;
      }
      else {
        dataFilePrefix[i] = new char[strlen(path)+1];
        strcpy( dataFilePrefix[i], path );
      }

      tk = strtok( NULL, ":" );

    }

  }
  else {

    getcwd( path, 127 );

    if ( path[strlen(path)-1] != '/' )
     Strncat( path, "/", 127 );

    numPaths = 1;

    dataFilePrefix = new char *[1];
    dataFilePrefix[0] = new char[strlen(path)+1];
    strcpy( dataFilePrefix[0], path );

  }

  if ( buf ) delete[] buf;

  if ( defaultPos > ( numPaths - 1 ) ) defaultPos = numPaths - 1;
  if ( defaultPos < 0 ) defaultPos = 0;

  return defaultPos;

}

static int containsHttp (
  char *fullName
) {

unsigned int i;
char buf[255+1], *tk, *context;

  strncpy( buf, fullName, 255 );
  buf[255] = 0;

  context = NULL;
  tk = strtok_r( buf, ":", &context );

  if ( !tk ) return 0;

  for ( i=0; i<strlen(tk); i++ ) {
    tk[i] = tolower( tk[i] );
  }

  if ( ( strcmp( tk, "http" ) == 0 ) ||
       ( strcmp( tk, "https" ) == 0 )
     ) {

    return 1;

  }

  return 0;

}

void appContextClass::expandFileName (
  int index,
  char *expandedName,
  char *inName,
  char *ext,
  int maxSize )
{

unsigned int i, l, first, more;
int state, noPrefix;

  if ( index >= numPaths ) {
    strcpy( expandedName, "" );
    return;
  }

  if ( containsHttp(inName) ) {

    noPrefix = 1;

  }
  else {

    // ^/ means use current directory, don't use search path

    noPrefix = 0;
    state = 1;
    for ( i=0; ( i<strlen(inName) ) && state; i++ ) {

      switch ( state ) {

      case 1: // looking for / or ^
        if ( inName[i] == '/' ) {
          noPrefix = 1;
          state = 0; // exit
        }
        else if ( inName[i] == '^' ) {
          state = 2;
        }
        else if ( inName[i] != ' ' ) {
          state = 0; // exit
        }
        break;

      case 2: // looking for / part of ^/
        if ( inName[i] == '/' ) {
          noPrefix = 1;
          inName[i-1] = '.';
        }
        state = 0; // exit
        break;

      }

    }

  }

  if ( noPrefix ) {
    strncpy( expandedName, inName, maxSize );
  }
  else {
    strncpy( expandedName, dataFilePrefix[index], maxSize );
    Strncat( expandedName, inName, maxSize );
  }


  // if no extension specified, add ext from argument

  l = strlen(expandedName);
  more = 1;
  first = 0;
  for ( i=l-1; (i>=0) && more; i-- ) {
    if ( expandedName[i] == '/' ) {
      more = 0;
      first = i;
    }
  }
  if ( !strstr( &expandedName[first], "." ) ) {
    Strncat( expandedName, ext, maxSize );
  }

  return;

}

void appContextClass::expandFileName (
  int index,
  char *expandedName,
  char *inName,
  int maxSize )
{

unsigned int i;
int state, noPrefix;

  if ( index >= numPaths ) {
    strcpy( expandedName, "" );
    return;
  }

  if ( containsHttp(inName) ) {

    noPrefix = 1;

  }
  else {

    // ^/ means use current directory, don't use search path

    noPrefix = 0;
    state = 1;
    for ( i=0; ( i<strlen(inName) ) && state; i++ ) {

      switch ( state ) {

      case 1: // looking for / or ^
        if ( inName[i] == '/' ) {
          noPrefix = 1;
          state = 0; // exit
        }
        else if ( inName[i] == '^' ) {
          state = 2;
        }
        else if ( inName[i] != ' ' ) {
          state = 0; // exit
        }
        break;

      case 2: // looking for / part of ^/
        if ( inName[i] == '/' ) {
          noPrefix = 1;
          inName[i-1] = '.';
        }
        state = 0; // exit
        break;

      }

    }

  }

  if ( noPrefix ) {
    strncpy( expandedName, inName, maxSize );
  }
  else {
    strncpy( expandedName, dataFilePrefix[index], maxSize );
    Strncat( expandedName, inName, maxSize );
  }

}

#define GETTING_SET_NAME 1
#define GETTING_LIST 2
void appContextClass::buildSchemeList ( void )
{

char *envPtr, *ptr;
char prefix[127+1], fName[127+1], buf[255+1], oName[127+1], sName[63+1],
 line[255+1], *tk;
schemeListPtr cur, curSet;
FILE *f;
int stat, dup, state, i;

  numSchemeSets = 0;
  schemeListExists = 0;
  schemeList = (AVL_HANDLE) NULL;
  schemeSet = (AVL_HANDLE) NULL;
  schemeSetList = (char **) NULL;

  //fprintf( stderr, "build scheme list\n" );
  stat = avl_init_tree( compare_nodes, compare_key, copy_nodes,
   &(this->schemeList) );
  if ( !( stat & 1 ) ) {
    numSchemeSets = 0;
    schemeListExists = 0;
    return;
  }

  stat = avl_init_tree( compare_nodes, compare_key, copy_nodes,
   &(this->schemeSet) );
  if ( !( stat & 1 ) ) {
    numSchemeSets = 0;
    schemeListExists = 0;
    return;
  }

  // open scheme list file and build tree
  envPtr = getenv(environment_str2);
  if ( envPtr ) {
    strncpy( prefix, envPtr, 127 );
    if ( prefix[strlen(prefix)-1] != '/' ) Strncat( prefix, "/", 127 );
  }
  else {
    strcpy( prefix, "/etc/edm/" );
  }

  strncpy( fName, prefix, 127 );
  Strncat( fName, "schemes.list", 127 );

  f = fopen( fName, "r" );
  if ( !f ) {
    numSchemeSets = 0;
    schemeListExists = 0;
    return;
  }

  state = GETTING_SET_NAME;

  while ( 1 ) {

    switch ( state ) {

    case GETTING_SET_NAME:

      //fprintf( stderr, "GETTING_SET_NAME\n" );

      do {

        ptr = fgets ( line, 255, f );
        if ( !ptr ) {
          fclose( f );
          schemeListExists = 1;
          goto done;
        }

      } while ( blank( line ) );

      tk = strtok( line, " \t\n" );
      if ( tk ) {

        strncpy( sName, tk, 63 );

        tk = strtok( NULL, " \t\n" );
        if ( tk ) {

          if ( strcmp( tk, "{" ) != 0 ) {
            fprintf( stderr, "appContextClass::buildSchemeList syntax err 1\n" );
            fclose( f );
            numSchemeSets = 0;
            schemeListExists = 0;
            return;
          }

	}
	else {
          fprintf( stderr, "appContextClass::buildSchemeList syntax err 2\n" );
          fclose( f );
          numSchemeSets = 0;
          schemeListExists = 0;
          return;
        }

      }
      else {
        fprintf( stderr, "appContextClass::buildSchemeList syntax err 3\n" );
        fclose( f );
        numSchemeSets = 0;
        schemeListExists = 0;
        return;
      }

      curSet = new schemeListType;
      curSet->objName = new char[strlen(sName)+1];
      strcpy( curSet->objName, sName );
      stat = avl_insert_node( this->schemeSet, (void *) curSet, &dup );
      if ( !( stat & 1 ) ) {
        fclose( f );
        numSchemeSets = 0;
        schemeListExists = 0;
        return;
      }
      if ( dup ) {
        fprintf( stderr, "appContextClass::buildSchemeList dups\n" );
      }
      else {
        numSchemeSets++;
      }

      state = GETTING_LIST;

      break;

    case GETTING_LIST:

      //fprintf( stderr, "GETTING_LIST\n" );

      do {

        ptr = fgets ( line, 255, f );
        if ( !ptr ) {
          fprintf( stderr, "appContextClass::buildSchemeList syntax err 4\n" );
          fclose( f );
          numSchemeSets = 0;
          schemeListExists = 0;
          return;
        }

      } while ( blank( line ) );

      tk = strtok( line, " \t\n" );
      if ( tk ) {

        if ( strcmp( tk, "}" ) == 0 ) {
          state = GETTING_SET_NAME;
          break;
	}

        strncpy( oName, tk, 127 );

        tk = strtok( NULL, " \t\n" );
        if ( tk ) {

          strncpy( fName, tk, 127 );

          cur = new schemeListType;
          if ( !cur ) {
            fclose( f );
            numSchemeSets = 0;
            schemeListExists = 0;
            return;
          }

          strncpy( buf, sName, 255 );
          Strncat( buf, "-", 255 );
          Strncat( buf, oName, 255 );

          cur->objName = new char[strlen(buf)+1];
          strcpy( cur->objName, buf );

          cur->fileName = new char[strlen(fName)+1];
          strcpy( cur->fileName, fName );

          stat = avl_insert_node( this->schemeList, (void *) cur, &dup );
          if ( !( stat & 1 ) ) {
            fclose( f );
            numSchemeSets = 0;
            schemeListExists = 0;
            return;
          }
          if ( dup ) {
            fprintf( stderr, "appContextClass::buildSchemeList dups\n" );
          }

        }
        else {
          fprintf( stderr, "appContextClass::buildSchemeList syntax err 5\n" );
          fclose( f );
          numSchemeSets = 0;
          schemeListExists = 0;
          return;
        }

      }
      else {
        fprintf( stderr, "appContextClass::buildSchemeList syntax err 6\n" );
        fclose( f );
        numSchemeSets = 0;
        schemeListExists = 0;
        return;
      }

      break;

    }

  }

done:

  //fprintf( stderr, "numSchemeSets = %-d\n", numSchemeSets );

  stat = avl_get_first( schemeSet, (void **) &curSet );
  if ( !( stat & 1 ) ) {
    numSchemeSets = 0;
    schemeListExists = 0;
    return;
  }

  schemeSetList = new char *[numSchemeSets];
  i = 0;
  while ( curSet ) {

    //fprintf( stderr, "curSet->objName = [%s]\n", curSet->objName );

    schemeSetList[i] = new char[strlen(curSet->objName)+1];
    strcpy( schemeSetList[i], curSet->objName );

    stat = avl_get_next( schemeSet, (void **) &curSet );
    if ( !( stat & 1 ) ) {
      numSchemeSets = 0;
      schemeListExists = 0;
      return;
    }

    i++;

  }

}

void appContextClass::destroySchemeList ( void )
{

int stat, i;
schemeListPtr cur;

  if ( schemeSetList ) {
    for ( i=0; i<numSchemeSets; i++ ) {
      delete[] schemeSetList[i];
    }
    delete[] schemeSetList;
  }

  if ( !schemeList ) return;

  stat = avl_get_first( schemeList, (void **) &cur );
  if ( !( stat & 1 ) ) {
    return;
  }

  while ( cur ) {

    stat = avl_delete_node( schemeList, (void **) &cur );
    delete[] cur->objName;
    delete[] cur->fileName;
    delete cur;

    stat = avl_get_first( schemeList, (void **) &cur );
    if ( !( stat & 1 ) ) {
      return;
    }

  }

  stat = avl_destroy( schemeList );

  if ( !schemeSet ) return;

  stat = avl_get_first( schemeSet, (void **) &cur );
  if ( !( stat & 1 ) ) {
    return;
  }

  while ( cur ) {

    stat = avl_delete_node( schemeSet, (void **) &cur );
    delete[] cur->objName;
    delete cur;

    stat = avl_get_first( schemeSet, (void **) &cur );
    if ( !( stat & 1 ) ) {
      return;
    }

  }

  stat = avl_destroy( schemeSet );

}

int appContextClass::schemeExists (
  char *schemeSetName,
  char *objName,
  char *objType )
{

int stat;
schemeListPtr cur;
char buf[255+1];

  if ( !schemeListExists ) {
    return 0;
  }

  if ( strcmp( schemeSetName, "" ) == 0 ) {
    return 0;
  }

  strncpy( buf, schemeSetName, 255 );
  Strncat( buf, "-", 255 );
  Strncat( buf, objType, 255 );
  Strncat( buf, "-", 255 );
  Strncat( buf, objName, 255 );

  stat = avl_get_match( this->schemeList, (void *) buf,
   (void **) &cur );
  if ( !( stat & 1 ) ) {
    return 0;
  }

  if ( cur ) {
    return 1;
  }
  else {
    return 0;
  }

}

void appContextClass::getScheme (
  char *schemeSetName,
  char *objName,
  char *objType,
  char *schemeFileName,
  int maxLen )
{

int stat;
schemeListPtr cur;
char buf[255+1];

  if ( !schemeListExists ) {
    strcpy( schemeFileName, "" );
    return;
  }

  if ( strcmp( schemeSetName, "" ) == 0 ) {
    strcpy( schemeFileName, "" );
    return;
  }

  strncpy( buf, schemeSetName, 255 );
  Strncat( buf, "-", 255 );
  Strncat( buf, objType, 255 );
  Strncat( buf, "-", 255 );
  Strncat( buf, objName, 255 );

  //fprintf( stderr, "get scheme file for %s\n", buf );

  stat = avl_get_match( this->schemeList, (void *) buf,
   (void **) &cur );
  if ( !( stat & 1 ) ) {
    strcpy( schemeFileName, "" );
    return;
  }

  if ( cur ) {
    strncpy( schemeFileName, cur->fileName, maxLen );
  }
  else {
    strncpy( schemeFileName, "default", maxLen );
  }

}

void appContextClass::termDeferredExecutionQueue ( void )
{

int stat;

  stat = sys_destroyq( &appDefExeFreeQueue );

  stat = sys_destroyq( &appDefExeActiveQueue );

  stat = sys_destroyq( &appDefExeActiveNextQueue );

}

int appContextClass::initDeferredExecutionQueue ( void )
{

int i, stat;

  stat = sys_iniq( &appDefExeFreeQueue );
  if ( !( stat & 1 ) ) {
    fprintf( stderr, appContextClass_str30 );
    return 2;
  }
  stat = sys_iniq( &appDefExeActiveQueue );
  if ( !( stat & 1 ) ) {
    fprintf( stderr, appContextClass_str30 );
    return 2;
  }
  stat = sys_iniq( &appDefExeActiveNextQueue );
  if ( !( stat & 1 ) ) {
    fprintf( stderr, appContextClass_str30 );
    return 2;
  }

  appDefExeFreeQueue.flink = NULL;
  appDefExeFreeQueue.blink = NULL;
  appDefExeActiveQueue.flink = NULL;
  appDefExeActiveQueue.blink = NULL;
  appDefExeActiveNextQueue.flink = NULL;
  appDefExeActiveNextQueue.blink = NULL;

  for ( i=0; i<APPDEFEXE_QUEUE_SIZE; i++ ) {

    stat = INSQTI( (void *) &appDefExeNodes[i], (void *) &appDefExeFreeQueue,
     0 );
    if ( !( stat & 1 ) ) {
      fprintf( stderr, appContextClass_str30 );
      return 2;
    }

  }

  return 1;

}

void appContextClass::removeAllDeferredExecutionQueueNode (
  class activeWindowClass *awo )
{

  // remove all nodes associated with awo

int q_stat_r, q_stat_i;
APPDEFEXE_NODE_PTR node;

  // first, place all next queue nodes on active queue
  do {

    q_stat_r = REMQHI( (void *) &appDefExeActiveNextQueue, (void **) &node,
     0 );

    if ( q_stat_r & 1 ) {

      q_stat_i = INSQTI( (void *) node, (void *) &appDefExeActiveQueue,
       0 );
      if ( !( q_stat_i & 1 ) ) {
        fprintf( stderr, appContextClass_str33 );
      }

    }
    else if ( q_stat_r != QUEWASEMP ) {
      fprintf( stderr, appContextClass_str32 );
    }

  } while ( q_stat_r & 1 );

  // now, remove all associated nodes from active queue
  do {

    q_stat_r = REMQHI( (void *) &appDefExeActiveQueue, (void **) &node, 0 );

    if ( q_stat_r & 1 ) {

      if ( node->awObj ) { // active window object

	if ( node->awObj == awo ) { // remove node

          q_stat_i = INSQTI( (void *) node, (void *) &appDefExeFreeQueue, 0 );
          if ( !( q_stat_i & 1 ) ) {
            fprintf( stderr, appContextClass_str31 );
          }

	}
	else { // don't remove, put it back

          q_stat_i = INSQTI( (void *) node, (void *) &appDefExeActiveNextQueue,
           0 );
          if ( !( q_stat_i & 1 ) ) {
            fprintf( stderr, appContextClass_str33 );
          }

	}

      }
      else { // active graphics object

        if ( node->obj->actWin == awo ) {  // remove node

          q_stat_i = INSQTI( (void *) node, (void *) &appDefExeFreeQueue, 0 );
          if ( !( q_stat_i & 1 ) ) {
            fprintf( stderr, appContextClass_str31 );
          }

	}
	else { // don't remove, put it back

          q_stat_i = INSQTI( (void *) node, (void *) &appDefExeActiveNextQueue,
           0 );
          if ( !( q_stat_i & 1 ) ) {
            fprintf( stderr, appContextClass_str33 );
          }

	}

      }

    }
    else if ( q_stat_r != QUEWASEMP ) {
      fprintf( stderr, appContextClass_str32 );
    }

  } while ( q_stat_r & 1 );

}

void appContextClass::processDeferredExecutionQueue ( void )
{

int q_stat_r, q_stat_i;
APPDEFEXE_NODE_PTR node;

  // first, place all next queue nodes on active queue
  do {

    q_stat_r = REMQHI( (void *) &appDefExeActiveNextQueue, (void **) &node,
     0 );

    if ( q_stat_r & 1 ) {

      q_stat_i = INSQTI( (void *) node, (void *) &appDefExeActiveQueue,
       0 );
      if ( !( q_stat_i & 1 ) ) {
        fprintf( stderr, appContextClass_str33 );
      }

    }
    else if ( q_stat_r != QUEWASEMP ) {
      fprintf( stderr, appContextClass_str32 );
    }

  } while ( q_stat_r & 1 );

  // process all active nodes
  do {

    q_stat_r = REMQHI( (void *) &appDefExeActiveQueue, (void **) &node, 0 );

    if ( q_stat_r & 1 ) {

      if ( node->obj ) node->obj->executeFromDeferredQueue();
      if ( node->awObj ) node->awObj->executeFromDeferredQueue();

      q_stat_i = INSQTI( (void *) node, (void *) &appDefExeFreeQueue, 0 );
      if ( !( q_stat_i & 1 ) ) {
        fprintf( stderr, appContextClass_str31 );
      }

    }
    else if ( q_stat_r != QUEWASEMP ) {
      fprintf( stderr, appContextClass_str32 );
    }

  } while ( q_stat_r & 1 );

}

void appContextClass::postDeferredExecutionQueue (
  class activeGraphicClass *ptr )
{

int q_stat_r, q_stat_i;
APPDEFEXE_NODE_PTR node;

    q_stat_r = REMQHI( (void *) &appDefExeFreeQueue, (void **) &node, 0 );

    if ( q_stat_r & 1 ) {

      node->awObj = NULL;
      node->obj = ptr;

      q_stat_i = INSQTI( (void *) node, (void *) &appDefExeActiveQueue, 0 );
      if ( !( q_stat_i & 1 ) ) {
        fprintf( stderr, appContextClass_str33 );
      }

    }
    else {
      fprintf( stderr, appContextClass_str34 );
    }

}

void appContextClass::postDeferredExecutionQueue (
  class activeWindowClass *ptr )
{

int q_stat_r, q_stat_i;
APPDEFEXE_NODE_PTR node;

    q_stat_r = REMQHI( (void *) &appDefExeFreeQueue, (void **) &node, 0 );

    if ( q_stat_r & 1 ) {

      node->awObj = ptr;
      node->obj = NULL;

      q_stat_i = INSQTI( (void *) node, (void *) &appDefExeActiveQueue, 0 );
      if ( !( q_stat_i & 1 ) ) {
        fprintf( stderr, appContextClass_str33 );
      }

    }
    else {
      fprintf( stderr, appContextClass_str34 );
    }

}

void appContextClass::postDeferredExecutionNextQueue (
  class activeGraphicClass *ptr )
{

int q_stat_r, q_stat_i;
APPDEFEXE_NODE_PTR node;

    q_stat_r = REMQHI( (void *) &appDefExeFreeQueue, (void **) &node, 0 );

    if ( q_stat_r & 1 ) {

      node->awObj = NULL;
      node->obj = ptr;

      q_stat_i = INSQTI( (void *) node, (void *) &appDefExeActiveNextQueue,
       0 );
      if ( !( q_stat_i & 1 ) ) {
        fprintf( stderr, appContextClass_str33 );
      }

    }
    else {
      fprintf( stderr, appContextClass_str34 );
    }

}

void appContextClass::postDeferredExecutionNextQueue (
  class activeWindowClass *ptr )
{

int q_stat_r, q_stat_i;
APPDEFEXE_NODE_PTR node;

    q_stat_r = REMQHI( (void *) &appDefExeFreeQueue, (void **) &node, 0 );

    if ( q_stat_r & 1 ) {

      node->awObj = ptr;
      node->obj = NULL;

      q_stat_i = INSQTI( (void *) node, (void *) &appDefExeActiveNextQueue,
       0 );
      if ( !( q_stat_i & 1 ) ) {
        fprintf( stderr, appContextClass_str33 );
      }

    }
    else {
      fprintf( stderr, appContextClass_str34 );
    }

}

void appContextClass::createMainWindow ( void ) {

XmString menuStr, str;
callbackBlockPtr curBlock;
int i, numVisible;

  mainWin = XtVaCreateManagedWidget( "main", xmMainWindowWidgetClass,
   appTop,
   XmNscrollBarDisplayPolicy, XmAS_NEEDED,
   XmNscrollingPolicy, XmAUTOMATIC,
   NULL );

  // create menubars
  menuBar = XmCreateMenuBar( mainWin, "menubar", NULL, 0 );

  filePullDown = XmCreatePulldownMenu( menuBar, "file", NULL, 0 );

  menuStr = XmStringCreateLocalized( appContextClass_str35 );    // File
  fileCascade = XtVaCreateManagedWidget( "filemenu", xmCascadeButtonWidgetClass,
   menuBar,
   XmNlabelString, menuStr,
   XmNmnemonic, XStringToKeysym("f"),
   XmNsubMenuId, filePullDown,
   NULL );
  XmStringFree( menuStr );

  if ( !noEdit ) {

    str = XmStringCreateLocalized( appContextClass_str36 );    // New
    newB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
     filePullDown,
     XmNlabelString, str,
     NULL );
    XmStringFree( str );
    XtAddCallback( newB, XmNactivateCallback, new_cb,
     (XtPointer) this );

  }

  str = XmStringCreateLocalized( appContextClass_str37 );     // Open by Path...
  newB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   filePullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( newB, XmNactivateCallback, open_from_path_cb,
   (XtPointer) this );

  str = XmStringCreateLocalized( appContextClass_str38 );     // Open ...
  newB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   filePullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( newB, XmNactivateCallback, open_cb,
   (XtPointer) this );

#if 0
  str = XmStringCreateLocalized( appContextClass_str39 );     // Import Exchange File...
  newB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   filePullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( newB, XmNactivateCallback, import_cb,
   (XtPointer) this );
#endif

  str = XmStringCreateLocalized( appContextClass_str40 );     // Refresh User Library
  newB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   filePullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( newB, XmNactivateCallback, refreshUserLib_cb,
   (XtPointer) this );

  str = XmStringCreateLocalized( appContextClass_str46 );     //Reload All
  newB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   filePullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( newB, XmNactivateCallback, reload_cb,
   (XtPointer) this );

  // save sreen config
  if ( primaryServer ) {
    if ( primaryServer == 1 ) {
      str = XmStringCreateLocalized( appContextClass_str47 );     // Save Screen Configuration
      newB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
        filePullDown,
        XmNlabelString, str,
        NULL );
      XmStringFree( str );
      XtAddCallback( newB, XmNactivateCallback, save_screen_config,
        (XtPointer) this );
    }
  }

  // switch screen config
  if ( primaryServer ) {
    if ( primaryServer == 1 ) {
      str = XmStringCreateLocalized( appContextClass_str48 );     // Switch Screen Configuration
      newB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
        filePullDown,
        XmNlabelString, str,
        NULL );
      XmStringFree( str );
      XtAddCallback( newB, XmNactivateCallback, switch_screen_config,
        (XtPointer) this );
    }
  }

  // add to sreen config
  if ( primaryServer ) {
    if ( primaryServer == 1 ) {
      str = XmStringCreateLocalized( appContextClass_str49 );     // Add to Screen Configuration
      newB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
        filePullDown,
        XmNlabelString, str,
        NULL );
      XmStringFree( str );
      XtAddCallback( newB, XmNactivateCallback, add_screen_config,
        (XtPointer) this );
    }
  }

  // load sreen config
  if ( primaryServer ) {
    if ( primaryServer == 1 ) {
      str = XmStringCreateLocalized( appContextClass_str50 );     // Load Screen Configuration
      newB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
        filePullDown,
        XmNlabelString, str,
        NULL );
      XmStringFree( str );
      XtAddCallback( newB, XmNactivateCallback, addall_screen_config,
        (XtPointer) this );
    }
  }

  str = XmStringCreateLocalized( appContextClass_str41 );     // Exit
  newB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   filePullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( newB, XmNactivateCallback, exit_cb,
   (XtPointer) this );

  if ( primaryServer ) {
    if ( primaryServer == 1 ) {
      str = XmStringCreateLocalized( appContextClass_str132 );     // Shutdown
    }
    else if ( primaryServer == 2 ) {
      str = XmStringCreateLocalized( appContextClass_str133 );     // Shutdown all displays
    }
    newB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
     filePullDown,
     XmNlabelString, str,
     NULL );
    XmStringFree( str );
    XtAddCallback( newB, XmNactivateCallback, shutdown_cb,
     (XtPointer) this );
  }


  viewPullDown = XmCreatePulldownMenu( menuBar, "view", NULL, 0 );

  menuStr = XmStringCreateLocalized( appContextClass_str42 );     // View ...
  viewCascade = XtVaCreateManagedWidget( "viewmenu", xmCascadeButtonWidgetClass,
   menuBar,
   XmNlabelString, menuStr,
   XmNmnemonic, XStringToKeysym("v"),
   XmNsubMenuId, viewPullDown,
   NULL );
  XmStringFree( menuStr );

  str = XmStringCreateLocalized( appContextClass_str43 );     // Messages
  msgB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   viewPullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( msgB, XmNactivateCallback, view_msgBox_cb,
   (XtPointer) this );

  str = XmStringCreateLocalized( appContextClass_str44 );     // PV List
  pvB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   viewPullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( pvB, XmNactivateCallback, view_pvList_cb,
   (XtPointer) this );

  str = XmStringCreateLocalized( appContextClass_str45 );     // Screens
  pvB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   viewPullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( pvB, XmNactivateCallback, view_screens_cb,
   (XtPointer) this );

  str = XmStringCreateLocalized( appContextClass_str112 );     // Show X/Y
  viewXyB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   viewPullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( viewXyB, XmNactivateCallback, view_xy_cb,
   (XtPointer) this );

  str = XmStringCreateLocalized( appContextClass_str135 );     // Disable image rendering
  renderImagesB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   viewPullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( renderImagesB, XmNactivateCallback, renderImages_cb,
   (XtPointer) this );

  str = XmStringCreateLocalized( appContextClass_str136 );     // Checkpoint PID
  checkpointPidB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   viewPullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( checkpointPidB, XmNactivateCallback, checkpointPid_cb,
   (XtPointer) this );

  str = XmStringCreateLocalized( appContextClass_str141 );     // Font Mappings
  viewFontMappingB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   viewPullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( viewFontMappingB, XmNactivateCallback, viewFontMapping_cb,
   (XtPointer) this );

  str = XmStringCreateLocalized( appContextClass_str144 );     // Environment
  viewEnvB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   viewPullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( viewEnvB, XmNactivateCallback, viewEnv_cb,
   (XtPointer) this );

  // --------------------------------------------------------------------

  if ( numPaths <= 30 ) {

    pathPullDown = XmCreatePulldownMenu( menuBar, "path", NULL, 0 );

    menuStr = XmStringCreateLocalized( appContextClass_str121 );     // Path
    pathCascade = XtVaCreateManagedWidget( "pathmenu", xmCascadeButtonWidgetClass,
     menuBar,
     XmNlabelString, menuStr,
     XmNmnemonic, XStringToKeysym("p"),
     XmNsubMenuId, pathPullDown,
     NULL );
    XmStringFree( menuStr );

    for ( i=0; i<numPaths; i++ ) {

      curBlock = new callbackBlockType;
      curBlock->ptr = (void *) dataFilePrefix[i];
      curBlock->apco = this;

      callbackBlockTail->flink = curBlock;
      callbackBlockTail = curBlock;
      callbackBlockTail->flink = NULL;

      str = XmStringCreateLocalized( dataFilePrefix[i] );
      msgB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
       pathPullDown,
       XmNlabelString, str,
       NULL );
      XmStringFree( str );
      XtAddCallback( msgB, XmNactivateCallback, setPath_cb,
       (XtPointer) curBlock );

    }

  }
  else {

    pathPullDown = XmCreatePulldownMenu( menuBar, "path", NULL, 0 );

    numVisible = ( numPaths > 30 ? 30 : numPaths );
    pathList.create( numPaths, mainWin, numVisible, this );

    menuStr = XmStringCreateLocalized( appContextClass_str121 );     // Path
    pathCascade = XtVaCreateManagedWidget( "pathmenu", xmCascadeButtonWidgetClass,
     menuBar,
     XmNlabelString, menuStr,
     XmNmnemonic, XStringToKeysym("p"),
     XmNsubMenuId, pathPullDown,
     NULL );
    XmStringFree( menuStr );

    curBlock = new callbackBlockType;
    curBlock->ptr = NULL;
    curBlock->apco = this;

    callbackBlockTail->flink = curBlock;
    callbackBlockTail = curBlock;
    callbackBlockTail->flink = NULL;

    str = XmStringCreateLocalized( appContextClass_str143 );     // Select Path...
    msgB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
     pathPullDown,
     XmNlabelString, str,
     NULL );
    XmStringFree( str );
    XtAddCallback( msgB, XmNactivateCallback, selectPath_cb,
     (XtPointer) curBlock );

    for ( i=0; i<numPaths; i++ ) {
      pathList.addItem( dataFilePrefix[i] );
    }

  }


  helpPullDown = XmCreatePulldownMenu( menuBar, "help", NULL, 0 );

  menuStr = XmStringCreateLocalized( appContextClass_str114 );     // Help
  helpCascade = XtVaCreateManagedWidget( "helpmenu", xmCascadeButtonWidgetClass,
   menuBar,
   XmNlabelString, menuStr,
   XmNmnemonic, XStringToKeysym("h"),
   XmNsubMenuId, helpPullDown,
   NULL );
  XmStringFree( menuStr );

  str = XmStringCreateLocalized( appContextClass_str115 );     // On-line
  msgB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   helpPullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( msgB, XmNactivateCallback, help_cb,
   (XtPointer) this );

  //str = XmStringCreateLocalized( appContextClass_str116 );     // About
  //msgB = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
  // helpPullDown,
  // XmNlabelString, str,
  // NULL );
  //XmStringFree( str );
  //XtAddCallback( msgB, XmNactivateCallback, help_cb,
  // (XtPointer) this );

  //mainDrawingArea = XtVaCreateManagedWidget( "", xmDrawingAreaWidgetClass,
  // mainWin,
  // NULL );

  XtVaSetValues( menuBar,
   XmNmenuHelpWidget, helpCascade,
   NULL );

  XtManageChild( menuBar );

  msgDialog.createWithOffset( appTop );

}

void appContextClass::addActiveWindow (
  activeWindowListPtr node )
{

  // link in

  //strcpy( node->winName, "" );
  node->requestDelete = 0;
  node->requestActivate = 0;
  node->requestActivateClear = 0;
  node->requestReactivate = 0;
  node->requestOpen = 0;
  node->requestPosition = 0;
  node->requestImport = 0;
  node->requestRefresh = 0;
  node->requestActiveRedraw = 0;
  node->requestIconize = 0;
  node->requestConvertAndExit = 0;

  node->blink = head->blink;
  head->blink->flink = node;
  head->blink = node;
  node->flink = head;

}

int appContextClass::refreshActiveWindow (
  activeWindowClass *activeWindowNode )
{

  // simply mark for refresh and increment request flag

activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( &(cur->node) == activeWindowNode ) {
      if ( !cur->requestRefresh ) {
        cur->requestRefresh = 1;
        requestFlag++;
      }
    }
    cur = cur->flink;
  }

  return 1;

}

int appContextClass::smartDrawAllActive (
  activeWindowClass *activeWindowNode )
{

  // simply mark for redraw and increment request flag

activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( &(cur->node) == activeWindowNode ) {
      if ( !cur->requestActiveRedraw ) {
        cur->requestActiveRedraw = 1;
        requestFlag++;
      }
    }
    cur = cur->flink;
  }

  return 1;

}

int appContextClass::removeActiveWindow (
  activeWindowClass *activeWindowNode )
{

  // simply mark for delete and increment request flag

activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( &(cur->node) == activeWindowNode ) {
      cur->requestDelete = 1;
      requestFlag++;
    }
    cur = cur->flink;
  }

  return 1;

}

int appContextClass::openEditActiveWindow (
  activeWindowClass *activeWindowNode )
{

  // simply mark for open and increment request flag

activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( &(cur->node) == activeWindowNode ) {
      cur->requestOpen = 1;
      cur->requestPosition = 0;
      cur->requestImport = 0;
      requestFlag++;
    }
    cur = cur->flink;
  }

  return 1;

}

int appContextClass::openActivateActiveWindow (
  activeWindowClass *activeWindowNode )
{

  // simply mark for open and activation and increment request flag

activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( &(cur->node) == activeWindowNode ) {
      cur->requestOpen = 1;
      cur->requestPosition = 0;
      cur->requestImport = 0;
      cur->requestIconize = 0;
      cur->requestConvertAndExit = 0;
      requestFlag++;
      cur->requestActivate = 1;
      requestFlag++;
    }
    cur = cur->flink;
  }

  return 1;

}

int appContextClass::openActivateActiveWindow (
  activeWindowClass *activeWindowNode,
  int x,
  int y )
{

  // simply mark for open and activation and increment request flag

activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( &(cur->node) == activeWindowNode ) {
      cur->requestOpen = 1;
      cur->requestPosition = 1;
      cur->requestImport = 0;
      cur->requestIconize = 0;
      cur->requestConvertAndExit = 0;
      cur->x = x;
      cur->y = y;
      requestFlag++;
      cur->requestActivate = 1;
      requestFlag++;
    }
    cur = cur->flink;
  }

  return 1;

}

int appContextClass::openActivateIconifiedActiveWindow (
  activeWindowClass *activeWindowNode,
  int x,
  int y )
{

  // simply mark for open and activation and increment request flag
  // request window to be iconified

activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( &(cur->node) == activeWindowNode ) {
      cur->requestOpen = 1;
      cur->requestPosition = 1;
      cur->requestImport = 0;
      cur->requestIconize = 1;
      cur->requestConvertAndExit = 0;
      cur->x = x;
      cur->y = y;
      requestFlag++;
      cur->requestActivate = 1;
      requestFlag++;
    }
    cur = cur->flink;
  }

  return 1;

}

int appContextClass::activateActiveWindow (
  activeWindowClass *activeWindowNode )
{

  // simply mark for activation and increment request flag

activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( &(cur->node) == activeWindowNode ) {
      cur->requestActivate = 1;
      requestFlag++;
    }
    cur = cur->flink;
  }

  return 1;

}

int appContextClass::reactivateActiveWindow (
  activeWindowClass *activeWindowNode )
{

  // simply mark for reactivation and increment request flag

activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( &(cur->node) == activeWindowNode ) {
      cur->requestReactivate = 1;
      requestFlag++;
    }
    cur = cur->flink;
  }

  return 1;

}

static void displayVersion ( void ) {

  fprintf( stderr, "  edm version %s Copyright (C) 1999-2016 John W. Sinclair\n", VERSION );
  fprintf( stderr, "\n" );

  fprintf( stderr, "  This program is free software; you can redistribute it and/or modify\n" );
  fprintf( stderr, "  it under the terms of the GNU General Public License as published by\n" );
  fprintf( stderr, "  the Free Software Foundation; either version 2 of the License, or\n" );
  fprintf( stderr, "  (at your option) any later version.\n" );
  fprintf( stderr, "\n" );

fprintf( stderr, "  This program is distributed in the hope that it will be useful,\n" );
  fprintf( stderr, "  but WITHOUT ANY WARRANTY; without even the implied warranty of\n" );
  fprintf( stderr, "  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n" );
  fprintf( stderr, "  GNU General Public License for more details.\n" );
  fprintf( stderr, "\n" );
 
fprintf( stderr, "  You should have received a copy of the GNU General Public License\n" );
  fprintf( stderr, "  along with this program; if not, write to the Free Software\n" );
  fprintf( stderr, "  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n" );

}

static void displayParamInfo ( void ) {

  fprintf( stderr, "\n" );
  displayVersion();

  fprintf( stderr, "\n" );
   fprintf( stderr, global_str24 );
  fprintf( stderr, "\n" );

  fprintf( stderr, global_str25 );

  fprintf( stderr, global_str27 );

  fprintf( stderr, global_str28 );

  fprintf( stderr, global_str29 );

  fprintf( stderr, global_str30 );
  fprintf( stderr, global_str31 );
  fprintf( stderr, global_str99 );
  fprintf( stderr, global_str101 );

  fprintf( stderr, global_str32 );
  fprintf( stderr, global_str33 );

  fprintf( stderr, global_str35 );
  fprintf( stderr, global_str36 );
  fprintf( stderr, global_str37 );
  fprintf( stderr, global_str87 );
  fprintf( stderr, global_str88 );

  fprintf( stderr, global_str77 );
  fprintf( stderr, global_str78 );
  fprintf( stderr, global_str85 );
  fprintf( stderr, global_str92 );

  fprintf( stderr, global_str57 );
  fprintf( stderr, global_str58 );
  fprintf( stderr, global_str59 );

  fprintf( stderr, global_str74 );

  fprintf( stderr, global_str60 );
  fprintf( stderr, global_str61 );

  fprintf( stderr, global_str90 );

  fprintf( stderr, global_str94 );

  fprintf( stderr, global_str129 );

  fprintf( stderr, global_str131 );

  fprintf( stderr, global_str133 );

  fprintf( stderr, global_str135 );

  fprintf( stderr, global_str137 );

  fprintf( stderr, global_str139 );

  fprintf( stderr, global_str141 );

  fprintf( stderr, global_str147 );

  fprintf( stderr, global_str97 );

  fprintf( stderr, global_str107 );

  fprintf( stderr, global_str103 );
  fprintf( stderr, global_str105 );

  fprintf( stderr, global_str38 );

  fprintf( stderr, global_str39 );

  fprintf( stderr, global_str40 );

  fprintf( stderr, global_str41 );
  fprintf( stderr, global_str42 );
  fprintf( stderr, global_str43 );
  fprintf( stderr, global_str44 );
  fprintf( stderr, global_str45 );

  fprintf( stderr, global_str66 );

  fprintf( stderr, global_str67 );

  fprintf( stderr, global_str68 );
  fprintf( stderr, global_str69 );
  fprintf( stderr, global_str70 );
  fprintf( stderr, global_str71 );
  fprintf( stderr, global_str72 );

  fprintf( stderr, global_str46 );
  fprintf( stderr, global_str47 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str75 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str48 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str49 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str50 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str51 );
  fprintf( stderr, global_str52 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str53 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str80 );
  fprintf( stderr, global_str81 );
  fprintf( stderr, global_str82 );
  fprintf( stderr, global_str83 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str84 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str156 );    // EDMSCREENCFG
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str158 );    // EDMSELECTOR
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str95 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str54 );
  fprintf( stderr, "\n" );

  fprintf( stderr, global_str108 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str109 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str110 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str111 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str112 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str113 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str114 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str115 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str116 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str117 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str118 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str120 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str121 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str122 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str123 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str124 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str125 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str126 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str127 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str143 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str144 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str145 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str149 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str150 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str151 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str152 );
  fprintf( stderr, "\n" );
  fprintf( stderr, global_str155 );
  fprintf( stderr, "\n" );

}

#define DONE -1
#define SWITCHES 1
#define FILES 2

int appContextClass::getParams(
  int argc,
  char **argv )
{

char buf[1023+1], mac[1023+1], exp[1023+1];
int state = SWITCHES;
int i, n = 1;
char *tk;
macroListPtr curMacro;
fileListPtr curFile;

  strcpy( displayName, "" );
  strcpy( colormode, "" );
  local = 0;
  privColorMap = 0;
  exitOnLastClose = 0;
  useScrollBars = 1;

  // check first for component management commands
  if ( argc > 1 ) {

    if ( ( strcmp( argv[1], global_str6 ) == 0 ) ||
         ( strcmp( argv[1], global_str7 ) == 0 ) ||
         ( strcmp( argv[1], global_str8 ) == 0 ) ) {
      if ( argc < 3 ) {
        fprintf( stderr, appContextClass_str77 );
        displayParamInfo();
        exit(0);
      }
      manageComponents( argv[1], argv[2] );
      fprintf( stderr, "\n\n" );
      exit(0);
    }

  }

  if ( argc > 1 ) {

    if ( ( strcmp( argv[1], global_str63 ) == 0 ) ||
         ( strcmp( argv[1], global_str64 ) == 0 ) ||
         ( strcmp( argv[1], global_str65 ) == 0 ) ) {
      if ( argc < 3 ) {
        fprintf( stderr, appContextClass_str77 );
        displayParamInfo();
        exit(0);
      }
      managePvComponents( argv[1], argv[2] );
      fprintf( stderr, "\n\n" );
      exit(0);
    }

  }

  while ( n < argc ) {

    switch ( state ) {

    case SWITCHES:

      if ( argv[n][0] == '-' ) {

        if ( strcmp( argv[n], global_str11 ) == 0 ) {
          displayParamInfo();
          exit(0);
        }
        else if ( strcmp( argv[n], global_str12 ) == 0 ) {
          displayParamInfo();
          exit(0);
        }
        else if ( strcmp( argv[n], global_str13 ) == 0 ) {
          displayParamInfo();
          exit(0);
        }
        else if ( strcmp( argv[n], global_str14 ) == 0 ) {
          fprintf( stderr, "\n" );
          displayVersion();
          fprintf( stderr, "\n" );
          exit(0);
        }
        else if ( strcmp( argv[n], global_str15 ) == 0 ) {
          fprintf( stderr, "\n" );
          displayVersion();
          fprintf( stderr, "\n" );
          exit(0);
        }
        else if ( strcmp( argv[n], global_str16 ) == 0 ) {
          fprintf( stderr, "\n" );
          displayVersion();
          fprintf( stderr, "\n" );
          exit(0);
        }
        else if ( strcmp( argv[n], global_str17 ) == 0 ) {
          executeOnOpen = 1;
        }
        else if ( strcmp( argv[n], global_str9 ) == 0 ) {
          local = 1;
        }
        else if ( strcmp( argv[n], global_str10 ) == 0 ) {
        }
        else if ( strcmp( argv[n], global_str89 ) == 0 ) {
        }
        else if ( strcmp( argv[n], global_str91 ) == 0 ) {
        }
        else if ( strcmp( argv[n], global_str102 ) == 0 ) {
        }
        else if ( strcmp( argv[n], global_str104 ) == 0 ) {
        }
        else if ( strcmp( argv[n], global_str93 ) == 0 ) {
        }
        else if ( strcmp( argv[n], global_str128 ) == 0 ) {
        }
        else if ( strcmp( argv[n], global_str130 ) == 0 ) {
        }
        else if ( strcmp( argv[n], global_str132 ) == 0 ) {
        }
        else if ( strcmp( argv[n], global_str134 ) == 0 ) {
        }
        else if ( strcmp( argv[n], global_str136 ) == 0 ) {
        }
        else if ( strcmp( argv[n], global_str138 ) == 0 ) {
        }
        else if ( strcmp( argv[n], global_str140 ) == 0 ) {
        }
        else if ( strcmp( argv[n], global_str146 ) == 0 ) {
        }
        else if ( strcmp( argv[n], global_str86 ) == 0 ) {
          n++; // just ignore, not used here
          if ( n >= argc ) return 2;
        }
        else if ( strcmp( argv[n], global_str18 ) == 0 ) {
          noEdit = 1;
        }
        else if ( strcmp( argv[n], global_str19 ) == 0 ) {
          n++;
          if ( n >= argc ) return 2; // missing macro arg
          strncpy( buf, argv[n], 1023 );
          tk = strtok( buf, "=," );
          while ( tk ) {
            strncpy( mac, tk, 1023 );
            tk = strtok( NULL, "=," );
            if ( tk ) {
              if ( strcmp( tk, "\"\"" ) == 0 ) {
                strcpy( exp, "" );
	            }
              else if ( strcmp( tk, "\'\'" ) == 0 ) {
                strcpy( exp, "" );
	            }
              else if ( strcmp( tk, "\\'\\'" ) == 0 ) {
                strcpy( exp, "" );
	            }
	            else {
                strncpy( exp, tk, 1023 );
	            }
            }
            else {
              return 4; // macro, but no value
            }
            numMacros++;
            curMacro = new macroListType;
            if ( !curMacro ) return 6;
            curMacro->macro = new char[strlen(mac)+1];
            if ( !curMacro->macro ) return 6;
            strcpy( curMacro->macro, mac );
            curMacro->expansion = new char[strlen(exp)+1];
            if ( !curMacro->expansion ) return 6;
            strcpy( curMacro->expansion, exp );
            macroHead->blink->flink = curMacro;
            curMacro->flink = macroHead;
            curMacro->blink = macroHead->blink;
            macroHead->blink = curMacro;
            tk = strtok( NULL, "=," );
          }

          if ( numMacros == 0 ) return 4;

        }
        else if ( strcmp( argv[n], global_str20 ) == 0 ) {
          n++;
          if ( n >= argc ) return 2;
          strncpy( ctlPV, argv[n], 127 );
        }
        else if ( strcmp( argv[n], global_str22 ) == 0 ) {
          n++;
          if ( n >= argc ) return 2;
          strncpy( userLib, argv[n], 127 );
          userLibObject.openUserLibrary( userLib );
        }
        else if ( strcmp( argv[n], global_str21 ) == 0 ) {
          n++;
          if ( n >= argc ) return 2;
          strncpy( displayName, argv[n], 63 );
        }
        else if ( strcmp( argv[n], global_str73 ) == 0 ) {
          n++; // just ignore, not used here
          if ( n >= argc ) return 2;
        }
        else if ( strcmp( argv[n], global_str76 ) == 0 ) {
          n++;
          if ( n >= argc ) return 2;
          strncpy( colormode, argv[n], 7 ); // index (default) or rgb
        }
        else if ( strcmp( argv[n], global_str79 ) == 0 ) { // private colormap
          privColorMap = 1;
        }
	else if ( strcmp( argv[n], global_str96 ) == 0 ) { //exit on last close
	  exitOnLastClose = 1;
	}
	else if ( strcmp( argv[n], global_str98 ) == 0 ) { //readonly
	}
	else if ( strcmp( argv[n], global_str100 ) == 0 ) { //disable scroll bars
          useScrollBars = 0;
	}
	else if ( strcmp( argv[n], global_str106 ) == 0 ) { //noautomsg
          msgBox.setAutoOpen( 0 );
	}

        else {
          return -2;
        }

        n++;

      }
      else {

        state = FILES;

      }

      break;

    case FILES:

      if ( argv[n][0] == '-' ) {

        return 8;

      }
      else {

        numFiles++;
        curFile = new fileListType;
        if ( !curFile ) return 6;
        curFile->file = new char[strlen(argv[n])+1];
        if ( !curFile->file ) return 6;
        strcpy( curFile->file, argv[n] );
        fileHead->blink->flink = curFile;
        curFile->flink = fileHead;
        curFile->blink = fileHead->blink;
        fileHead->blink = curFile;

        n++;

      }

      break;

    }

  }

  if ( numMacros > 0 ) {
    macros = new char *[numMacros];
    expansions = new char *[numMacros];
    i = 0;
    curMacro = macroHead->flink;
    while ( curMacro != macroHead ) {
      macros[i] = curMacro->macro;
      expansions[i] = curMacro->expansion;
      i++;
      curMacro = curMacro->flink;
    }
  }

  return 1;

}

int appContextClass::startApplication (
  int argc,
  char **argv,
  int _primaryServer )
{

  return startApplication( argc, argv, _primaryServer, 0, 0 );

}

extern "C" void scrolledWAcceleratorSupportInstall();

int appContextClass::startApplication (
  int argc,
  char **argv,
  int _primaryServer,
  int _oneInstance,
  int _convertOnly )
{

int stat, opStat;
activeWindowListPtr cur;
char *name, *envPtr;
char dspName[63+1], prefix[255+1], fname[255+1], msg[127+1];
fileListPtr curFile;
expStringClass expStr;
Atom wm_delete_window;
XTextProperty xtext;
char title[31+1], *pTitle;
int n;
Arg args[10];
XmString xmStr1;

  primaryServer = _primaryServer;
  oneInstance = _oneInstance;
  convertOnly = _convertOnly;

  envPtr = getenv("DISPLAY");
  if ( envPtr ) {
    strncpy( dspName, envPtr, 63 );
    dspName[63] = 0;
  }
  else {
    strcpy( dspName, ":0.0" );
  }

  name = argv[0];

  stat = getParams( argc, argv );
  if ( !( stat & 1 ) ) {
    switch ( stat ) {
    case 2:
      fprintf( stderr, appContextClass_str93 );
      goto err_return;
    case 4:
      fprintf( stderr, appContextClass_str94 );
      goto err_return;
    case 6:
      fprintf( stderr, appContextClass_str95 );
      goto err_return;
    case 8:
      fprintf( stderr, appContextClass_str96 );
      goto err_return;
    case -2:
      fprintf( stderr, appContextClass_str97 );
      goto err_return;
    }
err_return:
    fprintf( stderr, global_str55 );
    exitFlag = 1;
    return 0; // error
  }

  stat = initDeferredExecutionQueue();
  if ( !( stat & 1 ) ) {
    exitFlag = 1;
    return 0; // error
  }


  if ( strcmp( ctlPV, "" ) != 0 ) {

    initialConnection = 1;
    the_PV_Factory->clear_default_pv_type();
    ctlPvId = the_PV_Factory->create( ctlPV );
    if ( ctlPvId ) {
      ctlPvId->add_conn_state_callback( ctlPvMonitorConnection, this );
      ctlPvId->add_value_callback( ctlPvUpdate, this );
      usingControlPV = 1;
    }
    else {
      sprintf( msg, appContextClass_str99 );
      postMessage( msg );
    }

  }

  {

  int argCount = 3;
  char *args[3] = { "edm", global_str21, displayName };

    if ( strcmp( displayName, "" ) == 0 ) argCount = 1;

#if 0
    if ( executeOnOpen ) {
      appTop = XtVaAppInitialize( &app, "edm", NULL, 0, &argCount,
       args, NULL,
       XmNiconic, True,
       NULL );
      iconified = 1;
    }
    else {
      appTop = XtVaAppInitialize( &app, "edm", NULL, 0, &argCount,
       args, NULL,
       XmNiconic, False,
       XmNmappedWhenManaged, False,
       NULL );
      iconified = 0;
    }
#endif

#if 1
    if ( g_needXtInit ) {
      g_needXtInit = 0;
      XtToolkitInitialize();
    }

    app = XtCreateApplicationContext();

    /* install wheelmouse support */
    {
      String	fbr[]={
#if 0
        "edm*baseTranslations:#override\\n"
        "<Btn4Up>:\\n <Btn4Down>:\\n <Btn5Up>:\\n <Btn5Down>:\\n",
#endif
        "edm*XmScrolledWindow.VertScrollBar.accelerators:#override\\n"
        "~Shift<Btn4Up>:IncrementUpOrLeft(Up)\\n"
        "~Shift<Btn5Up>:IncrementDownOrRight(Down)\\n",
        "*XmScrolledWindow.HorScrollBar.accelerators:#override\\n"
        "Shift<Btn4Up>:IncrementUpOrLeft(Left)\\n"
        "Shift<Btn5Up>:IncrementDownOrRight(Right)\\n",
        0
      };
      scrolledWAcceleratorSupportInstall();
      XtAppSetFallbackResources(app, fbr);
    }

    for ( int i=0; i<argCount; i++ ) {
      if ( strcmp( args[i], "-display" ) == 0 ) {
        if ( i+1 < argCount ) {
          strncpy( dspName, args[i+1], 63 );
	  dspName[63] = 0;
	  break;
	}
      }
    }

    int result = 0;
    result = checkDisplay( dspName );
    if ( result ) {
      fprintf( stderr, appContextClass_str145, dspName );
      exitFlag = 1;
      return 0; // error
    }

    display = XtOpenDisplay( app, NULL, NULL, "edm", NULL, 0,
     &argCount, args );
    if ( !display ) {
      display = XtOpenDisplay( app, dspName, NULL, "edm", NULL, 0,
       &argCount, args );
      if ( !display ) {
        fprintf( stderr, appContextClass_str146 );
        exitFlag = 1;
        return 0; // error
      }
    }

    if ( executeOnOpen ) {
      appTop = XtVaAppCreateShell( NULL, "edm", applicationShellWidgetClass,
       display,
       XmNiconic, True,
       NULL );
      iconified = 1;
    }
    else {
      appTop = XtVaAppCreateShell( NULL, "edm", applicationShellWidgetClass,
       display,
       XmNiconic, False,
       XmNmappedWhenManaged, False,
       NULL );
      iconified = 0;
    }
#endif

  }

  {

    AppContextResourceRec rs;

    XtGetApplicationResources(
     appTop,
     &rs,
     resource_list, XtNumber( resource_list ),
     NULL, 0);

    entryFormX = rs.efX;
    entryFormY = rs.efY;
    entryFormW = rs.efW;
    entryFormY = rs.efH;
    largestH   = rs.llH;

  }

  this->createMainWindow();

  clipBd.clipbdInit( appTop );

  XtRealizeWidget( appTop );

  display = XtDisplay( appTop );

  strcpy( title, "edm " );
  Strncat( title, VERSION, 31 );
  pTitle = title;
  XStringListToTextProperty( &pTitle, 1, &xtext );
  XSetWMName( display, XtWindow(appTop), &xtext );
  XSetWMIconName( display, XtWindow(appTop), &xtext );
  XFree( xtext.value );

  processAllEvents( app, display );

  wm_delete_window = XmInternAtom( display, "WM_DELETE_WINDOW", False );
  XmAddWMProtocolCallback( appTop, wm_delete_window,
   exit_cb, (XtPointer) this );
  XtVaSetValues( appTop, XmNdeleteResponse, XmDO_NOTHING, NULL );

  displayH = XDisplayHeight( display, DefaultScreen(display) );
  displayW = XDisplayWidth( display, DefaultScreen(display) );
  //largestH = displayH;

  if (largestH <= 0) largestH += displayH;

  {
    char *envPtr = getenv( environment_str38 );
    if ( envPtr ) {
      char *err;
      int maxPropDialogH = strtol( envPtr, &err, 0 );
      if ( *err == '\0' ) {
        if ( maxPropDialogH > 0 ) {
          if ( maxPropDialogH < 400 ) maxPropDialogH = 400;
          if ( largestH > maxPropDialogH ) largestH = maxPropDialogH;
        }
      }
    }
  }

  msgBox.create( appTop, "msgbox", 0, 0, 50000, NULL, NULL );

  pvList.create( appTop, "pvlist", 20 );

  n = 0;
  //xmStr1 = XmStringCreateLocalized( "*.edl" );
  xmStr1 = XmStringCreateLocalized( activeWindowClass::defMask() );
  XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

  fileSelectFromPathBox = XmCreateFileSelectionDialog( appTop, "menuopenpathselect", args, n );

  XmStringFree( xmStr1 );

  XtAddCallback( fileSelectFromPathBox, XmNcancelCallback,
   app_fileSelectFromPathCancel_cb, (void *) this );
  XtAddCallback( fileSelectFromPathBox, XmNokCallback,
   app_fileSelectFromPathOk_cb, (void *) this );


  n = 0;
  //xmStr1 = XmStringCreateLocalized( "*.edl" );
  xmStr1 = XmStringCreateLocalized( activeWindowClass::defMask() );
  XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

  fileSelectBox = XmCreateFileSelectionDialog( appTop, "menuopenfileselect", args, n );

  XmStringFree( xmStr1 );

  XtAddCallback( fileSelectBox, XmNcancelCallback,
   app_fileSelectCancel_cb, (void *) this );
  XtAddCallback( fileSelectBox, XmNokCallback,
   app_fileSelectOk_cb, (void *) this );


  n = 0;
  xmStr1 = XmStringCreateLocalized( "*.xch" );
  XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

  importSelectBox = XmCreateFileSelectionDialog( appTop, "menuimportfileselect", args, n );

  XmStringFree( xmStr1 );

  XtAddCallback( importSelectBox, XmNcancelCallback,
   app_importSelectCancel_cb, (void *) this );
  XtAddCallback( importSelectBox, XmNokCallback,
   app_importSelectOk_cb, (void *) this );

  envPtr = getenv(environment_str2); // EDMFILES
  if ( envPtr ) {
    strncpy( prefix, envPtr, 255 );
    if ( prefix[strlen(prefix)-1] != '/' ) Strncat( prefix, "/", 255 );
  }
  else {
    strcpy( prefix, "/etc/edm/" );
  }

  envPtr = getenv(environment_str10); // EDMCOLORFILE
  if ( envPtr ) {

    strncpy( fname, envPtr, 255 );

  }
  else {

    strncpy( fname, prefix, 255 );
    Strncat( fname, "colors.list", 255 );

  }

  if ( privColorMap ) {
    ci.usePrivColorMap();
  }

  opStat = ci.initFromFile( app, display, appTop, fname );

  if ( strcmp( colormode, "rgb" ) == 0 ) {
    ci.useRGB();
  }
  else if ( strcmp( colormode, "RGB" ) == 0 ) {
    ci.useRGB();
  }

  if ( ci.colorModeIsRGB() ) {
    postMessage( appContextClass_str129 );
    postMessage( appContextClass_str130 );
  }

  if ( !( opStat & 1 ) ) {
    fprintf( stderr, appContextClass_str106 );
    exitFlag = 1;
    return 0; // error
  }

  if ( ci.majorVersion() == 3 ) {
    postMessage( appContextClass_str131 );
  }

  XSetWindowColormap( display, XtWindow(pvList.top()), ci.getColorMap() );

  postNote( appContextClass_str117 );

  processAllEvents( app, display );

  envPtr = getenv(environment_str11); // EDMFONTFILE
  if ( envPtr ) {

    strncpy( fname, envPtr, 255 );

  }
  else {

    strncpy( fname, prefix, 255 );
    Strncat( fname, "fonts.list", 255 );

  }

  opStat = fi.initFromFile( app, display, fname );

  msgDialogOpenCount = 20; // close note in applicationLoop function
  //closeNote();

  if ( !( opStat & 1 ) ) {
    fprintf( stderr, appContextClass_str107 );
    exitFlag = 1;
    return 0; // error
  }

  displayScheme.setAppCtx( this );
  displayScheme.loadDefault( &ci );

  curFile = fileHead->flink;
  while ( curFile != fileHead ) {

    cur = new activeWindowListType;
    //strcpy( cur->winName, "" );
    cur->requestDelete = 0;
    cur->requestActivate = 0;
    cur->requestActivateClear = 0;
    cur->requestReactivate = 0;
    cur->requestOpen = 0;
    cur->requestPosition = 0;
    cur->requestImport = 0;
    cur->requestRefresh = 0;
    cur->requestActiveRedraw = 0;
    cur->requestIconize = 0;

    if ( convertOnly ) {
      cur->requestConvertAndExit = 1;
    }
    else {
      cur->requestConvertAndExit = 0;
    }

    cur->blink = head->blink;
    head->blink->flink = cur;
    head->blink = cur;
    cur->flink = head;

    cur->node.create( this, NULL, 0, 0, 0, 0, numMacros, macros, expansions );
    cur->node.realize();
    cur->node.setGraphicEnvironment( &ci, &fi );

    cur->node.storeFileName( curFile->file );

    cur->requestOpen = 1;
    requestFlag++;

    if ( executeOnOpen ) {
      cur->requestActivate = 1;
      requestFlag++;
    }

    curFile = curFile->flink;

  }

  iconTestCount = 0;

  if ( !iconified ) XtMapWidget( appTop );

  return 1;

}

#define GETTING_INITIAL 1
#define GETTING_1ST_MACRO 2
#define GETTING_MACROS 3
#define GETTING_FILES 4

#define MAX_LOC_MACROS 20

void appContextClass::openFiles (
char *list
) {

activeWindowListPtr cur;
int i, doOpen;
unsigned int crc;
char *tk, *buf1, tmpMsg[255+1];
int locNumMacros;
char *locMacros[MAX_LOC_MACROS], *locExpansions[MAX_LOC_MACROS];
int state;
char *macTk, *macBuf, macTmp[255+1];
int stat;
char name[127+1], prefix[127+1];
char filePart[255+1], winNam[WINNAME_MAX+1];
int gotPosition, posx, posy;

  //fprintf( stderr, "list = [%s]\n", list );

  buf1 = NULL;
  strncpy( tmpMsg, list, 255 );
  tmpMsg[255] = 0;
  tk = strtok_r( tmpMsg, "|", &buf1 );
  tk = strtok_r( NULL, "|", &buf1 );
  tk = strtok_r( NULL, "|", &buf1 );
  tk = strtok_r( NULL, "|", &buf1 );
  tk = strtok_r( NULL, "|", &buf1 );
  tk = strtok_r( NULL, "|", &buf1 );

  locNumMacros = 0;
  state = GETTING_INITIAL;
  tk = strtok_r( NULL, "|", &buf1 );
  while ( tk ) {

    //fprintf( stderr, "state = %-d\n", state );
    //if ( tk ) {
    //  fprintf( stderr, "tk = [%s]\n", tk );
    //}
    //else {
    //  fprintf( stderr, "tk is null\n" );
    //}

    if ( state == GETTING_INITIAL ) {

      if ( strcmp( tk, global_str73 ) == 0 ) {
        tk = strtok_r( NULL, "|", &buf1 );
        tk = strtok_r( NULL, "|", &buf1 );
	if ( !tk ) return;
      }
      else if ( strcmp( tk, global_str93 ) == 0 ) {

        state = GETTING_FILES;
        tk = strtok_r( NULL, "|", &buf1 );
	if ( !tk ) return;

      }
      else if ( strcmp( tk, global_str19 ) == 0 ) {

        state = GETTING_1ST_MACRO;
        tk = strtok_r( NULL, "|", &buf1 );
	if ( !tk ) return;

      }
      else if ( tk[0] == '-' ) { //skip other options if present
        tk = strtok_r( NULL, "|", &buf1 );
        if ( !tk ) return;

      }
      else {

        state = GETTING_FILES;
        tk = strtok_r( NULL, "|", &buf1 );
	if ( !tk ) return;

      }

    }

    if ( state == GETTING_FILES ) {

      //fprintf( stderr, "Getting files\n" );

      //for ( i=0; i<locNumMacros; i++ ) {
      //  fprintf( stderr, "%s[%-d] = %s\n", locMacros[i], i, locExpansions[i] );
      //}

      if ( strcmp( tk, global_str93 ) != 0 ) {

        extractPosition( tk, filePart, 255, &gotPosition, &posx, &posy );

        extractWinName( filePart, 255, winNam, WINNAME_MAX );

        doOpen = 1;
        cur = head->flink;
        while ( cur != head ) {

          crc = 0;
          for ( i=0; i<locNumMacros; i++ ) {
            crc = updateCRC( crc, locMacros[i], strlen(locMacros[i]) );
            crc = updateCRC( crc, locExpansions[i], strlen(locExpansions[i]) );
          }

          stat = getFileName( name, filePart, 127 );
          stat = getFilePrefix( prefix, filePart, 127 );

          if ( ( strcmp( name, cur->node.displayName ) == 0 ) &&
               ( strcmp( prefix, cur->node.prefix ) == 0 ) &&
               ( crc == cur->node.crc ) && !cur->node.isEmbedded ) {

            doOpen = 0; // display is already open, just raise/deiconify it

            XMapWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
            XRaiseWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
            if ( gotPosition ) cur->node.move( posx, posy );

            break;

          }

          cur = cur->flink;

        }

        if ( doOpen ) {

          //fprintf( stderr, "Do open\n" );

          cur = new activeWindowListType;
          cur->setWinName( winNam );
	  //strncpy( cur->winName, winNam, WINNAME_MAX );
          //cur->winName[WINNAME_MAX] = 0;
          cur->requestDelete = 0;
          cur->requestActivate = 0;
          cur->requestActivateClear = 0;
          cur->requestReactivate = 0;
          cur->requestOpen = 0;
          cur->requestPosition = 0;
          cur->requestImport = 0;
          cur->requestRefresh = 0;
          cur->requestActiveRedraw = 0;
          cur->requestIconize = 0;
          cur->requestConvertAndExit = 0;

          cur->blink = head->blink;
          head->blink->flink = cur;
          head->blink = cur;
          cur->flink = head;

          cur->node.create( this, NULL, 0, 0, 0, 0, locNumMacros, locMacros,
           locExpansions );
          cur->node.realize();
          cur->node.setGraphicEnvironment( &ci, &fi );

	  if ( gotPosition ) {
	    cur->x = posx;
	    cur->y = posy;
	    cur->requestPosition = 1;
	  }

          cur->node.storeFileName( filePart );

          cur->requestOpen = 1;
          requestFlag++;

          cur->requestActivate = 1;
          requestFlag++;

        }

      }

      tk = strtok_r( NULL, "|", &buf1 );
      if ( !tk ) return;

    }

    if ( state == GETTING_1ST_MACRO ) {

      macBuf = NULL;
      strcpy( macTmp, tk );
      //fprintf( stderr, "getting 1st macro, macTmp = [%s]\n", macTmp );
      macTk = strtok_r( macTmp, "=,", &macBuf );
      if ( macTk ) {
        //fprintf( stderr, "getting 1st macro, sym = [%s]\n", macTk );
        locMacros[0] = macTk;
      }

      macTk = strtok_r( NULL, "=,", &macBuf );
      if ( macTk ) {
        //fprintf( stderr, "getting 1st macro, val = [%s]\n", macTk );
        locExpansions[0] = macTk;
        locNumMacros = 1;
        state = GETTING_MACROS;
      }
      else {
        locNumMacros = 0;
        state = GETTING_FILES;
        tk = strtok_r( NULL, "|", &buf1 );
	if ( !tk ) return;
      }

    }

    if ( state == GETTING_MACROS ) {

      macTk = strtok_r( NULL, "=,", &macBuf );
      if ( macTk ) {
        if ( locNumMacros >= MAX_LOC_MACROS ) {
          postMessage( appContextClass_str142 );
	  return;
	}
        //fprintf( stderr, "getting macros, sym = [%s]\n", macTk );
        locMacros[locNumMacros] = macTk;
      }
      
      macTk = strtok_r( NULL, "=,", &macBuf );
      if ( macTk ) {
        //fprintf( stderr, "getting macros, val = [%s]\n", macTk );
        locExpansions[locNumMacros] = macTk;
        locNumMacros++;
      }
      else {
        state = GETTING_FILES;
        tk = strtok_r( NULL, "|", &buf1 );
	if ( !tk ) return;
      }
      
    }

  }

}

void appContextClass::controlWinNames (
  char *cmd,
  char *list
) {

activeWindowListPtr cur, next;
int i, doOpen;
unsigned int crc;
char *tk, *buf1, tmpMsg[255+1];
int locNumMacros;
char *locMacros[MAX_LOC_MACROS], *locExpansions[MAX_LOC_MACROS];
int state;
char *macTk, *macBuf, macTmp[255+1];
int stat;
char name[127+1], prefix[127+1];
char filePart[255+1], winNam[WINNAME_MAX+1];
int gotPosition, posx, posy;
char controlCmd[31+1];

  strcpy( controlCmd, "" );

  buf1 = NULL;
  strncpy( tmpMsg, list, 255 );
  tmpMsg[255] = 0;
  tk = strtok_r( tmpMsg, "|", &buf1 );
  tk = strtok_r( NULL, "|", &buf1 );
  tk = strtok_r( NULL, "|", &buf1 );
  tk = strtok_r( NULL, "|", &buf1 );
  tk = strtok_r( NULL, "|", &buf1 );
  tk = strtok_r( NULL, "|", &buf1 );

  locNumMacros = 0;
  state = GETTING_INITIAL;
  tk = strtok_r( NULL, "|", &buf1 );
  while ( tk ) {

    //fprintf( stderr, "state = %-d\n", state );
    //if ( tk ) {
    //  fprintf( stderr, "tk = [%s]\n", tk );
    //}
    //else {
    //  fprintf( stderr, "tk is null\n" );
    //}

    if ( state == GETTING_INITIAL ) {

      if ( strcmp( tk, global_str73 ) == 0 ) {
        tk = strtok_r( NULL, "|", &buf1 );
        tk = strtok_r( NULL, "|", &buf1 );
	if ( !tk ) return;
      }
      else if ( 
        ( strcmp( tk, global_str128 ) == 0 ) ||
        ( strcmp( tk, global_str130 ) == 0 ) ||
        ( strcmp( tk, global_str132 ) == 0 ) ||
        ( strcmp( tk, global_str134 ) == 0 ) ||
        ( strcmp( tk, global_str136 ) == 0 ) ||
        ( strcmp( tk, global_str138 ) == 0 ) ||
        ( strcmp( tk, global_str140 ) == 0 ) ||
        ( strcmp( tk, global_str146 ) == 0 )
      ) {

        state = GETTING_FILES;
	strncpy( controlCmd, tk, 31 );
        controlCmd[31] = 0;
        tk = strtok_r( NULL, "|", &buf1 );
	if ( !tk ) return;

      }
      else if ( strcmp( tk, global_str19 ) == 0 ) {

        state = GETTING_1ST_MACRO;
        tk = strtok_r( NULL, "|", &buf1 );
	if ( !tk ) return;

      }
      else if ( tk[0] == '-' ) { //skip other options if present
        tk = strtok_r( NULL, "|", &buf1 );
        if ( !tk ) return;

      }
      else {

        state = GETTING_FILES;
        tk = strtok_r( NULL, "|", &buf1 );
	if ( !tk ) return;

      }

    }

    if ( state == GETTING_FILES ) {

      //fprintf( stderr, "Getting files\n" );

      //for ( i=0; i<locNumMacros; i++ ) {
      //  fprintf( stderr, "%s[%-d] = %s\n", locMacros[i], i, locExpansions[i] );
      //}

      if ( 
        ( strcmp( tk, global_str128 ) != 0 ) &&
        ( strcmp( tk, global_str130 ) != 0 ) &&
        ( strcmp( tk, global_str132 ) != 0 ) &&
        ( strcmp( tk, global_str134 ) != 0 ) &&
        ( strcmp( tk, global_str136 ) != 0 ) &&
        ( strcmp( tk, global_str138 ) != 0 ) &&
        ( strcmp( tk, global_str140 ) != 0 ) &&
        ( strcmp( tk, global_str146 ) != 0 )
      ) {

        extractPosition( tk, filePart, 255, &gotPosition, &posx, &posy );

        strncpy( winNam, filePart, WINNAME_MAX );
        winNam[WINNAME_MAX] = 0;

        doOpen = 1;
        cur = head->flink;
        while ( cur != head ) {

          crc = 0;
          for ( i=0; i<locNumMacros; i++ ) {
            crc = updateCRC( crc, locMacros[i], strlen(locMacros[i]) );
            crc = updateCRC( crc, locExpansions[i], strlen(locExpansions[i]) );
          }

          stat = getFileName( name, filePart, 127 );
          stat = getFilePrefix( prefix, filePart, 127 );

          next = cur->flink;

          if ( cur->winName ) {

            if ( strlen(winNam) > 0 ) {

              if ( cur->simpleMatch( winNam ) ) {

                if ( strcmp( controlCmd, global_str128 ) == 0 ) { // close
                  cur->clearWinName();         // clear this so no other
                                               // action may be taken
                  cur->node.returnToEdit( 1 );
                }
                else if ( strcmp( controlCmd, global_str130 ) == 0 ) { // move
                  if ( gotPosition ) {
                    cur->node.move( posx, posy );
                  }
                }
                else if ( strcmp( controlCmd, global_str132 ) == 0 ) { // raise
                  if ( gotPosition ) {
                    cur->node.move( posx, posy );
                  }
                  XRaiseWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
                }
                else if ( strcmp( controlCmd, global_str134 ) == 0 ) { // lower
                  if ( gotPosition ) {
                    cur->node.move( posx, posy );
                  }
                  XLowerWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
                }
                else if ( strcmp( controlCmd, global_str136 ) == 0 ) { // iconify
                  XtVaSetValues( cur->node.topWidgetId(),
                   XmNiconic, True,
                   NULL );
                  if ( gotPosition ) {
                    cur->node.move( posx, posy );
                  }
                }
                else if ( strcmp( controlCmd, global_str138 ) == 0 ) { // deiconify
                  if ( gotPosition ) {
                    cur->node.move( posx, posy );
                  }
                  XtVaSetValues( cur->node.topWidgetId(),
                   XmNiconic, False,
                   NULL );
                }
                else if ( strcmp( controlCmd, global_str140 ) == 0 ) { // snapshot
                  if ( gotPosition ) {
                    cur->node.move( posx, posy );
                  }
                  // snapshot
                  XRaiseWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
                  processAllEventsWithSync( app, display );
                  cur->node.smartDrawAllActive();
                  cur->node.refreshActive();
                  processAllEventsWithSync( app, display );
		  {
		    char name[255+1];
		    int winid = (int) XtWindow(cur->node.top);
                    snprintf( name, 255, "xwd -id %-d -out /tmp/", winid );
		    Strncat( name, cur->node.displayNameForSym, 255 );
		    Strncat( name, ".xwd", 255 );
		    if ( debugMode() ) fprintf( stderr, "%s\n", name );
                    system( name );
		  }
                }

              }

            }

	  }

          cur = next;

        }

      }

      tk = strtok_r( NULL, "|", &buf1 );
      if ( !tk ) return;

    }

    if ( state == GETTING_1ST_MACRO ) {

      macBuf = NULL;
      strcpy( macTmp, tk );
      //fprintf( stderr, "getting 1st macro, macTmp = [%s]\n", macTmp );
      macTk = strtok_r( macTmp, "=,", &macBuf );
      if ( macTk ) {
        //fprintf( stderr, "getting 1st macro, sym = [%s]\n", macTk );
        locMacros[0] = macTk;
      }

      macTk = strtok_r( NULL, "=,", &macBuf );
      if ( macTk ) {
        //fprintf( stderr, "getting 1st macro, val = [%s]\n", macTk );
        locExpansions[0] = macTk;
        locNumMacros = 1;
        state = GETTING_MACROS;
      }
      else {
        locNumMacros = 0;
        state = GETTING_FILES;
        tk = strtok_r( NULL, "|", &buf1 );
	if ( !tk ) return;
      }

    }

    if ( state == GETTING_MACROS ) {

      macTk = strtok_r( NULL, "=,", &macBuf );
      if ( macTk ) {
        if ( locNumMacros >= MAX_LOC_MACROS ) {
          postMessage( appContextClass_str142 );
	  return;
	}
        //fprintf( stderr, "getting macros, sym = [%s]\n", macTk );
        locMacros[locNumMacros] = macTk;
      }
      
      macTk = strtok_r( NULL, "=,", &macBuf );
      if ( macTk ) {
        //fprintf( stderr, "getting macros, val = [%s]\n", macTk );
        locExpansions[locNumMacros] = macTk;
        locNumMacros++;
      }
      else {
        state = GETTING_FILES;
        tk = strtok_r( NULL, "|", &buf1 );
	if ( !tk ) return;
      }
      
    }

  }

}

void appContextClass::openInitialFiles ( void ) {

fileListPtr curFile;
activeWindowListPtr cur;
int i, doOpen;
unsigned int crc;
char tmpName[127+1], prefix[127+1];

  curFile = fileHead->flink;
  while ( curFile != fileHead ) {

    //fprintf( stderr, "[%s]\n", curFile->file );

    doOpen = 1;
    cur = head->flink;
    while ( cur != head ) {

      crc = 0;
      for ( i=0; i<numMacros; i++ ) {
        crc = updateCRC( crc, macros[i], strlen(macros[i]) );
        crc = updateCRC( crc, expansions[i], strlen(expansions[i]) );
      }

      getFileName( tmpName, curFile->file, 127 );
      getFilePrefix( prefix, curFile->file, 127 );

      //fprintf( stderr, "crc = %-ud\n", crc );
      //fprintf( stderr, "cur->node.crc = %-ud\n", cur->node.crc );
      //fprintf( stderr, "curFile->file = %s\n", tmpName );
      //fprintf( stderr, "cur->node.displayName = %s\n", cur->node.displayName );
      //fprintf( stderr, "cur->node.isEmbedded = %-d\n", cur->node.isEmbedded );

      if ( ( strcmp( tmpName, cur->node.displayName ) == 0 ) &&
           ( strcmp( prefix, cur->node.prefix ) == 0 ) &&
           ( crc == cur->node.crc ) && !cur->node.isEmbedded ) {

	doOpen = 0; // display is already open, just raise/deiconify it

        XMapWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
        XRaiseWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );

	break;

      }

      cur = cur->flink;

    }

    if ( doOpen ) {

      //fprintf( stderr, "Do open\n" );

      cur = new activeWindowListType;
      //strcpy( cur->winName, "" );
      cur->requestDelete = 0;
      cur->requestActivate = 0;
      cur->requestActivateClear = 0;
      cur->requestReactivate = 0;
      cur->requestOpen = 0;
      cur->requestPosition = 0;
      cur->requestImport = 0;
      cur->requestRefresh = 0;
      cur->requestActiveRedraw = 0;
      cur->requestIconize = 0;
      cur->requestConvertAndExit = 0;

      cur->blink = head->blink;
      head->blink->flink = cur;
      head->blink = cur;
      cur->flink = head;

      cur->node.create( this, NULL, 0, 0, 0, 0, numMacros, macros,
       expansions );
      cur->node.realize();
      cur->node.setGraphicEnvironment( &ci, &fi );

      cur->node.storeFileName( curFile->file );

      cur->requestOpen = 1;
      requestFlag++;

      if ( executeOnOpen ) {
        cur->requestActivate = 1;
        requestFlag++;
      }

    }

    curFile = curFile->flink;

  }

}

int appContextClass::addActWin (
  char *name,
  int x,
  int y,
  int numMacs,
  char **syms,
  char **exps )
{

activeWindowListPtr cur;

  cur = new activeWindowListType;
  //strcpy( cur->winName, "" );
  cur->requestDelete = 0;
  cur->requestActivate = 0;
  cur->requestActivateClear = 0;
  cur->requestReactivate = 0;
  cur->requestOpen = 0;
  cur->requestPosition = 0;
  cur->requestImport = 0;
  cur->requestRefresh = 0;
  cur->requestActiveRedraw = 0;
  cur->requestIconize = 0;
  cur->requestConvertAndExit = 0;

  cur->blink = head->blink;
  head->blink->flink = cur;
  head->blink = cur;
  cur->flink = head;

  cur->node.create( this, NULL, x, y, 0, 0, numMacs, syms, exps );
  cur->node.realize();
  cur->node.setGraphicEnvironment( &ci, &fi );

  cur->node.storeFileName( name );

  cur->requestOpen = 1;
  requestFlag++;

  if ( executeOnOpen ) {
    cur->requestActivate = 1;
    requestFlag++;
  }

  return 1;

}

void appContextClass::applicationLoop ( void ) {

int stat, nodeCount, actionCount, iconNodeCount,
 iconActionCount, n;
activeWindowListPtr cur, next;
char msg[127+1];

  if ( epc.printEvent() ) {

    if ( epc.printDefFileError() ) {
      postMessage( epc.errorMsg() );
    }

    if ( epc.printCmdReady() ) {
      epc.doPrint();
    }

    if ( epc.printFinished() ) {
      postNote( appContextClass_str138 );
      msgDialogOpenCount = 20;
    }

    if ( epc.printFailure() ) {
      postMessage( epc.errorMsg() );
    }

  }

  if ( msgDialogOpenCount > 0 ) {
    if ( msgDialogOpenCount == 1 ) {
      closeNote();
    }
    msgDialogOpenCount--;
  }

  if ( reloadFlag == 2 ) {
    refreshAll();
    reloadFlag = 0;
  }
  else if ( reloadFlag == 1 ) {
    reloadAll();
    reloadFlag = 2;
  }
  else if ( reloadFlag == 3 ) {
    reloadSelected();
    reloadFlag = 2;
  }

  if ( requestFlag ) {

    cur = head->flink;
    while ( cur != head ) {
      if ( !cur->requestDelete ) {
        if ( cur->requestRefresh ) {
          cur->requestRefresh = 0;
          if ( requestFlag > 0 ) requestFlag--;
          cur->node.refreshActive();
        }
        if ( cur->requestActiveRedraw ) {
          cur->requestActiveRedraw = 0;
          if ( requestFlag > 0 ) requestFlag--;
          cur->node.smartDrawAllActive();
        }
      }
      cur = cur->flink;
    }

    nodeCount = iconNodeCount = actionCount = iconActionCount = 0;
    // traverse list and delete nodes so marked
    cur = head->flink;
    while ( cur != head ) {
      next = cur->flink;
      nodeCount++;
      if ( cur->requestDelete ) {

        if ( cur->node.numChildren == 0 ) {

          actionCount++;
          iconActionCount++;
          cur->blink->flink = cur->flink;
          cur->flink->blink = cur->blink;
          removeAllDeferredExecutionQueueNode( &cur->node );

          if ( cur->node.parent ) {
            if ( cur->node.parent->numChildren ) {
              (cur->node.parent->numChildren)--;
	    }
          }

          delete cur;
          if ( requestFlag > 0 ) requestFlag--;

	}

      }
      cur = next;
    }

    /* if all windows have been removed then deiconify main window */
    if ( iconNodeCount == iconActionCount ) {
      if ( !usingControlPV ) deiconifyMainWindow();
    }

    cur = head->flink;
    while ( cur != head ) {
      if ( cur->requestOpen ) {
        cur->requestOpen = 0;
        if ( cur->requestImport ) {
          cur->requestImport = 0;
          stat = cur->node.import();
        }
        else {
          if ( cur->requestPosition ) {
            cur->requestPosition = 0;
            stat = cur->node.load( cur->x, cur->y );
          }
          else {
            stat = cur->node.load();
            if ( cur->requestConvertAndExit ) {
              cur->requestConvertAndExit = 0;
	      fprintf( stderr, "Converting file [%s]\n", cur->node.fileName );
              cur->node.save( cur->node.fileName );
              exitFlag = 1;
	    }
          }
        }
        if ( !reloadFlag ) {
          //XtMapWidget( XtParent( cur->node.drawWidgetId() ) );
          XtMapWidget( cur->node.topWidgetId() );
	}

        if ( !( stat & 1 ) ) {
          sprintf( msg, appContextClass_str108, cur->node.fileName );
          postMessage( msg );
          cur->requestDelete = 1;
          cur->requestActivate = 0;
          cur->requestReactivate = 0;
          XtUnmanageChild( cur->node.drawWidgetId() );
          //XtUnmapWidget( XtParent( cur->node.drawWidgetId() ) );
          XtUnmapWidget( cur->node.topWidgetId() );
          if ( requestFlag > 0 ) requestFlag--;
          if ( cur->requestActivate == 1 ) {
            cur->requestActivate = 0;
            if ( requestFlag > 0 ) requestFlag--;
          }
          if ( requestFlag == 0 ) requestFlag = 1; // because req Del = 1
        }
        else {
          if ( requestFlag > 0 ) requestFlag--;
          if ( cur->requestActivate == 1 ) cur->node.setNoRefresh();
          if ( cur->requestReactivate == 1 ) cur->node.setNoRefresh();
        }

      }
      cur = cur->flink;
    }

    cur = head->flink;
    while ( cur != head ) {
      if ( cur->requestActivate == 1 ) cur->requestActivate = 2;
      if ( cur->requestReactivate == 1 ) cur->requestReactivate = 2;
      cur = cur->flink;
    }

    processAllEvents( app, display );

    cur = head->flink;
    while ( cur != head ) {
      if ( !cur->requestDelete ) {
        if ( cur->requestActivate == 3 ) {
          if ( requestFlag > 0 ) requestFlag--;
          cur->requestActivate = 0;
          if ( cur->requestActivateClear ) {
            cur->requestActivateClear = 0;
            cur->node.clearActive();
            cur->node.refreshActive();
	  }
	}
      }
      cur = cur->flink;
    }

    nodeCount = iconNodeCount = actionCount = iconActionCount = 0;
    cur = head->flink;
    while ( cur != head ) {
      nodeCount++;
      iconNodeCount++;
      if ( cur->node.isActive() ) {
        actionCount++;
        iconActionCount++;
      }
      if ( !cur->requestDelete ) {
        if ( cur->requestActivate == 2 ) {
          cur->requestActivate = 3;
          cur->node.execute();

          stat = pend_io( 1.0 );
          pend_event( 0.00001 );
          cur->node.processObjects();

          stat = pend_io( 1.0 );
          pend_event( 0.00001 );
          processAllEvents( app, display );

          actionCount++;
          iconActionCount++;
          cur->node.setRefresh();
          cur->node.refreshActive();

          // need to iconify?
          if ( cur->requestIconize ) {
            cur->requestIconize = 0;
            XIconifyWindow( display,
             XtWindow(XtParent(cur->node.executeWidget)),
             DefaultScreen(display) );
            cur->node.isIconified = 1;
          }

        }
        if ( cur->requestReactivate == 2 ) {
          cur->requestReactivate = 0;
          cur->node.reexecute();
          actionCount++;
          if ( requestFlag > 0 ) requestFlag--;
          cur->node.setRefresh();
          cur->node.refreshActive();
        }
      }

      cur->node.doMinCopy();

      cur = cur->flink;

    }

    if ( nodeCount ) atLeastOneOpen = 1;
    if ( exitOnLastClose && atLeastOneOpen ) {
      if ( nodeCount == 0 ) {
        pathList.popdown();
        exitFlag = 1;
        if ( diagnosticMode() ) logDiagnostic( "Program exit requested\n" );
      }
    }

    processAllEvents( app, display );

    /* if all windows have been activated then iconify main window */
    if ( ( iconNodeCount == iconActionCount ) && ( iconNodeCount != 0 ) ) {
      if ( !usingControlPV ) iconifyMainWindow();
    }
    else {
      if ( !usingControlPV ) deiconifyMainWindow();
    }

  }

  processDeferredExecutionQueue();

  n = 0;
  cur = head->flink;
  while ( cur != head ) {

    int stuffToDo;

    n++;

    // invoke the executeDeferred function for all activeGraphicClass
    // objects (display elements) for a given activeWindowClass object
    // (display screen)
    stuffToDo = cur->node.processObjects();

    if ( !ignoreIconic() ) {

      if ( iconTestCount > 10 ) { // periodically, check if iconified
        XtVaGetValues( cur->node.topWidgetId(),
         XmNiconic, &cur->node.isIconified,
         NULL );
      }

    }
    else {

      if ( cur->node.isIconified ) cur->node.isIconified = 0;

    }

    if ( !( n % 30 ) ) {
      stat = pend_io( 10.0 );
      pend_event( 0.00001 );
    }
    processAllEvents( app, display );

    cur = cur->flink;

  }

  if ( iconTestCount++ > 10 ) {
    iconTestCount = 0;
  }

  stat = pend_io( 10.0 );
  //pend_event( 0.00001 );

  processAllEvents( app, display );

  cur = head->flink;
  while ( cur != head ) {
    cur->node.doMinCopy();
    cur = cur->flink;
  }

  raiseMessageWindow();

}

XtAppContext appContextClass::appContext ( void )
{

  return app;

}

Widget appContextClass::fileSelectFromPathBoxWidgetId ( void )
{

  return fileSelectFromPathBox;

}

Widget appContextClass::fileSelectBoxWidgetId ( void )
{

  return fileSelectBox;

}

Widget appContextClass::importSelectBoxWidgetId ( void )
{

  return importSelectBox;

}

void appContextClass::setErrMsgPrefix (
  char *prefix
) {

  errMsgPrefix = new char[strlen(prefix)+1];
  strcpy( errMsgPrefix, prefix );

}

void appContextClass::useStdErr (
  int flag
) {

  useStdErrFlag = flag;

}

void appContextClass::postMessage (
  char *msg )
{

  if ( useStdErrFlag ) {
    if ( errMsgPrefix ) {
      fprintf( stderr, errMsgPrefix );
    }
    fprintf( stderr, msg );
    int l = strlen( msg );
    if ( !l || ( msg[l-1] != '\n' ) ) {
      fprintf( stderr, "\n" );
    }
  }
  else {
    msgBox.addText( msg );
  }

}

void appContextClass::raiseMessageWindow ( void )
{

  msgBox.raise();

}

void appContextClass::iconifyMainWindow ( void ) {

  return;

  if ( !iconified ) {
    XIconifyWindow( display, XtWindow(appTop), DefaultScreen(display) );
    iconified = 1;
  }

}

void appContextClass::deiconifyMainWindow ( void ) {

  return;

  if ( iconified ) {
    XUnmapWindow( display, XtWindow(appTop) );
    XMapWindow( display, XtWindow(appTop) );
    iconified = 0;
  }

}

int appContextClass::setProperty (
  char *winId,
  char *id,
  char *property,
  char *value )
{

int stat;
activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( strcmp( winId, cur->node.id ) == 0 ) {
      stat = cur->node.setProperty( id, property, value );
      return stat;
    }
    cur = cur->flink;
  }

  return 0;

}

int appContextClass::setProperty (
  char *winId,
  char *id,
  char *property,
  double *value )
{

int stat;
activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( strcmp( winId, cur->node.id ) == 0 ) {
      stat = cur->node.setProperty( id, property, value );
      return stat;
    }
    cur = cur->flink;
  }

  return 0;

}

int appContextClass::setProperty (
  char *winId,
  char *id,
  char *property,
  int *value )
{

int stat;
activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( strcmp( winId, cur->node.id ) == 0 ) {
      stat = cur->node.setProperty( id, property, value );
      return stat;
    }
    cur = cur->flink;
  }

  return 0;

}

int appContextClass::getProperty (
  char *winId,
  char *id,
  char *property,
  int bufSize,
  char *value )
{

int stat;
activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( strcmp( winId, cur->node.id ) == 0 ) {
      stat = cur->node.getProperty( id, property, bufSize, value );
      return stat;
    }
    cur = cur->flink;
  }

  return 0;

}

int appContextClass::getProperty (
  char *winId,
  char *id,
  char *property,
  double *value )
{

int stat;
activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( strcmp( winId, cur->node.id ) == 0 ) {
      stat = cur->node.getProperty( id, property, value );
      return stat;
    }
    cur = cur->flink;
  }

  return 0;

}

int appContextClass::getProperty (
  char *winId,
  char *id,
  char *property,
  int *value )
{

int stat;
activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( strcmp( winId, cur->node.id ) == 0 ) {
      stat = cur->node.getProperty( id, property, value );
      return stat;
    }
    cur = cur->flink;
  }

  return 0;

}

void appContextClass::reopenUserLib ( void ) {

  if ( strcmp( userLib, "" ) != 0 ) {
    userLibObject.openUserLibrary( userLib );
  }

}

void appContextClass::xSynchronize (
  int onoff
) {

  postMessage( "X Sync mode is on" );
  XSynchronize( display, (Bool) onoff );

}

void appContextClass::exitProgram ( void ) {

  exit_cb ( NULL, this, NULL );

}

void appContextClass::findTop ( void ) {

  //XUnmapWindow( display, XtWindow(appTop) );
  XMapWindow( display, XtWindow(appTop) );
  XRaiseWindow( display, XtWindow(appTop) );

}

void appContextClass::postNote ( 
  char *msg ) {

int _x, _y;

  _x = XDisplayWidth( display, DefaultScreen(display) ) / 2;
  _y = XDisplayHeight( display, DefaultScreen(display) ) / 2;

  // printf( "postNote - msgDialog = %-X\n", (int) &msgDialog );

  msgDialog.popup( msg, _x, _y );

}

void appContextClass::closeNote ( void ) {

  // printf( "closeNote - msgDialog = %-X\n", (int) &msgDialog );

  msgDialog.popdown();

}

int appContextClass::numScreens ( void ) {

  // return number of open screens

int count = 0;
activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    count++;
    cur = cur->flink;
  }

  return count;

}

void appContextClass::performShutdown (
  FILE *f ) {

  if ( !saveContextOnExit ) {
    shutdownFilePtr = f;
    saveContextOnExit = 1;
    pathList.popdown();
    exitFlag = 1;
    if ( diagnosticMode() ) logDiagnostic( "Program exit requested\n" );
    //abort_cb( (Widget) NULL, (XtPointer) this, (XtPointer) NULL );
  }

}

int appContextClass::getShutdownFlag ( void )
{

  return shutdownFlag;

}

int appContextClass::renderImages ( void ) {

  return renderImagesFlag;

}

void appContextClass::setRenderImages (
  int flag
) {

  renderImagesFlag = flag;

}

int appContextClass::openCheckPointScreen (
  char *screenName,
  int x,
  int y,
  int icon,
  int noEdit,
  int numCheckPointMacros,
  char *checkPointMacros
) {

activeWindowListPtr cur;
int n, stat;
char *newMacros[100+1];
char *newValues[100+1];

  //fprintf( stderr, "appContextClass::openCheckPointScreen\n" );

  if ( numCheckPointMacros ) {
    stat = parseSymbolsAndValues( checkPointMacros, 100,
     newMacros, newValues, &n );
  }
  else {
    n = 0;
  }

  //fprintf( stderr, "screenName = [%s]\n", screenName );
  //fprintf( stderr, "x = %-d\n", x );
  //fprintf( stderr, "y = %-d\n", y );
  //fprintf( stderr, "icon = %-d\n", icon );
  //fprintf( stderr, "numCheckPointMacros = %-d\n", numCheckPointMacros );
  //fprintf( stderr, "checkPointMacros = [%s]\n", checkPointMacros );

  //fprintf( stderr, "found %-d macros\n", n );
  //for ( i=0; i<n; i++ ) {
  //  fprintf( stderr, "sym=[%s]  val=[%s]\n", newMacros[i], newValues[i] );
  //}

  cur = new activeWindowListType;
  //strcpy( cur->winName, "" );

  addActiveWindow( cur );

  if ( n > 0 ) {
    if ( noEdit ) {
      cur->node.createNoEdit( this, NULL, 0, 0, 0, 0,
       n, newMacros, newValues );
    }
    else {
      cur->node.create( this, NULL, 0, 0, 0, 0,
       n, newMacros, newValues );
    }
  }
  else {
    if ( noEdit ) {
      cur->node.createNoEdit( this, NULL, 0, 0, 0, 0,
       0, NULL, NULL );
    }
    else {
      cur->node.create( this, NULL, 0, 0, 0, 0,
       0, NULL, NULL );
    }
  }

  cur->node.realize();

  cur->node.setGraphicEnvironment( &ci, &fi );

  cur->node.storeFileName( screenName );

  cur->node.noRaise = 1;
  cur->node.isIconified = True;

  if ( icon ) {
    stat = openActivateIconifiedActiveWindow( &cur->node, x, y );
  }
  else {
    stat = openActivateActiveWindow( &cur->node, x, y );
  }

  processAllEventsWithSync( app, display );

  return 1;

}

int appContextClass::okToExit ( void ) {

activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {

    if ( cur->node.windowState != AWC_COMPLETE_DEACTIVATE ) {
      if ( !cur->node.okToDeactivate() ) {
        return 0;
      }
    }

    cur = cur->flink;

  }

  return 1;

}

void appContextClass::addActions (
  XtActionsRec *actions, // actions must be a unique static address
  Cardinal n
) {

actionsPtr curAct;

  thread_lock( actionsLock );

  // see if actions have already been added
  curAct = actHead->flink;
  while ( curAct ) {
    if ( curAct->key == (void *) actions ) {
      thread_unlock( actionsLock );
      return; // actions have already been added for this app context
    }
    curAct = curAct->flink;
  }

  // actions have not yet been added

  // alloc node
  curAct = new actionsType;
  curAct->key = (void *) actions;

  // add to list
  actTail->flink = curAct;
  actTail = curAct;
  actTail->flink = NULL;

  XtAppAddActions( app, actions, n );

  thread_unlock( actionsLock );

}

void appContextClass::showEnv ( void ) {

char *envPtr, text[255+1];

  snprintf( text, 255, "Environment:" );
  postMessage( text );

  envPtr = getenv( environment_str1 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str1, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str1 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str2 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str2, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str2 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str3 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str3, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str3 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str4 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str4, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str4 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str5 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str5, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str5 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str6 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str6, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str6 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str7 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str7, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str7 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str8 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str8, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str8 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str9 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str9, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str9 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str10 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str10, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str10 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str11 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str11, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str11 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str12 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str12, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str12 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str13 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str13, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str13 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str14 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str14, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str14 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str15 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str15, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str15 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str16 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str16, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str16 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str17 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str17, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str17 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str18 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str18, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str18 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str19 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str19, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str19 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str20 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str20, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str20 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str21 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str21, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str21 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str22 ); //EDMCOMMENTS
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str22, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str22 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str23 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str23, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str23 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str24 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str24, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str24 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str25 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str25, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str25 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str26 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str26, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str26 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( "CALC_ENV" );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", "CALC_ENV", envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", "CALC_ENV" );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str32 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str32, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str32 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str33 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str33, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str33 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str34 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str34, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str34 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str36 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str36, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str36 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str37 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str37, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str37 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str38 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str38, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str38 );
  }
  text[255] = 0;
  postMessage( text );

  // site specific vars

  snprintf( text, 255, " " );
  postMessage( text );
  snprintf( text, 255, "  (Site Related)" );
  postMessage( text );

  envPtr = getenv( environment_str40 ); // "EDMRDDHS"
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str40, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str40 );
  }
  text[255] = 0;
  postMessage( text );


// diagnostic vars

  snprintf( text, 255, " " );
  postMessage( text );
  snprintf( text, 255, "  (Diagnostic)" );
  postMessage( text );

  envPtr = getenv( environment_str27 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str27, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str27 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str28 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str28, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str28 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str29 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str29, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str29 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str30 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str30, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str30 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str31 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str31, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str31 );
  }
  text[255] = 0;
  postMessage( text );

  envPtr = getenv( environment_str35 );
  if ( envPtr ) {
    snprintf( text, 255, "  %s=[%s]", environment_str35, envPtr );
  }
  else {
    snprintf( text, 255, "  %s=[]", environment_str35 );
  }
  text[255] = 0;
  postMessage( text );

  snprintf( text, 255, " " );
  postMessage( text );

}

Widget appContextClass::apptop ( void ) {

  return appTop;

}

int appContextClass::screenAlreadyUp(unordered_map<string, string> &sigs, 
  char *edlname, 
  char **macros, 
  char **values, 
  int nmacros)
{
  char signature[MAX_LINE];
  strcpy(signature, edlname);
  for (int i=0; i<nmacros; i++) {
    strcat(signature, macros[i]);
    strcat(signature, values[i]);
  }
  if (sigs.count(signature)) {
//    printf("%s is already up\n", edlname);
    return 1;
  } else {
//    printf("need to start %s\n", edlname);
    return 0;
  }
}

void appContextClass::closeAllButHead() 
{
//  printf("calling closeAllButHead\n");
    activeWindowListPtr cur = this->head->flink;
    while ( cur != this->head ) {
      if ( cur->node.okToDeactivate() ) {
        cur->node.returnToEdit( 1 );
      } else {
        cur->node.closeAnyDeferred( 20 );
      }
      cur = cur->flink;
    }
}

void appContextClass::getScreenSignatures(unordered_map<string, string> &sigs)
{
  char signature[MAX_LINE];
  // move to end of node list and get signatures of existing screens
  activeWindowListPtr cur = this->head->flink;
  while ( cur != this->head ) {
    if ( blank( cur->node.displayName ) ) {
      strcpy(signature, "");
    } else {
      int macro_count = 0;
      strcpy(signature, cur->node.displayName);
      for (int i=0; i<cur->node.numMacros; i++ ) {
        strcat(signature, cur->node.macros[i]);
        strcat(signature, cur->node.expansions[i]);
        macro_count++;
      }
      if (!cur->node.isEmbedded) {
        if (!macro_count) sigs[signature] = "nomacros";
        sigs[signature] = signature;
      }
    }
    cur = cur->flink;
  }
}

int appContextClass::screenConfigOk(FILE *fp) {
  char line[MAX_LINE];
  
  while (fgets(line, MAX_LINE, fp) != NULL) { 
    if (!strncmp(line, confOk, confOkCount)) return 1;
  }  
  return 0;
}


int appContextClass::getCfgDirectory(char *pfilter, char *pmsg) {
  char *home;
  
  home = getenv(environment_str39);     //EDMSCREENCFG
  if ( home ) {
    if (strlen(home) >= MAX_DIR) {
        sprintf(pmsg, appContextClass_str100, "EDMSCREENCFG");  // string too long: %s
        return -1;
    }
    int len = strlen(home);
    if (*(home+len) != '/') {
      strcpy(home+len, "/");
    }
    sprintf(pfilter, "%s*.edmcfg", home);
//    printf("using EDMSCREENCFG - filter %s\n", pfilter);
  } else {
    home = getenv("HOME");
    if ( home ) {
      if (strlen(home) >= MAX_DIR) {
          sprintf(pmsg, appContextClass_str100, "HOME");  // string too long: %s
          this->postMessage(pmsg);
          return -1;
      }
      int len = strlen(home);
      if (*(home+len) != '/') {
        strcpy(home+len, "/");
      }
      sprintf(pfilter, "%s/.edm/*.edmcfg", home);
    } else {
      sprintf(pfilter, "/tmp/*.edmcfg");
    }
  }
  return 0;
}

int appContextClass::writeConfig(char *fname) {
  FILE *fp = fopen(fname, "w");
  if (!fp) {
   return -1;
  } 
  int i = 0;
  float scale = 1.0;   // to be added to display node later
  activeWindowListPtr cur;

//  printf("writeConfig - %s\n", fname);
  cur = this->head->flink;
  fprintf(fp, "%s %s\n", this->getConfOk(), fname );
  while ( cur != this->head ) {
    if (!cur->node.isEmbedded) {
      if ( blank( cur->node.displayName ) ) {
        fprintf(fp, "%s", appContextClass_str27 );
      } else {
        fprintf(fp, "%s", cur->node.displayName );
        fprintf(fp, " x=%d y=%d", cur->node.x, cur->node.y);
        fprintf(fp, " scale=%f", scale);
      
        for ( i=0; i<cur->node.numMacros; i++ ) {
          if (i == 0) fprintf(fp, "  %s=%s", cur->node.macros[i], cur->node.expansions[i] );
          else  fprintf(fp, ",%s=%s", cur->node.macros[i], cur->node.expansions[i] );
        }
      }
      fprintf(fp, "\n");
    }
    cur = cur->flink;
  }
  fclose(fp);
}

char *appContextClass::checkCfgName(char *fname) {
  char *ps0 = strrchr(fname, '/');
  char *ps = strrchr(ps0+1, '.');
  int len = (ps ? ps - fname : strlen(fname));
//  printf("check extension - len %d fname %s length %d\n", len, fname, strlen(fname));
  if (len == strlen(fname)) {         // no . in file name
    strcpy(fname+len, ".edmcfg");     // force extension
  } else if (strcmp(ps, ".edmcfg")) {
    strcpy(fname+strlen(fname), ".edmcfg");
  }
  return fname;
}
