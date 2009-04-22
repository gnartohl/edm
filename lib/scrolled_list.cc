#define __scrolled_list_cc

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
#include "scrolled_list.h"
#include "thread.h"
#include "pvs.hpp"

static void cvtToUpper (
  char *str )
{

unsigned int i;

  for ( i=0; i<strlen(str); i++ ) {
    str[i] = toupper( str[i] );
  }

}

static void cvtToLower (
  char *str )
{

unsigned int i;

  for ( i=0; i<strlen(str); i++ ) {
    str[i] = tolower( str[i] );
  }

}

static void setUpper (
  Widget w,
  XtPointer client,
  XtPointer call )
{

scrolledListClass *slo = (scrolledListClass *) client;
XmToggleButtonCallbackStruct *toggleCallback =
 (XmToggleButtonCallbackStruct *) call;
int n;
Arg args[3];

  if ( toggleCallback->set ) {
    slo->upper = 1;
    slo->lower = 0;
    n = 0;
    XtSetArg( args[n], XmNset, False ); n++;
    XtSetValues( slo->lcTb, args, n );
  }
  else
    slo->upper = 0;

}

static void setLower (
  Widget w,
  XtPointer client,
  XtPointer call )
{

scrolledListClass *slo = (scrolledListClass *) client;
XmToggleButtonCallbackStruct *toggleCallback =
 (XmToggleButtonCallbackStruct *) call;
int n;
Arg args[3];

  if ( toggleCallback->set ) {
    slo->lower = 1;
    slo->upper = 0;
    n = 0;
    XtSetArg( args[n], XmNset, False ); n++;
    XtSetValues( slo->ucTb, args, n );
  }
  else
    slo->lower = 0;

}

static void setReplace (
  Widget w,
  XtPointer client,
  XtPointer call )
{

scrolledListClass *slo = (scrolledListClass *) client;
XmToggleButtonCallbackStruct *toggleCallback =
 (XmToggleButtonCallbackStruct *) call;

  if ( toggleCallback->set ) {
    slo->replace = 1;
  }
  else {
    slo->replace = 0;
  }

}

static void setFileDoFilter (
  Widget w,
  XtPointer client,
  XtPointer call )
{

scrolledListClass *slo = (scrolledListClass *) client;
char *buf;

  buf = XmTextGetString( w );
  slo->setFile( buf );
  XtFree( buf );

  XtUnmanageChild( slo->list );
  slo->clear();
  slo->filterList();
  XtManageChild( slo->list );

}

static void setPrefix (
  Widget w,
  XtPointer client,
  XtPointer call )
{

scrolledListClass *slo = (scrolledListClass *) client;
char *buf;

  buf = XmTextGetString( w );
  strncpy( slo->prefixString, buf, 31 );
  XtFree( buf );

}

static void doFilter (
  Widget w,
  XtPointer client,
  XtPointer call )
{

scrolledListClass *slo = (scrolledListClass *) client;
char *buf;

  buf = XmTextGetString( w );
  strncpy( slo->filterString, buf, 63 );
  XtFree( buf );

  XtUnmanageChild( slo->list );
  slo->clear();
  slo->setFilterString( slo->filterString );
  slo->filterList();
  XtManageChild( slo->list );

}

static void slc_select (
  Widget w,
  XtPointer client,
  XtPointer call )
{

XmListCallbackStruct *cbs = (XmListCallbackStruct *) call;
scrolledListClass *slo = (scrolledListClass *) client;
char *item;
char buf[63+1];

  XmStringGetLtoR( cbs->item, XmFONTLIST_DEFAULT_TAG, &item );

  if ( slo->replace ) {

    char matchChar[2], replBuf[63+1], *loc;
    int l;

    l = strlen( slo->prefixString );
    if ( l > 0 ) {
      matchChar[0] = slo->prefixString[l-1];
      matchChar[1] = 0;
    }
    else {
      matchChar[0] = 0;
    }

    if ( matchChar[0] ) {

      strncpy( replBuf, item, 63 );
      replBuf[63] = 0;
      loc = strstr( replBuf, matchChar );

      if ( loc ) {
        strcpy( buf, slo->prefixString );
        Strncat( buf, &loc[1], 63 );
      }
      else {
        strncpy( buf, slo->prefixString, 63 );
	buf[63] = 0;
        Strncat( buf, item, 63 );
      }

    }
    else {
      strncpy( buf, slo->prefixString, 63 );
      buf[63] = 0;
      Strncat( buf, item, 63 );
    }

  }
  else {

    strncpy( buf, slo->prefixString, 63 );
    buf[63] = 0;
    Strncat( buf, item, 63 );

  }

  if ( slo->lower )
    cvtToLower( buf );
  else if ( slo->upper )
    cvtToUpper( buf );

  XmTextFieldSetString( slo->text, buf );
  XmTextFieldSetSelection( slo->text, 0, strlen(buf), CurrentTime );

  XtFree( item );

  return;

}

