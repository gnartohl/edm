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

#define __ulBindings_c 1
#include "ulBindings.h"
#include "thread.h"

ulBindingClass::ulBindingClass ( void )
{

  strcpy( dllName, "" );
  dllHandle = NULL;

}

ulBindingClass::~ulBindingClass ( void ) {

}

int ulBindingClass::openUserLibrary (
  char *libName )
{

char *envPtr, *error;
char prefix[127+1];
int stat;

  if ( dllHandle ) {
    stat = dlclose( dllHandle );
    if ((error = dlerror()) != NULL)  {
      fputs(error, stderr);
      fputs( "\n", stderr );
      return 0;
    }
  }

  dllHandle = NULL;

  envPtr = getenv(environment_str6);
  if ( envPtr ) {
    strncpy( prefix, envPtr, 127 );
    if ( prefix[strlen(prefix)-1] != '/' ) Strncat( prefix, "/", 127 );
  }
  else {
    strcpy( prefix, "" );
  }

  strncpy( dllName, prefix, 127 );
  Strncat( dllName, libName, 127 );

  dllHandle = dlopen( dllName, RTLD_LAZY );
  if ((error = dlerror()) != NULL)  {
    fputs(error, stderr);
    fputs( "\n", stderr );
    return 0;
  }

  return 1;

}

IPFUNC ulBindingClass::getIntFunc (
  char *funcName )
{

IPFUNC func;
char *error;

//fprintf( stderr, "ulBindingClass::getFunc, func name = [%s]\n", funcName );

  if ( !dllHandle ) return NULL;

  func = (IPFUNC) dlsym( dllHandle, funcName );
  if ((error = dlerror()) != NULL)  {
    fputs(error, stderr);
    fputs( "\n", stderr );
    return NULL;
  }

  return func;

}

VPFUNC ulBindingClass::getFunc (
  char *funcName )
{

VPFUNC func;
char *error;

//    fprintf( stderr, "ulBindingClass::getFunc, func name = [%s]\n", funcName );

  if ( !dllHandle ) return NULL;

  func = (VPFUNC) dlsym( dllHandle, funcName );
  if ((error = dlerror()) != NULL)  {
    fputs(error, stderr);
    fputs( "\n", stderr );
    return NULL;
  }

  return func;

}

RULEFUNC ulBindingClass::getRuleFunc (
  char *funcName )
{

RULEFUNC func;
char *error;

// fprintf( stderr, "ulBindingClass::getRuleFunc, func name = [%s]\n", funcName );

  if ( !dllHandle ) return NULL;

  func = (RULEFUNC) dlsym( dllHandle, funcName );
  if ((error = dlerror()) != NULL)  {
    fputs(error, stderr);
    fputs( "\n", stderr );
    return NULL;
  }

  return func;

}
