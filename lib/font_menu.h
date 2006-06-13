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

#ifndef __font_menu_h
#define __font_menu_h 1

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>

#include "sys_types.h"
#include "thread.h"
#include "avl.h"

#include "font_pkg.h"

#define FONTMENU_SUCCESS 1
#define FONTMENU_FAIL 100

class fontMenuClass;

typedef struct alignOptionListTag {
  struct alignOptionListTag *flink;
  fontMenuClass *fmp;
  char *alignString;
  Widget pb;
  int align;
} alignOptionListType, *alignOptionListPtr;

typedef struct sizeMenuListTag {
  struct sizeMenuListTag *flink;
  fontMenuClass *fmp;
  void *vfop;
  char *sizeString;
  Widget pb;
  int size;
  float fsize;
  char pad[3];
} sizeMenuListType, *sizeMenuListPtr;

typedef struct familyOptionListTag {
  struct familyOptionListTag *flink;
  fontMenuClass *fmp;
  char *name;
  Widget familyPb;
  Widget sizeHistory;
  char *curSizeString;
  Widget sizePullDown;
  sizeMenuListPtr head;
  sizeMenuListPtr tail;
} familyOptionListType, *familyOptionListPtr;

class fontMenuClass {

private:

friend void setFamily_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void setSize_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void setAlign_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void Bold_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void Italics_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

Widget form, familyOption, familyPullDown, sizeOption,
 alignOption, alignPullDown, boldToggle, italicsToggle;

familyOptionListPtr familyHead;
familyOptionListPtr familyTail;

alignOptionListPtr alignHead, alignTail;

char *alignStr;
int align;
char *sizeStr;
char *familyStr;
int bold;
char boldStr[8];
int italics;
char italicsStr[4];

char fontTagStr[127+1];

int change;

public:

fontMenuClass ( void );

fontMenuClass ( const fontMenuClass& source ) {

  fprintf( stderr, "In fontMenuClass( const fontMenuClass& source )\n" );
  // do not let compiler generate default copy constructor

}

~fontMenuClass ( void );

void copy ( const fontMenuClass& source );

int fontChanged ( void ) {
int i;
  i = change;
  change = 0;
  return i;
}

Widget createFontMenu (
  Widget parent,
  fontInfoClass *fi,
  Arg args[],
  int numArgs,
  int includeAlignInfo );

Widget createFontMenu (
  Widget parent,
  fontInfoClass *fi,
  Arg args[],
  int numArgs );

int destroyFontMenu ( void );

char *currentFontTag ( void );

int currentFontAlignment ( void ) {
  return align;
}

void show ( void );

int setFontTag (
  char *string );

int setFontAlignment (
  int _align );

Widget familyWidget ( void )
{

  return familyOption;

}

Widget sizeWidget ( void )
{

  return sizeOption;

}

Widget alignWidget ( void )
{

  return alignOption;

}

Widget boldWidget ( void )
{

  return boldToggle;

}

Widget italicsWidget ( void )
{

  return italicsToggle;

}

};

#endif

