//  edm - extensible display manager

//  Copyright (C) 1999 John W. Sinclair

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

#define __pv_bindings_c 1

#include <stdlib.h>
#include <string.h>

#include "pvBindings.h"
#include "thread.h"
//#include "utility.h"

//  contents of file edmPvObjects

//  numberOfPvObjects
//  className1 dllName userVisibleName
//  className2 dllName userVisibleName
//  	.
//  	.
//  	.
//  classNameN dllName userVisibleName

static int blank (
  char *string )
{

unsigned int i, l;

  l = strlen(string);
  if ( !l ) return 1;

  for ( i=0; i<l; i++ ) {
    if ( !isspace( (int) string[i] ) ) return 0;
  }

  return 1;

}

static char *expandEnvVars (
  char *inStr,
  int maxOut,
  char *outStr
) {

  // expands all environment vars of the form $(envVar) found in inStr
  // and sends the expanded string to outStr

int i, ii, iii=0, inL, state, bufOnHeap;
char *ptr, stackBuf[255+1];
char *buf;

static const int DONE = -1;
static const int FINDING_DOLLAR = 1;
static const int FINDING_LEFT_PAREN = 2;
static const int FINDING_RIGHT_PAREN = 3;

  if ( !inStr || !outStr || ( maxOut < 1 ) ) return NULL;

  inL = strlen( inStr );
  if ( inL < 1 ) return NULL;

  if ( maxOut > 255 ) {
    buf = new char[maxOut+1];
    bufOnHeap = 1;
  }
  else {
    buf = stackBuf;
    bufOnHeap = 0;
  }

  state = FINDING_DOLLAR;
  strcpy( outStr, "" );
  i = ii = 0;
  while ( state != DONE ) {

    switch ( state ) {

    case FINDING_DOLLAR:

      if ( i >= inL ) {
        state = DONE;
        break;
      }

      if ( inStr[i] == '\n' ) {
        state = DONE;
      }
      else if ( inStr[i] == '$' ) {
        state = FINDING_LEFT_PAREN;
      }
      else {
        if ( ii >= maxOut ) goto limitErr; // out string too big
        outStr[ii] = inStr[i];
        ii++;
      }

      break;

    case FINDING_LEFT_PAREN:

      if ( i >= inL ) goto syntaxErr; // never found '(' after '$'
      if ( inStr[i] == '\n' ) goto syntaxErr; // never found '(' after '$'
      if ( inStr[i] != '(' ) goto syntaxErr; // char after '$' was not '('
      strcpy( buf, "" );
      iii = 0;
      state = FINDING_RIGHT_PAREN;

      break;

    case FINDING_RIGHT_PAREN:

      if ( i >= inL ) goto syntaxErr; // never found ')'
      if ( inStr[i] == '\n' ) goto syntaxErr; // never found ')'

      if ( inStr[i] == ')' ) {

	// add terminating 0
        if ( iii >= maxOut ) goto syntaxErr; // env var value too big
        buf[iii] = 0;

        // translate and output env var value using buf

        if ( !blank( buf ) ) {

          ptr = getenv( buf );
          if ( ptr ) {

            for ( iii=0; iii<(int) strlen(ptr); iii++ ) {
              if ( ii > maxOut ) goto limitErr;
              outStr[ii] = ptr[iii];
              ii++;
	    }

	  }

	}

        state = FINDING_DOLLAR;

      }
      else {

        if ( iii >= maxOut ) goto syntaxErr; // env var value too big
        buf[iii] = inStr[i];
        iii++;

      }

      break;

    }

    i++;

  }

// normalReturn

  // add terminating 0
  if ( ii > maxOut ) goto limitErr;
  outStr[ii] = 0;

  if ( bufOnHeap ) delete [] buf;

  return outStr;

syntaxErr:

  fprintf( stderr, "Syntax error in env var reference\n" );
  if ( bufOnHeap ) delete [] buf;
  return NULL;

limitErr:

  fprintf( stderr, "Parameter size limit exceeded in env var reference\n" );
  if ( bufOnHeap ) delete [] buf;
  return NULL;

}

