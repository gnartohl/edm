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

#ifndef __bindings_h
#define __bindings_h 1

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include "act_grf.h"
#include "group.h"

#include "symbol.h"

#include "asymbol.h"

#include "dynSymbol.h"

#if 0
#include "line_obj.h"
#include "rectangle_obj.h"
#include "circle_obj.h"
#include "x_text_obj.h"
#include "meter.h"
#include "bar.h"
#include "x_text_dsp_obj.h"
#include "button.h"
#include "menu_button.h"
#include "message_button.h"
#include "exit_button.h"
#include "menu_mux.h"
#include "related_display.h"
#include "message_box.h"
#include "slider.h"
#include "gif.h"
#include "edmBox2.h"
#endif

#ifdef __bindings_c

#include "bindings.str"
#include "environment.str"

static int num = 0;
static char **names = NULL;
static char **classNames = NULL;
static char **param = NULL;
static char **types = NULL;
static void **dllHandle = NULL;
static char **dllName = NULL;
static char *groupName = objBindingClass_str6;

#endif

class objBindingClass {

private:

int cur_index, max;

public:

objBindingClass ( void );

~objBindingClass ( void );

char *firstObjName (
  char *objType );

char *nextObjName (
  char *objType );

activeGraphicClass *createNew (
  char *oneName );

activeGraphicClass *clone (
  char *oneName,
  activeGraphicClass *source );

char *getOjbName (
  int i );

char *getOjbType (
  int i );

char *getNameFromClass (
  char *className );

};

#endif
