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

#ifndef __scrolled_list_h
#define __scrolled_list_h 1

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

#ifdef __scrolled_list_cc

#include "scrolled_list.str"

typedef struct ItemListTag {
  struct ItemListTag *flink;
  char *item;
} ItemListType, *ItemListPtr;

static void setUpper (
  Widget w,
  XtPointer client,
  XtPointer call );

static void setLower (
  Widget w,
  XtPointer client,
  XtPointer call );

static void setFileDoFilter (
  Widget w,
  XtPointer client,
  XtPointer call );

static void setPrefix (
  Widget w,
  XtPointer client,
  XtPointer call );

static void doFilter (
  Widget w,
  XtPointer client,
  XtPointer call );

static void slc_select (
  Widget w,
  XtPointer client,
  XtPointer call );

static void slc_dismiss (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class scrolledListClass {

private:

friend void setUpper (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void setLower (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void setFileDoFilter (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void setPrefix (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void doFilter (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void slc_select (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void slc_dismiss (
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

public:

scrolledListClass::scrolledListClass ( void );

scrolledListClass::~scrolledListClass ( void );

int scrolledListClass::destroy ( void );

int scrolledListClass::create (
  Widget top,
  int numVisItems );

void scrolledListClass::setFile (
  char *name );

void scrolledListClass::setFilterString (
  char *filterString );

void scrolledListClass::addItem (
  char *item );

void scrolledListClass::addComplete ( void );

void scrolledListClass::filterList ( void );

void scrolledListClass::clear ( void );

int scrolledListClass::popup ( void );

int scrolledListClass::popdown ( void );

Widget scrolledListClass::top ( void );

Widget scrolledListClass::topRowCol ( void );

Widget scrolledListClass::botForm ( void );

Widget scrolledListClass::paneWidget ( void );

Widget scrolledListClass::dismissPbWidget ( void );

Widget scrolledListClass::HorzScrollWidget ( void );

int scrolledListClass::match (
  char *pattern,
  char *string );

};

#endif
