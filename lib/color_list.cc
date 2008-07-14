#define __color_list_cc 1

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

#include <string.h>
#include <ctype.h>
#include "color_list.h"
#include "color_pkg.h"
#include "color_button.h"
#include "thread.h"

#if 0
static void doFilter (
  Widget w,
  XtPointer client,
  XtPointer call )
{

colorListClass *clo = (colorListClass *) client;

  strncpy( clo->filterString, "*", 63 );

  XtUnmanageChild( clo->list );
  clo->clear();
  clo->setFilterString( clo->filterString );
  clo->filterList();
  XtManageChild( clo->list );

}
#endif

static void clc_select (
  Widget w,
  XtPointer client,
  XtPointer call )
{

int i, mIndex;
XmListCallbackStruct *cbs = (XmListCallbackStruct *) call;
colorListClass *clo = (colorListClass *) client;
int *dest;

  for ( i=0; i<clo->numColors; i++ ) {

    //if ( (void *) cbs->item == clo->items[i] ) {
    if ( XmStringCompare( cbs->item, (XmString) clo->items[i] ) ) {

      mIndex = clo->ci->menuIndex( i );

      clo->ci->setCurIndex( mIndex );

      if ( clo->ci->curCb ) clo->ci->curCb->setIndex( mIndex );

      dest = clo->ci->getCurDestination();
      if ( dest ) {
        *dest = mIndex;
      }

      break;

    }

  }

  return;

}

static void clc_dismiss (
  Widget w,
  XtPointer client,
  XtPointer call )
{

colorListClass *clo = (colorListClass *) client;

  clo->popdown();

}

colorListClass::colorListClass ( void ) {

  totalItems = numItems = 0;
  numVisibleItems = 1;
  dismiss_pb = (Widget) NULL;
  windowIsOpen = 0;
  strcpy( fileName, "" );
  strcpy( filterString, "" );
  strcpy( prefixString, "" );
  upper = 0;
  lower = 0;

}

colorListClass::~colorListClass ( void ) {

int i;

  for ( i=0; i<numColors; i++ ) {
    if ( items[i] ) {
      XmStringFree( (XmString) items[i] );
      items[i] = (void *) NULL; 
    }
  }

  delete[] items;

}

int colorListClass::destroy ( void ) {

  XtDestroyWidget( shell );

  return 1;

}

