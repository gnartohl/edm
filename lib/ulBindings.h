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

#ifndef __ulBindings_h
#define __ulBindings_h 1

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include "act_grf.h"
#include "ulBindings.str"
#include "environment.str"

typedef void (*VPFUNC)( void *ptr );
typedef int (*RULEFUNC)( void *classPtr, int arraySize, void *valArray );
typedef int (*IPFUNC)( void *ptr );

class ulBindingClass {

private:

void *dllHandle;
char dllName[127+1];

public:

ulBindingClass ( void );

~ulBindingClass ( void );

int openUserLibrary (
  char *libName );

IPFUNC getIntFunc (
  char *funcName );

VPFUNC getFunc (
  char *funcName );

RULEFUNC getRuleFunc (
  char *funcName );

};

#endif
