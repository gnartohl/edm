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

#ifndef __color_list_h
#define __color_list_h 1

#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/BulletinB.h>
#include <Xm/DrawingA.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleB.h>
#include <Xm/ArrowBG.h>
#include <Xm/Label.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/DialogS.h>
#include <Xm/PanedW.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/Form.h>
#include <Xm/List.h>
#include <Xm/Protocols.h>

#ifdef __color_list_cc

#include "color_list.str"

#if 0
static void doFilter (
  Widget w,
  XtPointer client,
  XtPointer call );
#endif

static void clc_select (
  Widget w,
  XtPointer client,
  XtPointer call );

static void clc_dismiss (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

typedef struct ItemListTag {
  struct ItemListTag *flink;
  char *item;
} ItemListType, *ItemListPtr;

class colorInfoClass;

class colorListClass {

private:

#if 0
friend void doFilter (
  Widget w,
  XtPointer client,
  XtPointer call );
#endif

friend void clc_select (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void clc_dismiss (
  Widget w,
  XtPointer client,
  XtPointer call );

Display *display;
Widget shell, pane, formTop, fileLabel, file, filterLabel, filter,
 prefixLabel, prefix, rowColTop, formMid, formBot, list, text,
 dismiss_pb, lcTb, ucTb;
int totalItems, numItems, numVisibleItems, windowIsOpen, upper, lower;
char filterString[63+1], prefixString[31+1], fileName[127+1];
ItemListPtr head, tail;
colorInfoClass *ci;

int numColors, indexColor;
void **items; // dynamic array

public:

colorListClass ( void );

~colorListClass ( void );

int destroy ( void );

int create (
			    int _numColors,
  Widget top,
  int numVisItems,
  colorInfoClass *_ci );

void setFile (
  char *name );

void setFilterString (
  char *filterString );

void addItem (
  char *item );

void addComplete ( void );

void filterList ( void );

void clear ( void );

int popup ( void );

int popdown ( void );

Widget top ( void );

Widget topRowCol ( void );

Widget botForm ( void );

Widget paneWidget ( void );

Widget listWidget ( void );

Widget dismissPbWidget ( void );

Widget HorzScrollWidget ( void );

int match (
  char *pattern,
  char *string );

};

#endif