pvBindingClass::pvBindingClass ( void ) {

int needToOpenDll, i, index, comment;
char *more, *envPtr, *tk, *error;
char prefix[127+1], fname[127+1], line[255+1], buf[255+1], rawLine[255+1];
FILE *f;

//   fprintf( stderr, "pvBindingClass::pvBindingClass\n" );

  max = 0;

  if ( !num ) {

    strcpy( pvOptionMenuList, "" );

    envPtr = getenv(environment_str4);
    if ( envPtr ) {
      strncpy( prefix, envPtr, 127 );
      if ( prefix[strlen(prefix)-1] != '/' ) Strncat( prefix, "/", 127 );
    }
    else {
      strcpy( prefix, "/etc/edm/" );
    }

    strncpy( fname, prefix, 127 );
    Strncat( fname, "edmPvObjects", 127 );

    f = fopen( fname, "r" );
    if ( !f ) {
      fprintf( stderr, pvBindingClass_str1, fname );
      return;
    }

    more = fgets( line, 255, f );
    if ( more ) {
      tk = strtok( line, "\n" );
      num = atol( tk );
      if ( num <= 0 ) {
        fprintf( stderr, pvBindingClass_str2, fname );
        return;
      }
    }
    else {
      fprintf( stderr, pvBindingClass_str3, fname );
      return;
    }

    names = new char *[num];
    if ( !names ) {
      fprintf( stderr, pvBindingClass_str4 );
      exit(-1);
    }

    classNames = new char *[num];
    if ( !classNames ) {
      fprintf( stderr, pvBindingClass_str5 );
      exit(-1);
    }

    dllHandle = new void *[num];
    if ( !dllHandle ) {
      fprintf( stderr, pvBindingClass_str6 );
      exit(-1);
    }

    dllName = new char *[num];
    if ( !dllName ) {
      fprintf( stderr, pvBindingClass_str7 );
      exit(-1);
    }

    index = 0;
    do {

      more = fgets( rawLine, 255, f );
      if ( more ) {

        expandEnvVars( rawLine, 255, line );

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

          if ( index < num ) {

            tk = strtok( line, " \t\n" );
            if ( !tk ) {
              fprintf( stderr, pvBindingClass_str8 );
              exit(-1);
            }
            classNames[index] = new char[strlen(tk)+1];
            strcpy( classNames[index], tk );

            tk = strtok( NULL, " \t\n" );
            if ( !tk ) {
              fprintf( stderr, pvBindingClass_str9 );
              exit(-1);
            }
            dllName[index] = new char[strlen(tk)+1];
            strcpy( dllName[index], tk );

//              tk = strtok( NULL, " \t\n" );
//              if ( !tk ) {
//                fprintf( stderr, pvBindingClass_str10 );
//                exit(-1);
//              }

            tk = strtok( NULL, "\n" );
            if ( !tk ) {
              fprintf( stderr, pvBindingClass_str11 );
              exit(-1);
            }
            names[index] = new char[strlen(tk)+1];
            strcpy( names[index], tk );

            if ( strcmp( pvOptionMenuList, "" ) == 0 )
              Strncat( pvOptionMenuList, tk, 255 );
            else {
              Strncat( pvOptionMenuList, "|", 255 );
              Strncat( pvOptionMenuList, tk, 255 );
	    }

          }

          index++;

        }

      }

    } while ( more );

    fclose( f );

    if ( index != num ) {
      fprintf( stderr, pvBindingClass_str12, fname );
      exit(-1);
    }

    for ( index=0; index<num; index++ ) {

       //fprintf( stderr, "\nclassNames = [%s]\n", classNames[index] );
       //fprintf( stderr, "names = [%s]\n", names[index] );
       //fprintf( stderr, "dllName = [%s]\n", dllName[index] );

      needToOpenDll = 1;
      for ( i=0; i<index; i++ ) {
        if ( strcmp( dllName[index], dllName[i] ) == 0 ) {
          needToOpenDll = 0;
          dllHandle[index] = dllHandle[i];
          break;
	}
      }

      if ( needToOpenDll ) {

        if ( strcmp( dllName[index], "!" ) != 0 ) {

//           fprintf( stderr, "Open %s\n", dllName[index] );

          dllHandle[index] = dlopen( dllName[index], RTLD_LAZY );
          if ((error = dlerror()) != NULL)  {
            fputs(error, stderr);
            fputs( "\n", stderr );
            exit(1);
          }

        }
        else {

          dllHandle[index] = NULL;

        }

      }

    }

  }

  cur_index = 0;
  max = num;

}

pvBindingClass::~pvBindingClass ( void ) {

}

char *pvBindingClass::firstPvName ( void )
{

  cur_index = 0;
  if ( cur_index >= max ) return NULL;

  return classNames[cur_index];

}

char *pvBindingClass::nextPvName ( void )
{

  cur_index++;
  if ( cur_index >= max ) return NULL;

  return classNames[cur_index];

}

class ProcessVariable *pvBindingClass::createNew (
  const char *oneClassName,
  const char *PV_name )
{

typedef void *(*VPFUNC)( const char *PV_name );
VPFUNC func;
ProcessVariable *cur;
int i;
char name[127+1], *error;

  // fprintf( stderr, "pvBindingClass::createNew - name = [%s]\n", oneClassName );

  for ( i=0; i<max; i++ ) {

    if ( strcmp( oneClassName, classNames[i] ) == 0 ) {

      strcpy( name, "create_" );
      Strncat( name, classNames[i], 127 );
      Strncat( name, "Ptr", 127 );

      //fprintf( stderr, "func name = [%s]\n", name );

      func = (VPFUNC) dlsym( dllHandle[i], name );
      if ((error = dlerror()) != NULL)  {
        fputs(error, stderr);
        fputs( "\n", stderr );
        return NULL;
      }

// fprintf( stderr, "1\n" );
      cur = (ProcessVariable *) (*func)( PV_name );
// fprintf( stderr, "2\n" );
      return cur;

    }

  }

  return NULL;

}