static void slc_dismiss (
  Widget w,
  XtPointer client,
  XtPointer call )
{

scrolledListClass *slo = (scrolledListClass *) client;

  slo->popdown();

}

scrolledListClass::scrolledListClass ( void ) {

  totalItems = numItems = 0;
  numVisibleItems = 1;
  dismiss_pb = (Widget) NULL;
  windowIsOpen = 0;
  strcpy( fileName, "" );
  strcpy( filterString, "" );
  strcpy( prefixString, "" );
  upper = 0;
  lower = 0;
  replace = 0;

}

scrolledListClass::~scrolledListClass ( void ) {

}

int scrolledListClass::destroy ( void ) {

  XtDestroyWidget( shell );

  return 1;

}

int scrolledListClass::create (
  Widget top,
  char *widgetName,
  int numVisItems )
{

int n, i;
Arg args[10];
XmString str;

  numVisibleItems = numVisItems;

  display = XtDisplay( top );

  shell = XtVaCreatePopupShell( widgetName, topLevelShellWidgetClass,
   top,
   XmNmappedWhenManaged, False,
   NULL );

  pane = XtVaCreateWidget( "pane", xmPanedWindowWidgetClass, shell,
   XmNsashWidth, 1,
   XmNsashHeight, 1,
   XmNallowResize, True,
   NULL );

  formTop = XtVaCreateWidget( "topform", xmFormWidgetClass, pane,
   NULL );

  str = XmStringCreateLocalized( scrolledListClass_str1 );

  file = XtVaCreateWidget( "filetext", xmTextFieldWidgetClass, formTop,
   XmNcolumns, (short) 31,
   XmNmaxLength, (short) 127,
   XmNtopAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   NULL );

  fileLabel = XtVaCreateManagedWidget( "filelabel", xmLabelWidgetClass, formTop,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, file,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, file,
   NULL );

  XmStringFree( str );

  XtAddCallback( file, XmNactivateCallback, setFileDoFilter, this );

  str = XmStringCreateLocalized( scrolledListClass_str2 );

  filter = XtVaCreateWidget( "filtertext", xmTextFieldWidgetClass, formTop,
   XmNcolumns, (short) 31,
   XmNmaxLength, (short) 31,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, file,
   XmNrightAttachment, XmATTACH_FORM,
   NULL );

  filterLabel = XtVaCreateManagedWidget( "filterlabel", xmLabelWidgetClass, formTop,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, filter,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, filter,
   NULL );

  XmStringFree( str );

  XtAddCallback( filter, XmNactivateCallback, doFilter, this );

  str = XmStringCreateLocalized( scrolledListClass_str3 );

  prefix = XtVaCreateWidget( "prefixtext", xmTextFieldWidgetClass, formTop,
   XmNcolumns, (short) 31,
   XmNmaxLength, (short) 31,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, filter,
   XmNrightAttachment, XmATTACH_FORM,
   NULL );

  prefixLabel = XtVaCreateManagedWidget( "prefixlabel", xmLabelWidgetClass, formTop,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, prefix,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, prefix,
   NULL );

  XmStringFree( str );

  XtAddCallback( prefix, XmNactivateCallback, setPrefix, this );

  str = XmStringCreateLocalized( scrolledListClass_str4 );

  ucTb = XtVaCreateManagedWidget( "uctoggle", xmToggleButtonWidgetClass, formTop,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, prefix,
   XmNrightAttachment, XmATTACH_FORM,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( ucTb, XmNvalueChangedCallback, setUpper, this );

  str = XmStringCreateLocalized( scrolledListClass_str5 );

  lcTb = XtVaCreateManagedWidget( "lctoggle", xmToggleButtonWidgetClass, formTop,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, ucTb,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, ucTb,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( lcTb, XmNvalueChangedCallback, setLower, this );

  str = XmStringCreateLocalized( scrolledListClass_str7 );

  replTb = XtVaCreateManagedWidget( "repltoggle", xmToggleButtonWidgetClass,
   formTop,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, ucTb,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, lcTb,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( replTb, XmNvalueChangedCallback, setReplace, this );

  rowColTop = XtVaCreateWidget( "rowcol", xmRowColumnWidgetClass, pane,
   NULL );

  n = 0;
  XtSetArg( args[n], XmNvisibleItemCount, numVisibleItems ); n++;
  XtSetArg( args[n], XmNselectionPolicy, XmSINGLE_SELECT ); n++;
  list = XmCreateScrolledList( rowColTop, "list", args, n );

  XtAddCallback( list, XmNsingleSelectionCallback, slc_select, this );

  formMid = XtVaCreateWidget( "midform", xmFormWidgetClass, pane,
   NULL );

  text = XtVaCreateWidget( "text", xmTextFieldWidgetClass, formMid,
   XmNcolumns, (short) 31,
   XmNmaxLength, (short) 63,
   XmNtopAttachment, XmATTACH_FORM,
   NULL );

  formBot = XtVaCreateWidget( "botform", xmFormWidgetClass, pane,
   NULL );

  str = XmStringCreateLocalized( scrolledListClass_str6 );

  dismiss_pb = XtVaCreateManagedWidget( "dismisspb", xmPushButtonGadgetClass,
   formBot,
   XmNtopAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   XmNdefaultButtonShadowThickness, 1,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( dismiss_pb, XmNactivateCallback, slc_dismiss, this );

  Atom wm_delete_window = XmInternAtom( XtDisplay(this->top()),
   "WM_DELETE_WINDOW", False );

  XmAddWMProtocolCallback( this->top(), wm_delete_window, slc_dismiss,
    this );

  XtVaSetValues( this->top(), XmNdeleteResponse, XmDO_NOTHING, NULL );

  XtManageChild( pane );
  XtManageChild( formTop );
  XtManageChild( rowColTop );
  XtManageChild( formMid );
  XtManageChild( formBot );
  XtManageChild( list );
  XtManageChild( text );
  XtManageChild( prefix );
  XtManageChild( filter );
  XtManageChild( file );

  for ( i=0; i<numVisItems; i++ ) {
    addItem( " " );
  }

  XtRealizeWidget( shell );

  windowIsOpen = 0;

  return 1;

}

void scrolledListClass::addItem (
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

  XmStringFree( str );

}

void scrolledListClass::addComplete ( void ) {

}

int scrolledListClass::popup ( void ) {

  XtPopup( shell, XtGrabNone );

  windowIsOpen = 1;

  return 1;

}

int scrolledListClass::popdown ( void ) {

  XtPopdown( shell );
  windowIsOpen = 0;

  return 1;

}

Widget scrolledListClass::top ( void ) {

  return shell;

}

Widget scrolledListClass::topRowCol ( void ) {

  return rowColTop;

}

Widget scrolledListClass::botForm ( void ) {

  return formBot;

}

Widget scrolledListClass::paneWidget ( void ) {

  return pane;

}

Widget scrolledListClass::dismissPbWidget ( void ) {

  return dismiss_pb;

}

Widget scrolledListClass::HorzScrollWidget ( void ) {

int n;
Arg args[2];
Widget w;

  n = 0;
  XtSetArg( args[n], XmNhorizontalScrollBar, &w ); n++;
  XtGetValues( XtParent(list), args, n );

  return w;

}

void scrolledListClass::filterList ( void )
{

int l, stat, n;
FILE *f;
char pv[63+1], *ptr, *name;
pvsClass *pvs;

  if ( strstr( fileName, ":" ) ) { // assume host:port

    pvs = new pvsClass( fileName );
    if ( !pvs ) {
      addItem( "<Error>" );
      return;
    }

    stat = pvs->getNumPvs( &n );
    if ( !( stat & 1 ) ) {
      addItem( "<Error>" );
      goto errRet;
    }

    if ( !n ) {
      addItem( "<No PVs found>" );
      goto errRet;
    }

    numItems = 0;

    stat = pvs->getFirstPvsName( &name );
    if ( !( stat & 1 ) ) {
      addItem( "<Error>" );
      goto errRet;
    }

    while ( stat != pvsClass::PVS_NOMORE ) {

      strncpy( pv, name, 63 );
      pv[63] = 0;

      l = strlen(pv);

      if ( l && match( filterString, pv ) ) {
        addItem( pv );
      }

      stat = pvs->getNextPvsName( &name );
      if ( !( stat & 1 ) ) {
        addItem( "<Error>" );
        goto errRet;
      }

    }

errRet:

    delete pvs;

    addComplete();

  }
  else { // file

    f = fopen( fileName, "r" );

    if ( f ) {

      numItems = 0;
      do {

        ptr = fgets( pv, 63, f );

        if ( ptr ) {

          l = strlen(pv);
          if ( l ) {
            if ( pv[l-1] == '\n' ) {
              pv[l-1] = 0;
              l--;
            }
          }

          if ( l && match( filterString, pv ) ) {
            addItem( pv );
          }

        }

      } while ( ptr );

      addComplete();

      fclose( f );

    }
    else {

      return;

    }

  }

}

void scrolledListClass::clear ( void ) {

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

void scrolledListClass::setFile (
  char *name )
{

  strncpy( fileName, name, 127 );

}

void scrolledListClass::setFilterString (
  char *string )
{

  strncpy( filterString, string, 63 );

}

int scrolledListClass::match (
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
