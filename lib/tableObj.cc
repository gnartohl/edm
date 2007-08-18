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

#include "tableObj.h"

tableClass::tableClass ( void ) {

  main = scroll = NULL;
  head = tail = curCol = NULL;
  numCols = 0;
  headerAlignment = NULL;
  colAlignment = NULL;
  fi = NULL;
  fontTag = NULL;
  eoc = 0;
  fontList = NULL;
  rowNum = colNum = 0;

}

void tableClass::deleteMain ( void ) {

  if ( main ) {
    main = NULL;
  }

  if ( scroll ) {
    XtDestroyWidget( scroll );
    scroll = NULL;
  }

}

void tableClass::deleteRows ( void ) {

rowListPtr curR, nextR;
colListPtr curC, nextC;

  curC = head->flink;
  while ( curC ) {

    nextC = curC->flink;

    curR = curC->head->flink;
    while ( curR ) {

      nextR = curR->flink;

      curR->w = NULL;
      delete curR;

      curR = nextR;

    }

    curC->w = NULL;

    delete curC->head;

    delete curC;

    curC = nextC;

  }

  tail = head;
  tail->flink = NULL;

}

tableClass::~tableClass ( void ) {

  if ( main ) {

    deleteRows();
    numCols = 0;
    curCol = NULL;

    deleteMain();

  }

  if ( head ) {
    delete head;
    head = NULL;
    tail = head;
  }

  if ( headerAlignment ) {
    delete[] headerAlignment;
    headerAlignment = NULL;
  }

  if ( colAlignment ) {
    delete[] colAlignment;
    colAlignment = NULL;
  }

  if ( fontTag ) {
    delete[] fontTag;
    fontTag = NULL;
  }

  if ( fontList ) {
    XmFontListFree( fontList );
    fontList = NULL;
  }

  eoc = 0;

}

int tableClass::create (
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
) {

int i;

  if ( main ) return TABLECLASS_E_ALREADY_EXISTS;

  parent = _parent;
  x = _x;
  y = _y;
  w = _w;
  h = _h;
  numCols = _numCols;
  fg = _fg;
  bg = _bg;
  odd = _odd;
  even = _even;
  top = _top;
  bot = _bot;
  fi = _fi;
  curCol = NULL;
  eoc = 0;
  oddRow = 1;
  rowNum = colNum = 0;

  if ( _headerAlignment ) {
    headerAlignment = new char[strlen(_headerAlignment)+1];
    strcpy( headerAlignment, _headerAlignment );
  }
 
  if ( _colAlignment ) {
    colAlignment = new char[strlen(_colAlignment)+1];
    strcpy( colAlignment, _colAlignment );
  }
 
  if ( _fontTag ) {
    fontTag = new char[strlen(_fontTag)+1];
    strcpy( fontTag, _fontTag );
  }
 
  if ( fi ) {
    if ( fontTag ) {
      fi->getTextFontList( fontTag, &fontList );
    }
  }

  scroll = XtVaCreateWidget( "", xmScrolledWindowWidgetClass,
   parent,
   XmNx, x,
   XmNy, y,
   XmNwidth, w,
   XmNheight, h,
   XmNscrollBarDisplayPolicy, XmAS_NEEDED,
   XmNscrollingPolicy, XmAUTOMATIC,
   XmNvisualPolicy, XmCONSTANT,
   XmNmarginWidth, 0,
   XmNmarginHeight, 0,
   XmNtopShadowColor, top,
   XmNbottomShadowColor, bot,
   XmNborderColor, bg,
   XmNhighlightColor, bg,
   XmNbackground, bg,
   XmNforeground, bg,
   NULL );

  if ( !scroll ) return TABLECLASS_E_FAILURE;

  main = XtVaCreateWidget( "", xmRowColumnWidgetClass,
   scroll,
   XmNorientation, XmHORIZONTAL,
   XmNpacking, XmPACK_TIGHT,
   XmNnumColumns, numCols,
   XmNwidth, w,
   XmNheight, h,
   XmNtopShadowColor, bg,
   XmNbottomShadowColor, bg,
   XmNborderColor, bg,
   XmNhighlightColor, bg,
   XmNforeground, bg,
   XmNbackground, bg,
   XmNentryBorder, 0,
   XmNmarginHeight, 0,
   XmNmarginWidth, 0,
   XmNspacing, 0,
   XmNadjustLast, 0,
   XmNadjustMargin, 0,
   NULL );

  if ( !main ) return TABLECLASS_E_FAILURE;

  head = new colListType;
  tail = head;
  tail->flink = NULL;

  for ( i=0; i<numCols; i++ ) {

    curCol = new colListType;
    tail->flink = curCol;
    tail = curCol;
    tail->flink = NULL;

    curCol->head = new rowListType;
    curCol->tail = curCol->head;
    curCol->tail->flink = NULL;

    curCol->w = XtVaCreateWidget( "", xmRowColumnWidgetClass, main,
     XmNorientation, XmVERTICAL,
     XmNpacking, XmPACK_COLUMN,
     XmNforeground, fg,
     XmNbackground, fg,
     XmNtopShadowColor, bg,
     XmNbottomShadowColor, bg,
     XmNborderColor, bg,
     XmNhighlightColor, bg,
     XmNentryBorder, 0,
     XmNmarginHeight, 1,
     XmNmarginWidth, 1,
     XmNspacing, 0,
     XmNadjustLast, 0,
     XmNadjustMargin, 0,
     XmNisAligned, 0,
     //XmNentryAlignment, XmALIGNMENT_END,
     NULL );

  }

  curCol = head->flink;

  return TABLECLASS_E_SUCCESS;

}