char *pvBindingClass::getPvName (
  int i )
{

  if ( i >= max ) return NULL;
  return names[i];

}

char *pvBindingClass::getPvClassName (
  int i )
{

  if ( i >= max ) return NULL;
  return classNames[i];

}

char *pvBindingClass::getNameFromClass (
  char *className )
{

int i, l1, l2;

  l1 = strlen(className);

  for ( i=0; i<max; i++ ) {

    l2 = strlen(classNames[i]);

    if ( l1 == l2 ) {
      if ( strcmp( className, classNames[i] ) == 0 ) {
        return names[i];
      }
    }

  }

  return NULL;

}

char *pvBindingClass::getClassFromName (
  char *name )
{

int i, l1, l2;

  l1 = strlen(name);

  for ( i=0; i<max; i++ ) {

    l2 = strlen(names[i]);

    if ( l1 == l2 ) {
      if ( strcmp( name, names[i] ) == 0 ) {
        return classNames[i];
      }
    }

  }

  return NULL;

}

int pvBindingClass::getClassNum (
  char *className )
{

int i, l1, l2;

  l1 = strlen(className);

  for ( i=0; i<max; i++ ) {

    l2 = strlen(classNames[i]);

    if ( l1 == l2 ) {
      if ( strcmp( className, classNames[i] ) == 0 ) {
        return i;
      }
    }

  }

  return -1;

}

int pvBindingClass::getNameNum (
  char *name )
{

int i, l1, l2;

  l1 = strlen(name);

  for ( i=0; i<max; i++ ) {

    l2 = strlen(names[i]);

    if ( l1 == l2 ) {
      if ( strcmp( name, names[i] ) == 0 ) {
        return i;
      }
    }

  }

  return -1;

}

void pvBindingClass::getOptionMenuList (
  char *list,
  int listSize,
  int *_num )
{

  strncpy( list, pvOptionMenuList, listSize );
  *_num = num;

  // fprintf( stderr, "pvOptionMenuList = [%s]\n", pvOptionMenuList );
  // fprintf( stderr, "list = [%s]\n", list );

}

int pvBindingClass::pend_io (
  double sec
) {

char *error;
int i;
static int init=1;
typedef int (*FUNC1)( double );
static FUNC1 func = NULL;

  if ( init ) {

    init = 0;

    for ( i=0; i<max; i++ ) {

      if ( strcmp( "EPICS", classNames[i] ) == 0 ) {

        func = (FUNC1) dlsym( dllHandle[i], "epics_pend_io" );
        if ((error = dlerror()) != NULL)  {
          fputs(error, stderr);
          fputs( "\n", stderr );
          return -1;
        }

        return (*func)( sec );

      }

    }

    return 1;

  }
  else {

    if ( func ) {
      return (*func)( sec );
    }
    else {
      return 1;
    }

  }

}

int pvBindingClass::pend_event (
  double sec
) {

char *error;
int i;
static int init=1;
typedef int (*FUNC1)( double );
static FUNC1 func = NULL;
static THREAD_HANDLE h = NULL;

  if ( init ) {

    init = 0;

    for ( i=0; i<max; i++ ) {

      if ( strcmp( "EPICS", classNames[i] ) == 0 ) {

        func = (FUNC1) dlsym( dllHandle[i], "epics_pend_event" );
        if ((error = dlerror()) != NULL)  {
          fputs(error, stderr);
          fputs( "\n", stderr );
          return -1;
        }

        return (*func)( sec );

      }

    }

    thread_create_handle( &h, NULL );
    if ( h ) thread_delay( h, sec );
    return 1;

  }
  else {

    if ( func ) {
      return (*func)( sec );
    }
    else {
      if ( h ) thread_delay( h, sec );
      return 1;
    }

  }

}

void pvBindingClass::task_exit ( void ) {

char *error;
int i;
static int init=1;
typedef void (*FUNC1)( void );
static FUNC1 func = NULL;

  if ( init ) {

    init = 0;

    for ( i=0; i<max; i++ ) {

      if ( strcmp( "EPICS", classNames[i] ) == 0 ) {

        func = (FUNC1) dlsym( dllHandle[i], "epics_task_exit" );
        if ((error = dlerror()) != NULL)  {
          fputs(error, stderr);
          fputs( "\n", stderr );
          return;
        }

        (*func)();

      }

    }

  }
  else {

    if ( func ) {
      (*func)();
    }

  }

}
