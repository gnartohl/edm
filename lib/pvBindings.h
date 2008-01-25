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

#ifndef __pv_bindings_h
#define __pv_bindings_h 1

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include "pv_factory.h"

#ifdef __osf__
#include <sys/time.h>
#endif

#ifdef __linux__
#include <time.h>
#include <sys/time.h>
#endif

#include "sys_types.h"

#ifdef __pv_bindings_c

#include "pvBindings.str"
#include "environment.str"

static int num = 0;
static char **names = NULL;
static char **classNames = NULL;
static void **dllHandle = NULL;
static char **dllName = NULL;
static char pvOptionMenuList[255+1];

#endif

class pvBindingClass {

private:

int cur_index, max;

public:

pvBindingClass ( void );

~pvBindingClass ( void );

char *firstPvName ( void );

char *nextPvName ( void );

class ProcessVariable *createNew (
  const char *oneName,
  const char *PV_name );

char *getPvName (
  int i );

char *getPvClassName (
  int i );

char *getPvType (
  int i );

char *getNameFromClass (
  char *className );

char *getClassFromName (
  char *name );

int getClassNum (
  char *className );

int getNameNum (
  char *name );

void getOptionMenuList (
  char *list,
  int listSize,
  int *num );

int pend_io (
  double sec
);

int pend_event (
  double sec
);

void task_exit ( void );

};

#endif
