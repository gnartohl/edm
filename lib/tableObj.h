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

#ifndef __tableObj_h
#define __tableObj_h 1

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
#include <Xm/Protocols.h>
#include <Xm/Separator.h>
#include <Xm/ScrolledW.h>

#include "font_pkg.h"
#include "utility.h"

class tableClass {

public:

static const int TABLECLASS_E_SUCCESS = 1;
static const int TABLECLASS_E_FAILURE = 2;
static const int TABLECLASS_E_ALREADY_EXISTS = 4;

tableClass ( void );

~tableClass ( void );

int create (
  Widget _parent,
  int _x,
  int _y,
  int _w,
  int _h,
  int _numCols,
  char *_headerAlignment,
  char *_colAlignment,
  fontInfoClass *_fi,
  const char *_fontTag,
  unsigned int _fg,
  unsigned int _bg,
  unsigned int _odd,
  unsigned int _even,
  unsigned int _top,
  unsigned int _bot
);

int destroy ( void );

Widget addCell (
  char *label
);

void endOfContent ( void );

private:

typedef struct rowListTag {
  struct rowListTag *flink;
  Widget w;
} rowListType, *rowListPtr;

typedef struct colListTag {
  struct colListTag *flink;
  Widget w;
  rowListPtr head;
  rowListPtr tail;
} colListType, *colListPtr;

Widget parent, scroll, main;
colListPtr head;
colListPtr tail;
colListPtr curCol;
int numCols;
char *colAlignment, *headerAlignment;
fontInfoClass *fi;
char *fontTag;
int eoc; // end of content
XmFontList fontList;
unsigned int fg, top, bot;
unsigned int bg, odd, even;
int x, y, w, h;
int oddRow, rowNum, colNum;

void deleteMain ( void );

void deleteRows ( void );

void manageAll ( void );

Widget getClipWidget ( void );

};

#endif
