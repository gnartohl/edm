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

#define __bindings_c 1
#include "bindings.h"
#include "thread.h"
#include "utility.h"

objBindingClass::objBindingClass ( void ) {

int needToOpenDll, i, index, comment;
char *more, *envPtr, *tk, *tk1, *c1, *c2, *c3, *error;
char prefix[127+1], fname[127+1], line[255+1], buf[255+1], buf1[255+1],
 rawLine[255+1];
FILE *f;

//   fprintf( stderr, "objBindingClass::objBindingClass\n" );

  max = 0;

  if ( !num ) {

    envPtr = getenv(environment_str3);
    if ( envPtr ) {
      strncpy( prefix, envPtr, 127 );
      if ( prefix[strlen(prefix)-1] != '/' ) Strncat( prefix, "/", 127 );
    }
    else {
      strcpy( prefix, "/etc/edm/" );
    }

    strncpy( fname, prefix, 127 );
    Strncat( fname, "edmObjects", 127 );

    f = fopen( fname, "r" );
    if ( !f ) {
      fprintf( stderr, objBindingClass_str1, fname );
      return;
    }

    more = fgets( line, 255, f );
    if ( more ) {
      c1 = NULL;
      tk = strtok_r( line, "\n", &c1 );
      num = atol( tk );
      if ( num <= 0 ) {
        fprintf( stderr, objBindingClass_str2, fname );
        return;
      }
    }
    else {
      fprintf( stderr, objBindingClass_str2, fname );
      return;
    }

    names = new char *[num+3]; /* add one more for symbols, asymbols, and
                                    one more for dynSymbols */
    if ( !names ) {
      fprintf( stderr, objBindingClass_str3 );
      exit(-1);
    }

    classNames = new char *[num+3]; /* add one more for symbols, asymbols, and
                                         one more for dynSymbols */
    if ( !classNames ) {
      fprintf( stderr, objBindingClass_str3 );
      exit(-1);
    }

    param = new char *[num+3]; /* add one more for symbols, asymbols, and
                                         one more for dynSymbols */
    if ( !param ) {
      fprintf( stderr, objBindingClass_str3 );
      exit(-1);
    }

    types = new char *[num+3]; /* add one more for symbols, asymbols, and
                                         one more for dynSymbols */
    if ( !types ) {
      fprintf( stderr, objBindingClass_str3 );
      exit(-1);
    }

    dllHandle = new void *[num];
    if ( !dllHandle ) {
      fprintf( stderr, objBindingClass_str3 );
      exit(-1);
    }

    dllName = new char *[num];
    if ( !dllName ) {
      fprintf( stderr, objBindingClass_str3 );
      exit(-1);
    }

    index = 0;
    do {

      more = fgets( rawLine, 255, f );
      if ( more ) {

        expandEnvVars( rawLine, 255, line );

        strncpy( buf, line, 255 );

        comment = 0;
        c2 = NULL;
        tk = strtok_r( buf, " \t\n", &c2 );

        if ( !tk ) {
          comment = 1;
        }
        else if ( tk[0] == '#' ) {
          comment = 1;
        }

        if ( !comment ) { /* not an empty line or comment */

          if ( index < num ) {

            c1 = NULL;
            tk = strtok_r( line, " \t\n", &c1 );
            if ( !tk ) {
              fprintf( stderr, objBindingClass_str4 );
              exit(-1);
            }

            classNames[index] = new char[strlen(tk)+1];
            strcpy( classNames[index], tk );

            // allow a paramter string (no white space) to be supplied
            // in the form object:param

            strncpy( buf1, tk, 255 );

            c3 = NULL;
            tk1 = strtok_r( buf1, ":", &c3 );
            tk1 = strtok_r( NULL, ":", &c3 );
            if ( tk1 ) {
              param[index] = new char[strlen(tk1)+1];
              strcpy( param[index], tk1 );
	    }
            else {
              param[index] = new char[1];
              strcpy( param[index], "" );
	    }

            tk = strtok_r( NULL, " \t\n", &c1 );
            if ( !tk ) {
              fprintf( stderr, objBindingClass_str4 );
              exit(-1);
            }
            dllName[index] = new char[strlen(tk)+1];
            strcpy( dllName[index], tk );

            tk = strtok_r( NULL, " \t\n", &c1 );
            if ( !tk ) {
              fprintf( stderr, objBindingClass_str4 );
              exit(-1);
            }
            types[index] = new char[strlen(tk)+1];
            strcpy( types[index], tk );

            tk = strtok_r( NULL, "\n", &c1 );
            if ( !tk ) {
              fprintf( stderr, objBindingClass_str4 );
              exit(-1);
            }
            names[index] = new char[strlen(tk)+1];
            strcpy( names[index], tk );

          }

          index++;

        }

      }

    } while ( more );

    fclose( f );

    if ( index != num ) {
      fprintf( stderr, objBindingClass_str5, fname );
      exit(-1);
    }

    for ( index=0; index<num; index++ ) {

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

    /* add text for symbols, note that names and types have been
       allocated to contain an additional element */

    classNames[num] = new char[strlen("activeSymbolClass")+1];
    strcpy( classNames[num], "activeSymbolClass" );

    param[num] = new char[1];
    strcpy( param[num], "" );

    names[num] = new char[strlen(global_str1)+1];
    strcpy( names[num], global_str1 );

    types[num] = new char[strlen(global_str2)+1];
    strcpy( types[num], global_str2 );

    /* add text for asymbols, note that names and types have been
       allocated to contain an additional element */

    classNames[num+1] = new char[strlen("aniSymbolClass")+1];
    strcpy( classNames[num+1], "aniSymbolClass" );

    param[num+1] = new char[1];
    strcpy( param[num+1], "" );

    names[num+1] = new char[strlen(global_str142)+1];
    strcpy( names[num+1], global_str142 );

    types[num+1] = new char[strlen(global_str2)+1];
    strcpy( types[num+1], global_str2 );

    /* add text for dynSymbols, note that names and types have been
       allocated to contain an additional element */

    classNames[num+2] = new char[strlen("activeDynSymbolClass")+1];
    strcpy( classNames[num+2], "activeDynSymbolClass" );

    param[num+2] = new char[1];
    strcpy( param[num+2], "" );

    names[num+2] = new char[strlen(global_str4)+1];
    strcpy( names[num+2], global_str4 );

    types[num+2] = new char[strlen(global_str3)+1];
    strcpy( types[num+2], global_str3 );

  }

  cur_index = 0;

  max = num + 3;

}

objBindingClass::~objBindingClass ( void ) {

}

char *objBindingClass::firstObjName (
  char *objType )
{

int i;

  cur_index = max;

  // find first matching type
  for ( i=0; i<max; i++ ) {
    if ( strcmp( objType, types[i] ) == 0 ) {
      cur_index = i;
      goto found_index;
    }
  }

  return NULL;

found_index:

  return classNames[cur_index];

}

char *objBindingClass::nextObjName (
  char *objType )
{

int i;

  // find first matching type
  for ( i=cur_index+1; i<max; i++ ) {
    if ( strcmp( objType, types[i] ) == 0 ) {
      cur_index = i;
      goto found_index;
    }
  }

  return NULL;

found_index:

  return classNames[cur_index];

}

activeGraphicClass *objBindingClass::createNew (
  char *oneClassName )
{

typedef void *(*VPFUNC)( void );
VPFUNC func;
activeGraphicClass *cur;
int i;
char name[127+1], *error, buf[127+1], *tk;

//    fprintf( stderr, "objBindingClass::createNew - name = [%s]\n", oneClassName );

  if ( strcmp( oneClassName, "activeGroupClass" ) == 0 ) {

    cur = new activeGroupClass;
    return cur;

  }
  else if ( strcmp( oneClassName, "activeSymbolClass" ) == 0 ) {

    cur = new activeSymbolClass;
    return cur;

  }
  else if ( strcmp( oneClassName, "aniSymbolClass" ) == 0 ) {

    cur = new aniSymbolClass;
    return cur;

  }
  else if ( strcmp( oneClassName, "anaSymbolClass" ) == 0 ) { // old name

    cur = new aniSymbolClass;
    return cur;

  }
  else if ( strcmp( oneClassName, "activeDynSymbolClass" ) == 0 ) {

    cur = new activeDynSymbolClass;
    return cur;

  }

#if 0
  else if ( strcmp( oneClassName, "activeLineClass" ) == 0 ) {

    cur = new activeLineClass;
    return cur;

  }

  else if ( strcmp( oneClassName, "activeRectangleClass" ) == 0 ) {

    cur = new activeRectangleClass;
    return cur;

  }

  else if ( strcmp( oneClassName, "activeCircleClass" ) == 0 ) {

    cur = new activeCircleClass;
    return cur;

  }

  else if ( strcmp( oneClassName, "activeXTextClass" ) == 0 ) {

    cur = new activeXTextClass;
    return cur;

  }

  else if ( strcmp( oneClassName, "activeMeterClass" ) == 0 ) {

    cur = new activeMeterClass;
    return cur;

  }

  else if ( strcmp( oneClassName, "activeBarClass" ) == 0 ) {

    cur = new activeBarClass;
    return cur;

  }

  else if ( strcmp( oneClassName, "activeMessageBoxClass" ) == 0 ) {

    cur = new activeMessageBoxClass;
    return cur;

  }

  else if ( strcmp( oneClassName, "activeXTextDspClass" ) == 0 ) {

    cur = new activeXTextDspClass;
    return cur;

  }

  else if ( strcmp( oneClassName, "activeSliderClass" ) == 0 ) {

    cur = new activeSliderClass;
    return cur;

  }

  else if ( strcmp( oneClassName, "activeButtonClass" ) == 0 ) {

    cur = new activeButtonClass;
    return cur;

  }

  else if ( strcmp( oneClassName, "activeMenuButtonClass" ) == 0 ) {

    cur = new activeMenuButtonClass;
    return cur;

  }

  else if ( strcmp( oneClassName, "activeMessageButtonClass" ) == 0 ) {

    cur = new activeMessageButtonClass;
    return cur;

  }

  else if ( strcmp( oneClassName, "activeExitButtonClass" ) == 0 ) {

    cur = new activeExitButtonClass;
    return cur;

  }

  else if ( strcmp( oneClassName, "menuMuxClass" ) == 0 ) {

    cur = new menuMuxClass;
    return cur;

  }

  else if ( strcmp( oneClassName, "relatedDisplayClass" ) == 0 ) {

    cur = new relatedDisplayClass;
    return cur;

  }

#endif

  for ( i=0; i<max; i++ ) {

    if ( strcmp( oneClassName, classNames[i] ) == 0 ) {

      strncpy( buf, oneClassName, 127 );
      tk = strtok( buf, ":" );

      strcpy( name, "create_" );
      //Strncat( name, classNames[i], 127 );
      Strncat( name, tk, 127 );
      Strncat( name, "Ptr", 127 );

//        fprintf( stderr, "func name = [%s]\n", name );

      func = (VPFUNC) dlsym( dllHandle[i], name );
      if ((error = dlerror()) != NULL)  {
        fputs(error, stderr);
        fputs( "\n", stderr );
        return NULL;
      }

      cur = (activeGraphicClass *) (*func)();
      cur->setCreateParam( param[i] );

      return cur;

    }

  }

  return NULL;

}

activeGraphicClass *objBindingClass::clone (
  char *oneClassName,
  activeGraphicClass *source )
{

typedef void *(*VPFUNC)( void * );
VPFUNC func;
activeGraphicClass *cur;
int i;
char name[127+1], *error, buf[127+1], *tk;

//    fprintf( stderr, "In objBindingClass::clone, name = [%s]\n", oneClassName );

  if ( strcmp( oneClassName, "activeGroupClass" ) == 0 ) {

    cur = new activeGroupClass( (activeGroupClass *) source );
    return cur;

  }
  else if ( strcmp( oneClassName, "activeSymbolClass" ) == 0 ) {

    cur = new activeSymbolClass( (activeSymbolClass *) source );
    return cur;

  }
  else if ( strcmp( oneClassName, "aniSymbolClass" ) == 0 ) {

    cur = new aniSymbolClass( (aniSymbolClass *) source );
    return cur;

  }
  else if ( strcmp( oneClassName, "anaSymbolClass" ) == 0 ) { // old name

    cur = new aniSymbolClass( (aniSymbolClass *) source );
    return cur;

  }
  else if ( strcmp( oneClassName, "activeDynSymbolClass" ) == 0 ) {

    cur = new activeDynSymbolClass( (activeDynSymbolClass *) source );
    return cur;

  }

#if 0

  else if ( strcmp( oneClassName, "activeLineClass" ) == 0 ) {

    cur = new activeLineClass( (activeLineClass *) source );
    return cur;

  }

  else if ( strcmp( oneClassName, "activeRectangleClass" ) == 0 ) {

    cur = new activeRectangleClass( (activeRectangleClass *) source );
    return cur;

  }

  else if ( strcmp( oneClassName, "activeCircleClass" ) == 0 ) {

    cur = new activeCircleClass( (activeCircleClass *) source );
    return cur;

  }

  else if ( strcmp( oneClassName, "activeXTextClass" ) == 0 ) {

    cur = new activeXTextClass( (activeXTextClass *) source );
    return cur;

  }

  else if ( strcmp( oneClassName, "activeMeterClass" ) == 0 ) {

    cur = new activeMeterClass( (activeMeterClass *) source );
    return cur;

  }

  else if ( strcmp( oneClassName, "activeBarClass" ) == 0 ) {

    cur = new activeBarClass( (activeBarClass *) source );
    return cur;

  }

  else if ( strcmp( oneClassName, "activeMessageBoxClass" ) == 0 ) {

    cur = new activeMessageBoxClass( (activeMessageBoxClass *) source );
    return cur;

  }

  else if ( strcmp( oneClassName, "activeXTextDspClass" ) == 0 ) {

    cur = new activeXTextDspClass( (activeXTextDspClass *) source );
    return cur;

  }

  else if ( strcmp( oneClassName, "activeSliderClass" ) == 0 ) {

    cur = new activeSliderClass( (activeSliderClass *) source );
    return cur;

  }

  else if ( strcmp( oneClassName, "activeButtonClass" ) == 0 ) {

    cur = new activeButtonClass( (activeButtonClass *) source );
    return cur;

  }

  else if ( strcmp( oneClassName, "activeMenuButtonClass" ) == 0 ) {

    cur = new activeMenuButtonClass( (activeMenuButtonClass *) source );
    return cur;

  }

  else if ( strcmp( oneClassName, "activeMessageButtonClass" ) == 0 ) {

    cur = new activeMessageButtonClass( (activeMessageButtonClass *) source );
    return cur;

  }

  else if ( strcmp( oneClassName, "activeExitButtonClass" ) == 0 ) {

    cur = new activeExitButtonClass( (activeExitButtonClass *) source );
    return cur;

  }

  else if ( strcmp( oneClassName, "menuMuxClass" ) == 0 ) {

    cur = new menuMuxClass( (menuMuxClass *) source );
    return cur;

  }

  else if ( strcmp( oneClassName, "relatedDisplayClass" ) == 0 ) {

    cur = new relatedDisplayClass( (relatedDisplayClass *) source );
    return cur;

  }

#endif

  for ( i=0; i<max; i++ ) {

    if ( strcmp( oneClassName, classNames[i] ) == 0 ) {

      strncpy( buf, oneClassName, 127 );
      tk = strtok( buf, ":" );

      strcpy( name, "clone_" );
      //Strncat( name, classNames[i], 127 );
      Strncat( name, tk, 127 );
      Strncat( name, "Ptr", 127 );

      func = (VPFUNC) dlsym( dllHandle[i], name );
      if ((error = dlerror()) != NULL)  {
        fputs(error, stderr);
        fputs( "\n", stderr );
        return NULL;
      }

      cur = (activeGraphicClass *) (*func)( (void *) source );
      return cur;

    }

  }

  return NULL;

}

char *objBindingClass::getOjbName (
  int i )
{

  if ( i >= max ) return NULL;
  return names[i];

}

char *objBindingClass::getOjbType (
  int i )
{

  if ( i >= max ) return NULL;
  return types[i];

}

char *objBindingClass::getNameFromClass (
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

  if ( strcmp( className, "activeGroupClass" ) == 0 ) {
    return groupName;
  }

  return NULL;

}
