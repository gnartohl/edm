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

#ifndef __color_button_h
#define __color_button_h 1

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/MainW.h>
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>

#include "sys_types.h"
#include "avl.h"

#include "color_pkg.h"

#include "color_button.str"

#define COLORBUTTON_SUCCESS 1
#define COLORBUTTON_FAIL 102

#ifdef __color_button_cc

static void nameSetActive_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class colorButtonClass {

private:

friend void nameSetActive_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

Widget form, pb, namePb, tf;
int *destPtr;
colorInfoClass *ci;
char *colorPvName;
int curIndex;

public:

colorButtonClass::colorButtonClass ( void );

colorButtonClass::colorButtonClass (
 const colorButtonClass &source );

colorButtonClass colorButtonClass::operator = (
  const colorButtonClass &source );

colorButtonClass::~colorButtonClass ( void );

Widget colorButtonClass::create (
  Widget parent,
  int *dest,
  colorInfoClass *ptr,
  Arg args[],
  int num_args );

Widget colorButtonClass::createWithText (
  Widget parent,
  int *dest,
  colorInfoClass *ptr,
  char *pvName,
  Arg fArgs[],
  int fNum_args,
  Arg bArgs[],
  int bNum_args,
  Arg tArgs[],
  int tNum_args );

Widget colorButtonClass::createWithRule (
  Widget parent,
  int *dest,
  colorInfoClass *ptr,
  char *pvName,
  Arg fArgs[],
  int fNum_args,
  Arg bArgs[],
  int bNum_args,
  Arg nbArgs[],
  int nbNum_args,
  Arg tArgs[],
  int tNum_args );

Widget colorButtonClass::widget ( void );

Widget colorButtonClass::formWidget ( void );

Widget colorButtonClass::nameWidget ( void );

Widget colorButtonClass::textWidget ( void );

colorInfoClass *colorButtonClass::colorInfo( void );

int *colorButtonClass::destination( void );

unsigned int colorButtonClass::getPixel ( void );

int colorButtonClass::setPixel (
  unsigned int p );

int colorButtonClass::getIndex ( void );

int colorButtonClass::setIndex (
  int i );

void colorButtonClass::setPv (
  char *name );

char *colorButtonClass::getPv ( void );

int colorButtonClass::PvSize ( void );

};

#endif