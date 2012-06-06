#define __path_list_cc 1

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
#include "path_list.h"
#include "app_pkg.h"

#ifdef index
#undef index
#endif

static void plc_select (
  Widget w,
  XtPointer client,
  XtPointer call )
{

int i;
XmListCallbackStruct *cbs = (XmListCallbackStruct *) call;
pathListClass *plo = (pathListClass *) client;

  for ( i=0; i<plo->numPaths; i++ ) {

    if ( (void *) cbs->item == plo->items[i] ) {

      plo->index = i;

      strncpy( plo->apco->curPath, plo->pathName[i], 127 );

      break;

    }
  }

  return;

}

static void plc_dismiss (
  Widget w,
  XtPointer client,
  XtPointer call )
{

pathListClass *plo = (pathListClass *) client;

  plo->popdown();

}

pathListClass::pathListClass ( void ) {

  shell = rowColTop = formTop = topLabel = formBot = list = pane =
   dismiss_pb = NULL;
  totalItems = numItems = numPaths = 0;
  items = NULL;
  pathName = NULL;
  numVisibleItems = 1;
  windowIsOpen = 0;

}

pathListClass::~pathListClass ( void ) {

int i;

  for ( i=0; i<numPaths; i++ ) {
    if ( items[i] ) {
      XmStringFree( (XmString) items[i] );
      items[i] = (void *) NULL; 
    }
    if ( pathName ) {
      if ( pathName[i] ) {
        delete[] pathName[i];
      }
    }
  }

  if ( items ) delete[] items;
  if ( pathName ) delete[] pathName;

  destroy();

}

int pathListClass::destroy ( void ) {

  popdown();
  //if ( shell ) {
  //  XtDestroyWidget( shell );
  //  shell = NULL;
  //}

  return 1;

}

int pathListClass::create (
  int _numPaths,
  Widget top,
  int numVisItems,
  appContextClass *_apco
) {

int i, n;
Arg args[10];
XmString str;
XTextProperty xtext;
char *pTitle;

  apco = _apco;

  pathName = new char *[_numPaths];
  for ( i=0; i<numPaths; i++ ) {
    pathName[i] = NULL;
  }

  indexPath = 0;
  numPaths = _numPaths;
  items = new void *[numPaths];
  for ( i=0; i<numPaths; i++ ) {
    items[i] = (void *) NULL; 
  }

  numVisibleItems = numVisItems;

  display = XtDisplay( top );

  //shell = XtVaAppCreateShell( "edm", "edm",
  // topLevelShellWidgetClass,
  // XtDisplay(top),
  // XtNmappedWhenManaged, False,
  // NULL );

  shell = XtVaCreatePopupShell( "edm", topLevelShellWidgetClass,
   top,
   XtNmappedWhenManaged, False,
   NULL );

  pane = XtVaCreateWidget( "pathmenu", xmPanedWindowWidgetClass, shell,
   XmNsashWidth, 1,
   XmNsashHeight, 1,
   XmNallowResize, True,
   NULL );

  formTop = XtVaCreateWidget( "botform", xmFormWidgetClass, pane,
   NULL );

  str = XmStringCreateLocalized( pathListClass_str8 );

  topLabel = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   formTop,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   XmNleftAttachment, XmATTACH_FORM,
   NULL );

  XmStringFree( str );

  rowColTop = XtVaCreateWidget( "rowcol", xmRowColumnWidgetClass, pane,
   NULL );

  n = 0;
  XtSetArg( args[n], XmNvisibleItemCount, numVisibleItems ); n++;
  XtSetArg( args[n], XmNselectionPolicy, XmSINGLE_SELECT ); n++;
  list = XmCreateScrolledList( rowColTop, "scrolledlist", args, n );

  XtAddCallback( list, XmNsingleSelectionCallback, plc_select, this );

  formBot = XtVaCreateWidget( "botform", xmFormWidgetClass, pane,
   NULL );

  str = XmStringCreateLocalized( pathListClass_str6 );

  dismiss_pb = XtVaCreateManagedWidget( "dismisspb", xmPushButtonGadgetClass,
   formBot,
   XmNtopAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   XmNdefaultButtonShadowThickness, 1,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( dismiss_pb, XmNactivateCallback, plc_dismiss, this );

  Atom wm_delete_window = XmInternAtom( XtDisplay(this->top()),
   "WM_DELETE_WINDOW", False );

  XmAddWMProtocolCallback( this->top(), wm_delete_window, plc_dismiss,
    this );

  XtVaSetValues( this->top(), XmNdeleteResponse, XmDO_NOTHING, NULL );

  XtManageChild( pane );
  XtManageChild( formTop );
  XtManageChild( rowColTop );
  XtManageChild( formBot );

  XtManageChild( list );

  XtRealizeWidget( shell );

  strncpy( title, pathListClass_str8, 31 );
  title[31] = 0;

  pTitle = title;
  XStringListToTextProperty( &pTitle, 1, &xtext );
  XSetWMName( display, XtWindow(shell), &xtext );
  XSetWMIconName( display, XtWindow(shell), &xtext );
  XFree( xtext.value );

  windowIsOpen = 0;

  return 1;

}

void pathListClass::addItem (
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

  if ( indexPath < numPaths ) {
    pathName[indexPath] = new char[strlen(item)+1];
    strcpy( pathName[indexPath], item );
    items[indexPath] = (void *) str;
    indexPath++;
  }

}

void pathListClass::addComplete ( void ) {

}

int pathListClass::popup ( void ) {

  if ( shell ) {

    if ( !windowIsOpen ) {
      XtMapWidget( shell );
      windowIsOpen = 1;
    }
    else {
      XRaiseWindow( display, XtWindow(shell) );
    }

  }

  return 1;

}

int pathListClass::popdown ( void ) {

  if ( windowIsOpen ) {
    if ( shell ) XtUnmapWidget( shell );
    windowIsOpen = 0;
  }

  return 1;

}

Widget pathListClass::top ( void ) {

  return shell;

}

Widget pathListClass::topRowCol ( void ) {

  return rowColTop;

}

Widget pathListClass::topForm ( void ) {

  return formTop;

}

Widget pathListClass::botForm ( void ) {

  return formBot;

}

Widget pathListClass::listWidget ( void ) {

  return list;

}

Widget pathListClass::paneWidget ( void ) {

  return pane;

}

Widget pathListClass::dismissPbWidget ( void ) {

  return dismiss_pb;

}

Widget pathListClass::HorzScrollWidget ( void ) {

int n;
Arg args[2];
Widget w;

  n = 0;
  XtSetArg( args[n], XmNhorizontalScrollBar, &w ); n++;
  XtGetValues( XtParent(list), args, n );

  return w;

}
