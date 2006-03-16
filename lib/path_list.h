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

#ifndef __path_list_h
#define __path_list_h 1

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

class appContextClass;

#ifdef __path_list_cc

#include "path_list.str"

static void plc_select (
  Widget w,
  XtPointer client,
  XtPointer call );

static void plc_dismiss (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class pathListClass {

private:

friend void plc_select (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void plc_dismiss (
  Widget w,
  XtPointer client,
  XtPointer call );

appContextClass *apco;
Display *display;
Widget shell, pane, formTop, topLabel, rowColTop, formBot,
 list, dismiss_pb;
int totalItems, numItems, numVisibleItems, windowIsOpen;
char title[31+1];

int numPaths, indexPath, index;
void **items; // dynamic array
char **pathName; // dynamic array

public:

pathListClass ( void );

~pathListClass ( void );

int destroy ( void );

int create (
  int _numDirs,
  Widget top,
  int numVisItems,
  appContextClass *_apco
);

void addItem (
  char *item
);

void addComplete ( void );

int popup ( void );

int popdown ( void );

Widget top ( void );

Widget topRowCol ( void );

Widget topForm ( void );

Widget botForm ( void );

Widget paneWidget ( void );

Widget listWidget ( void );

Widget dismissPbWidget ( void );

Widget HorzScrollWidget ( void );

};

#endif
