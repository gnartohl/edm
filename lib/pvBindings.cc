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
#include "utility.h"

//  contents of file edmPvObjects

//  numberOfPvObjects
//  className1 dllName userVisibleName
//  className2 dllName userVisibleName
//  	.
//  	.
//  	.
//  classNameN dllName userVisibleName

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

//       fprintf( stderr, "\nclassNames = [%s]\n", classNames[index] );
//       fprintf( stderr, "names = [%s]\n", names[index] );
//       fprintf( stderr, "dllName = [%s]\n", dllName[index] );

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

pvClass *pvBindingClass::createNew (
  char *oneClassName )
{

typedef void *(*VPFUNC)( void );
VPFUNC func;
pvClass *cur;
int i;
char name[127+1], *error;

  // fprintf( stderr, "pvBindingClass::createNew - name = [%s]\n", oneClassName );

  for ( i=0; i<max; i++ ) {

    if ( strcmp( oneClassName, classNames[i] ) == 0 ) {

      strcpy( name, "create_" );
      Strncat( name, classNames[i], 127 );
      Strncat( name, "Ptr", 127 );

      // fprintf( stderr, "func name = [%s]\n", name );

      func = (VPFUNC) dlsym( dllHandle[i], name );
      if ((error = dlerror()) != NULL)  {
        fputs(error, stderr);
        fputs( "\n", stderr );
        return NULL;
      }

// fprintf( stderr, "1\n" );
      cur = (pvClass *) (*func)();
// fprintf( stderr, "2\n" );
      return cur;

    }

  }

  return NULL;

}

pvClass *pvBindingClass::clone (
  char *oneClassName,
  pvClass *source )
{

typedef void *(*VPFUNC)( void * );
VPFUNC func;
pvClass *cur;
int i;
char name[127+1], *error;

  // fprintf( stderr, "In pvBindingClass::clone, name = [%s]\n", oneClassName );

  for ( i=0; i<max; i++ ) {

    if ( strcmp( oneClassName, classNames[i] ) == 0 ) {

      strcpy( name, "clone_" );
      Strncat( name, classNames[i], 127 );
      Strncat( name, "Ptr", 127 );

      func = (VPFUNC) dlsym( dllHandle[i], name );
      if ((error = dlerror()) != NULL)  {
        fputs(error, stderr);
        fputs( "\n", stderr );
        return NULL;
      }

      cur = (pvClass *) (*func)( (void *) source );
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