int tableClass::destroy ( void ) {

  if ( !main ) return TABLECLASS_E_SUCCESS;

  deleteRows();
  numCols = 0;
  curCol = NULL;

  if ( head ) {
    delete head;
    head = NULL;
    tail = head;
  }

  deleteMain();

  if ( headerAlignment ) {
    delete[] headerAlignment;
    headerAlignment = NULL;
  }

  if ( colAlignment ) {
    delete[] colAlignment;
    colAlignment = NULL;
  }

  if ( fontTag ) {
    delete[] fontTag;
    fontTag = NULL;
  }

  if ( fontList ) {
    XmFontListFree( fontList );
    fontList = NULL;
  }

  return TABLECLASS_E_SUCCESS;

}

Widget tableClass::addCell (
  char *label
) {

rowListPtr curR;
XmString str;
unsigned int bgc;
char *buf;
int align;

  if ( !curCol ) return NULL;

  curR = new rowListType;
  curCol->tail->flink = curR;
  curCol->tail = curR;
  curCol->tail->flink = NULL;

  buf = new char[strlen(label)+3];
  //strcpy( buf, " " );
  //strcat( buf, label );
  //strcat( buf, " " );
  strcpy( buf, label );

  if ( fontList ) {
    str = XmStringCreate( buf, fontTag );
  }
  else {
    str = XmStringCreateLocalized( buf );
  }

  if ( oddRow ) {
    bgc = odd;
  }
  else {
    bgc = even;
  }

  //fprintf( stderr, "r %-d     c %-d\n", rowNum, colNum );

  if ( rowNum == 0 ) {

    if ( headerAlignment ) {
      if ( headerAlignment[colNum] == 'r' ) {
        align = XmALIGNMENT_END;
      }
      else if ( headerAlignment[colNum] == 'c' ) {
        align = XmALIGNMENT_CENTER;
      }
      else {
        align = XmALIGNMENT_BEGINNING;
      }
    }
    else {
      align = XmALIGNMENT_BEGINNING;
    }

  }
  else {

    if ( colAlignment ) {
      if ( colAlignment[colNum] == 'r' ) {
        align = XmALIGNMENT_END;
      }
      else if ( colAlignment[colNum] == 'c' ) {
        align = XmALIGNMENT_CENTER;
      }
      else {
        align = XmALIGNMENT_BEGINNING;
      }
    }
    else {
      align = XmALIGNMENT_BEGINNING;
    }

  }

  curR->w = XtVaCreateWidget( "", xmLabelWidgetClass,
   curCol->w,
   XmNlabelString, str,
   XmNfontList, fontList,
   XmNforeground, fg,
   XmNbackground, bgc,
   XmNtopShadowColor, bg,
   XmNbottomShadowColor, bg,
   XmNborderColor, bg,
   XmNhighlightColor, bg,
   XmNalignment, align,
   NULL );

  XmStringFree( str );

  delete[] buf;

  curCol = curCol->flink;
  colNum++;
  if ( !curCol ) {
    colNum = 0;
    rowNum++;
    curCol = head->flink;
    if ( oddRow ) {
      oddRow = 0;
    }
    else {
      oddRow = 1;
    }
  }



  return curR->w;

}

void tableClass::manageAll ( void ) {

rowListPtr curR, nextR;
colListPtr curC, nextC;
int count = 0;

  curC = head->flink;
  while ( curC ) {

    nextC = curC->flink;

    curR = curC->head->flink;
    while ( curR ) {

      nextR = curR->flink;

      XtManageChild( curR->w );
      count++;

      curR = nextR;

    }

    XtManageChild( curC->w );

    curC = nextC;

  }

  XtManageChild( scroll );
  XtManageChild( main );

}

void tableClass::endOfContent ( void ) {

unsigned short theW, theH;

  eoc = 1;
  manageAll();
  XtRealizeWidget( parent );

  XtVaSetValues( getClipWidget(),
   XmNtopShadowColor, bg,
   XmNbottomShadowColor, bg,
   XmNborderColor, bg,
   XmNhighlightColor, bg,
   XmNbackground, bg,
   XmNforeground, bg,
   NULL );

  theW = theH = 0;
  XtVaGetValues( main,
   XmNwidth, &theW,
   XmNheight, &theH,
   NULL );

  if ( theW < w ) {
    x = x + ( w - theW ) / 2 ;
  }

  XtVaSetValues( main,
   XmNx, (short) x,
   NULL );

}

Widget tableClass::getClipWidget ( void ) {

int n;
Arg args[2];
Widget w;

  n = 0;
  XtSetArg( args[n], XmNclipWindow, &w ); n++;
  XtGetValues( scroll, args, n );

  return w;

}