int colorListClass::create (
  int _numColors,
  Widget top,
  int numVisItems,
  colorInfoClass *_ci )
{

int i, n;
Arg args[10];
XmString str;

  indexColor = 0;
  numColors = _numColors;
  items = new void *[numColors];
  for ( i=0; i<numColors; i++ ) {
    items[i] = (void *) NULL; 
  }

  numVisibleItems = numVisItems;
  ci = _ci;

  display = XtDisplay( top );

  //shell = XtVaAppCreateShell( colorListClass_str7, colorListClass_str7,
  //shell = XtVaAppCreateShell( "edm", "edm",
  // topLevelShellWidgetClass,
  // XtDisplay(top),
  // XtNmappedWhenManaged, False,
  // NULL );

  shell = XtVaCreatePopupShell( colorListClass_str7,
   topLevelShellWidgetClass,
   top,
   XmNmappedWhenManaged, False,
   NULL );

  pane = XtVaCreateWidget( "colormenu", xmPanedWindowWidgetClass, shell,
   XmNsashWidth, 1,
   XmNsashHeight, 1,
   XmNallowResize, True,
   NULL );

  rowColTop = XtVaCreateWidget( "rowcol", xmRowColumnWidgetClass, pane,
   NULL );

  n = 0;
  XtSetArg( args[n], XmNvisibleItemCount, numVisibleItems ); n++;
  XtSetArg( args[n], XmNselectionPolicy, XmSINGLE_SELECT ); n++;
  list = XmCreateScrolledList( rowColTop, "scrolledlist", args, n );

  XtAddCallback( list, XmNsingleSelectionCallback, clc_select, this );

  formBot = XtVaCreateWidget( "botform", xmFormWidgetClass, pane,
   NULL );

  str = XmStringCreateLocalized( colorListClass_str6 );

  dismiss_pb = XtVaCreateManagedWidget( "dismisspb", xmPushButtonGadgetClass,
   formBot,
   XmNtopAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   XmNdefaultButtonShadowThickness, 1,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( dismiss_pb, XmNactivateCallback, clc_dismiss, this );

  Atom wm_delete_window = XmInternAtom( XtDisplay(this->top()),
   "WM_DELETE_WINDOW", False );

  XmAddWMProtocolCallback( this->top(), wm_delete_window, clc_dismiss,
    this );

  XtVaSetValues( this->top(), XmNdeleteResponse, XmDO_NOTHING, NULL );

  XtManageChild( pane );
  XtManageChild( rowColTop );
  XtManageChild( formBot );

  setFilterString( "*" );
  filterList();

  XtManageChild( list );

  XtRealizeWidget( shell );

  windowIsOpen = 0;

  return 1;

}

void colorListClass::addItem (
  char *item )
{

int n;
Arg args[10];
XmString str;

  if ( !item ) return;

  str = XmStringCreateLocalized( item );

  XmListAddItemUnselected( list, str, 0 );

  totalItems++;
  numItems++;
  if ( numItems <= numVisibleItems ) {
    n = 0;
    XtSetArg( args[n], XmNvisibleItemCount, numItems ); n++;
    XtSetValues( list, args, n );
  }

  if ( indexColor < numColors ) {
    items[indexColor++] = (void *) str;
  }

  //XmStringFree( str );

}

void colorListClass::addComplete ( void ) {

}

int colorListClass::popup ( void ) {

  if ( !windowIsOpen ) {
    XtMapWidget( shell );
    //XtPopup( shell, XtGrabNone );
    windowIsOpen = 1;
  }
  else {
    XRaiseWindow( display, XtWindow(shell) );
  }

  return 1;

}

int colorListClass::popdown ( void ) {

  //XtPopdown( shell );
  XtUnmapWidget( shell );
  windowIsOpen = 0;

  return 1;

}

Widget colorListClass::top ( void ) {

  return shell;

}

Widget colorListClass::topRowCol ( void ) {

  return rowColTop;

}

Widget colorListClass::botForm ( void ) {

  return formBot;

}

Widget colorListClass::listWidget ( void ) {

  return list;

}

Widget colorListClass::paneWidget ( void ) {

  return pane;

}

Widget colorListClass::dismissPbWidget ( void ) {

  return dismiss_pb;

}

Widget colorListClass::HorzScrollWidget ( void ) {

int n;
Arg args[2];
Widget w;

  n = 0;
  XtSetArg( args[n], XmNhorizontalScrollBar, &w ); n++;
  XtGetValues( XtParent(list), args, n );

  return w;

}

void colorListClass::filterList ( void )
{

int i, l;
char *colorName;

  for ( i=0; i<ci->menuSize(); i++ ) {

    colorName = ci->colorName( ci->menuIndex( i ) );

    if ( colorName ) {

      l = strlen(colorName);

      if ( l && match( filterString, colorName ) ) {
        addItem( colorName );
      }

    }

  }

  addComplete();

}

void colorListClass::clear ( void ) {

int i, n;
Arg args[10];

  for ( i=0; i<numItems; i++ ) {
    XmListDeletePos( list, 0 );
  }

  numItems = 0;

  n = 0;
  XtSetArg( args[n], XmNvisibleItemCount, 1 ); n++;
  XtSetValues( list, args, n );

}

void colorListClass::setFile (
  char *name )
{

  strncpy( fileName, name, 127 );

}

void colorListClass::setFilterString (
  char *string )
{

  strncpy( filterString, string, 63 );

}

int colorListClass::match (
  char *pattern,
  char *string )
{

int lp, ls, ends, begins, contains, start;
char buf[127+1], *ptr;

  // *<pattern> : string ends in pattern
  // <pattern>* : string begins with pattern
  // <pattern>  : string contains pattern

  if ( !pattern ) return 0;
  lp = strlen(pattern);
  if ( lp == 0 ) return 1; // empty pattern same as "*"

  if ( !string ) return 0;
  ls = strlen(string);
  if ( ls == 0 ) return 0;

  ends = 0;
  begins = 0;
  contains = 0;

  if ( pattern[0] == '*' )
    ends = 1;
  else if ( pattern[lp-1] == '*' )
    begins = 1;
  else
    contains = 1;

  if ( strcmp( pattern, "*" ) == 0 ) {
    return 1;
  }
  else {

    if ( contains ) {

      ptr = strstr( string, pattern );
      if ( ptr )
        return 1;
      else
        return 0;

    }
    else if ( begins ) {

      strncpy( buf, pattern, 127 );
      buf[lp-1] = 0;                    // discard last char
      ptr = strstr( string, buf );
      if ( ptr == string )
        return 1;
      else
        return 0;

    }
    else if ( ends ) {

      strncpy( buf, &pattern[1], 127 ); // discard first char
      start = ls - strlen(buf);
      if ( start < 0 ) return 0;
      ptr = strstr( &string[start], buf );
      if ( ptr )
        return 1;
      else
        return 0;

    }

  }

  return 0;

}
