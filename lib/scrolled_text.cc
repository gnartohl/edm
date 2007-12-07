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

#include "scrolled_text.h"

#include "thread.h"

void stc_toggle_autoOpen (
  Widget w,
  XtPointer client,
  XtPointer call )
{

scrolledTextClass *sto = (scrolledTextClass *) client;

  if ( sto->autoOpenWindow )
    sto->autoOpenWindow = 0;
  else
    sto->autoOpenWindow = 1;

}

void stc_toggle_autoRaise (
  Widget w,
  XtPointer client,
  XtPointer call )
{

scrolledTextClass *sto = (scrolledTextClass *) client;

  if ( sto->autoRaiseWindow )
    sto->autoRaiseWindow = 0;
  else
    sto->autoRaiseWindow = 1;

}

void stc_dismiss (
  Widget w,
  XtPointer client,
  XtPointer call )
{

scrolledTextClass *sto = (scrolledTextClass *) client;

  sto->popdown();

}

void stc_clear (
  Widget w,
  XtPointer client,
  XtPointer call )
{

scrolledTextClass *sto = (scrolledTextClass *) client;

  sto->totalSize = 0;

  XmTextSetString( sto->topScrolledText, "" );

}

scrolledTextClass::scrolledTextClass ( void ) {

  maxSize = 0;
  bufSize = 0;
  textTag = (char *) NULL;
  textFontList = NULL;
  dismiss_pb = (Widget) NULL;
  clear_pb = (Widget) NULL;
  autoOpen_tb = (Widget) NULL;
  autoRaise_tb = (Widget) NULL;
  autoOpenWindow = 1;
  autoRaiseWindow = 1;
  windowIsOpen = 0;
  shell = NULL;
  pane = NULL;
  topScrolledText = NULL;
  topForm = NULL;
  clear_pb = NULL;

}

scrolledTextClass::~scrolledTextClass ( void ) {

  maxSize = 0;
  bufSize = 0;

  if ( textTag ) delete[] textTag;
  textTag = (char *) NULL;

  if ( textFontList ) XmFontListFree( textFontList );
  textFontList = NULL;

}

int scrolledTextClass::destroy ( void ) {

  XtDestroyWidget( shell );

  if ( textTag ) delete[] textTag;
  textTag = (char *) NULL;

  if ( textFontList ) XmFontListFree( textFontList );
  textFontList = NULL;

  maxSize = 0;
  bufSize = 0;

  shell = NULL;
  pane = NULL;
  topScrolledText = NULL;
  topForm = NULL;
  clear_pb = NULL;

  return 1;

}

int scrolledTextClass::create (
  Widget top,
  char *widgetName,
  int _x,
  int _y,
  int _bufSize,
  fontInfoClass *fi,
  const char *textFontTag )
{

int n;
Arg args[10];
XmString str;

  x = _x;
  y = _y;

  if ( _bufSize < 1000 )
    bufSize = 1000;
  else
    bufSize = _bufSize;

  maxSize = bufSize * 2 - 80;
  bufExtra = (int) ( ( maxSize - bufSize ) * 0.5 );

  totalSize = 0;

  display = XtDisplay( top );

  if ( fi ) {

    if ( textFontTag ) {
      textTag = new char[strlen(textFontTag)+1];
      strcpy( textTag, textFontTag );
      fi->getTextFontList( textTag, &textFontList );
    }

  }

  shell = XtVaCreatePopupShell( widgetName, topLevelShellWidgetClass,
   top,
   XmNmappedWhenManaged, False,
   NULL );

  pane = XtVaCreateWidget( "pane", xmPanedWindowWidgetClass, shell,
   XmNsashWidth, 1,
   XmNsashHeight, 1,
   NULL );

  topForm = XtVaCreateWidget( "topform", xmFormWidgetClass, pane,
   NULL );

  n = 0;
  XtSetArg( args[n], XmNrows, 24 ); n++;
  XtSetArg( args[n], XmNcolumns, 80 ); n++;
  XtSetArg( args[n], XmNeditable, False ); n++;
  XtSetArg( args[n], XmNeditMode, XmMULTI_LINE_EDIT ); n++;
  XtSetArg( args[n], XmNcursorPositionVisible, False ); n++;
  if ( textFontList ) { XtSetArg( args[n], XmNfontList, textFontList ); n++; }
  XtSetArg( args[n], XmNmaxLength, maxSize+10 ); n++;
  // XtSetArg( args[n], XmN, ); n++;

  topScrolledText = XmCreateScrolledText( pane, "scrolledtext", args, n );

  if ( textTag )
    str = XmStringCreate( scrolledTextClass_str1, textTag );
  else
    str = XmStringCreateLocalized( scrolledTextClass_str1 );

  if ( textFontList ) {
    dismiss_pb = XtVaCreateManagedWidget( "dismisspb", xmPushButtonGadgetClass,
     topForm,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     XmNdefaultButtonShadowThickness, 1,
     XmNlabelString, str,
     XmNfontList, textFontList,
     NULL );
  }
  else {
    dismiss_pb = XtVaCreateManagedWidget( "dismisspb", xmPushButtonGadgetClass,
     topForm,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     XmNdefaultButtonShadowThickness, 1,
     XmNlabelString, str,
     NULL );
  }

  XmStringFree( str );

  XtAddCallback( dismiss_pb, XmNactivateCallback, stc_dismiss, this );

  Atom wm_delete_window = XmInternAtom( XtDisplay(this->top()),
   "WM_DELETE_WINDOW", False );

  XmAddWMProtocolCallback( this->top(), wm_delete_window, stc_dismiss,
    this );

  XtVaSetValues( this->top(), XmNdeleteResponse, XmDO_NOTHING, NULL );

  if ( textTag )
    str = XmStringCreate( scrolledTextClass_str2, textTag );
  else
    str = XmStringCreateLocalized( scrolledTextClass_str2 );

  if ( textFontList ) {
    clear_pb = XtVaCreateManagedWidget( "clearpb", xmPushButtonGadgetClass,
     topForm,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, dismiss_pb,
     XmNrightAttachment, XmATTACH_WIDGET,
     XmNrightWidget, dismiss_pb,
     XmNdefaultButtonShadowThickness, 1,
     XmNlabelString, str,
     XmNfontList, textFontList,
     NULL );
  }
  else {
    clear_pb = XtVaCreateManagedWidget( "clearpb", xmPushButtonGadgetClass,
     topForm,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, dismiss_pb,
     XmNrightAttachment, XmATTACH_WIDGET,
     XmNrightWidget, dismiss_pb,
     XmNdefaultButtonShadowThickness, 1,
     XmNlabelString, str,
     NULL );
  }

  XmStringFree( str );

  XtAddCallback( clear_pb, XmNactivateCallback, stc_clear, this );

  if ( textTag )
    str = XmStringCreate( scrolledTextClass_str3, textTag );
  else
    str = XmStringCreateLocalized( scrolledTextClass_str3 );

  if ( textFontList ) {
    autoOpen_tb = XtVaCreateManagedWidget( "autoopentoggle", xmToggleButtonWidgetClass,
     topForm,
     XmNset, (XtArgVal) autoOpenWindow,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, dismiss_pb,
     XmNleftAttachment, XmATTACH_FORM,
     XmNlabelString, str,
     XmNfontList, textFontList,
     NULL );
  }
  else {
    autoOpen_tb = XtVaCreateManagedWidget( "autoopentoggle", xmToggleButtonWidgetClass,
     topForm,
     XmNset, (XtArgVal) autoOpenWindow,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, dismiss_pb,
     XmNleftAttachment, XmATTACH_FORM,
     XmNlabelString, str,
     NULL );
  }

  XmStringFree( str );

  XtAddCallback( autoOpen_tb, XmNarmCallback, stc_toggle_autoOpen, this );

  autoRaiseWindow = 1;

  if ( textTag )
    str = XmStringCreate( scrolledTextClass_str4, textTag );
  else
    str = XmStringCreateLocalized( scrolledTextClass_str4 );

  if ( textFontList ) {
    autoRaise_tb = XtVaCreateManagedWidget( "autoraisetoggle", xmToggleButtonWidgetClass,
     topForm,
     XmNset, (XtArgVal) True,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, dismiss_pb,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, autoOpen_tb,
     XmNlabelString, str,
     XmNfontList, textFontList,
     NULL );
  }
  else {
    autoRaise_tb = XtVaCreateManagedWidget( "autoraisetoggle", xmToggleButtonWidgetClass,
     topForm,
     XmNset, (XtArgVal) True,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, dismiss_pb,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, autoOpen_tb,
     XmNlabelString, str,
     NULL );
  }

  XmStringFree( str );

  XtAddCallback( autoRaise_tb, XmNarmCallback, stc_toggle_autoRaise,
   this );

  XtManageChild( topScrolledText );
  XtManageChild( topForm );
  XtManageChild( pane );

  XmTextSetString( topScrolledText, "" );

  windowIsOpen = 0;

  return 1;

}

int scrolledTextClass::destroyEmbedded ( void ) {

  if ( textTag ) delete[] textTag;
  textTag = (char *) NULL;

  if ( textFontList ) XmFontListFree( textFontList );
  textFontList = NULL;

  maxSize = 0;
  bufSize = 0;

  return 1;

}

int scrolledTextClass::createEmbedded (
  Widget top,
  int _x,
  int _y,
  int nRows,
  int nCols,
  int _bufSize,
  fontInfoClass *fi,
  const char *textFontTag )
{

int n;
Arg args[10];
XmString str;

  x = _x;
  y = _y;

  if ( _bufSize < 1000 )
    bufSize = 1000;
  else
    bufSize = _bufSize;

  maxSize = bufSize * 2 - 80;
  bufExtra = (int) ( ( maxSize - bufSize ) * 0.5 );

  totalSize = 0;

  display = XtDisplay( top );

  if ( fi ) {

    if ( textFontTag ) {
      textTag = new char[strlen(textFontTag)+1];
      strcpy( textTag, textFontTag );
      fi->getTextFontList( textTag, &textFontList );
    }

  }

  pane = XtVaCreateWidget( "pane", xmPanedWindowWidgetClass, top,
   XmNsashWidth, 1,
   XmNsashHeight, 1,
   NULL );

  topForm = XtVaCreateWidget( "topform", xmFormWidgetClass, pane,
   NULL );

  n = 0;
  XtSetArg( args[n], XmNrows, nRows ); n++;
  XtSetArg( args[n], XmNcolumns, nCols ); n++;
  XtSetArg( args[n], XmNeditable, False ); n++;
  XtSetArg( args[n], XmNeditMode, XmMULTI_LINE_EDIT ); n++;
  XtSetArg( args[n], XmNcursorPositionVisible, False ); n++;
  XtSetArg( args[n], XmNfontList, textFontList ); n++;
  XtSetArg( args[n], XmNmaxLength, maxSize+10 ); n++;

  topScrolledText = XmCreateScrolledText( pane, "scrolledtext", args, n );

  if ( textTag )
    str = XmStringCreate( scrolledTextClass_str2, textTag );
  else
    str = XmStringCreateLocalized( scrolledTextClass_str2 );

  clear_pb = XtVaCreateManagedWidget( "clearpb", xmPushButtonGadgetClass,
   topForm,
   XmNtopAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   XmNdefaultButtonShadowThickness, 1,
   XmNlabelString, str,
   XmNfontList, textFontList,
   NULL );

  XmStringFree( str );

  XtAddCallback( clear_pb, XmNactivateCallback, stc_clear, this );

  autoOpenWindow = 0;
  autoRaiseWindow = 0;

  XtManageChild( topScrolledText );
  XtManageChild( topForm );
  XtManageChild( pane );

  XmTextSetString( topScrolledText, "" );

  windowIsOpen = 0;

  return 1;

}

int scrolledTextClass::createEmbeddedWH (
  Widget top,
  int _x,
  int _y,
  int _w,
  int _h,
  int _bufSize,
  fontInfoClass *fi,
  const char *textFontTag,
  int provideClearButton )
{

int n;
Arg args[10];
XmString str;

  x = _x;
  y = _y;

  if ( _bufSize < 1000 )
    bufSize = 1000;
  else
    bufSize = _bufSize;

  maxSize = bufSize * 2 - 80;
  bufExtra = (int) ( ( maxSize - bufSize ) * 0.5 );

  totalSize = 0;

  display = XtDisplay( top );

  if ( fi ) {

    if ( textFontTag ) {
      textTag = new char[strlen(textFontTag)+1];
      strcpy( textTag, textFontTag );
      fi->getTextFontList( textTag, &textFontList );
    }

  }

  pane = XtVaCreateWidget( "pane", xmPanedWindowWidgetClass, top,
   XmNsashWidth, 1,
   XmNsashHeight, 1,
   NULL );

  topForm = XtVaCreateWidget( "topform", xmFormWidgetClass, pane,
   NULL );

  n = 0;
  XtSetArg( args[n], XmNheight, _h ); n++;
  XtSetArg( args[n], XmNwidth, _w ); n++;
  XtSetArg( args[n], XmNeditable, False ); n++;
  XtSetArg( args[n], XmNeditMode, XmMULTI_LINE_EDIT ); n++;
  XtSetArg( args[n], XmNcursorPositionVisible, False ); n++;
  XtSetArg( args[n], XmNfontList, textFontList ); n++;
  XtSetArg( args[n], XmNmaxLength, maxSize+10 ); n++;

  topScrolledText = XmCreateScrolledText( pane, "scrolledtext", args, n );

  if ( provideClearButton ) {

    if ( textTag )
      str = XmStringCreate( scrolledTextClass_str2, textTag );
    else
      str = XmStringCreateLocalized( scrolledTextClass_str2 );

    clear_pb = XtVaCreateManagedWidget( "clearpb", xmPushButtonGadgetClass,
     topForm,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     XmNdefaultButtonShadowThickness, 1,
     XmNlabelString, str,
     XmNfontList, textFontList,
     NULL );

    XmStringFree( str );

    XtAddCallback( clear_pb, XmNactivateCallback, stc_clear, this );

  }
  else {

    clear_pb = NULL;

  }

  autoOpenWindow = 0;
  autoRaiseWindow = 0;

  XtManageChild( topScrolledText );
  XtManageChild( topForm );
  XtManageChild( pane );

  XmTextSetString( topScrolledText, "" );

  windowIsOpen = 0;

  return 1;

}

int scrolledTextClass::createEmbeddedWH (
  Widget top,
  int _x,
  int _y,
  int _w,
  int _h,
  int _bufSize,
  fontInfoClass *fi,
  const char *textFontTag )
{

int stat;

  stat = createEmbeddedWH( top, _x, _y, _w, _h, _bufSize, fi,
   textFontTag, 1 );

  return stat;

}

int scrolledTextClass::addTextNoNL (
  char *text )
{

int stringSize, newTotalSize, num;

  stringSize = strlen(text);

  if ( ( text[stringSize-2] == '\\' ) &&
       ( text[stringSize-1] == 'n' ) ) {
    text[stringSize-2] = '\n';
    text[stringSize-1] = 0;
    stringSize--;
  }

  if ( stringSize > bufSize ) return 0; // too big;

  newTotalSize = stringSize + totalSize;

  if ( newTotalSize > ( maxSize ) ) {

    num = newTotalSize - maxSize + bufExtra;

//     fprintf( stderr, "replace %-d characters\n", num );
    XmTextReplace( topScrolledText, 0, num, "" );
    totalSize -= num - 1;

  }

  XmTextInsert( topScrolledText, totalSize, text );
  totalSize += stringSize;

  if ( windowIsOpen ) {
    if ( autoRaiseWindow ) this->popup();
  }
  else {
    if ( autoOpenWindow ) this->popup();
  }

  return 1;

}

int scrolledTextClass::addText (
  char *text )
{

int stringSize, newTotalSize, num;

  stringSize = strlen(text);

  if ( stringSize > bufSize ) return 0; // too big;

  newTotalSize = stringSize + totalSize;

  if ( newTotalSize > ( maxSize ) ) {

    num = newTotalSize - maxSize + bufExtra;

//     fprintf( stderr, "replace %-d characters\n", num );
    XmTextReplace( topScrolledText, 0, num, "" );
    totalSize -= num - 1;

  }

  XmTextInsert( topScrolledText, totalSize, text );
  totalSize += stringSize;

  // there will always be room to add a linefeed
  if ( text[stringSize-1] != '\n' ) {
    XmTextInsert( topScrolledText, totalSize, "\n" );
    totalSize++;
  }

  if ( windowIsOpen ) {
    if ( autoRaiseWindow ) this->popup();
  }
  else {
    if ( autoOpenWindow ) this->popup();
  }

  return 1;

}

int scrolledTextClass::popup ( void ) {

Arg args[5];
int n;

 if ( ( x != 0 ) ) {
   n = 0;
   XtSetArg( args[n], XmNx, (XtArgVal) x ); n++;
   XtSetValues( shell, args, n );
 }

 if ( ( y != 0 ) ) {
   n = 0;
   XtSetArg( args[n], XmNy, (XtArgVal) y ); n++;
   XtSetValues( shell, args, n );
 }

  XtPopup( shell, XtGrabNone );

  windowIsOpen = 1;

  return 1;

}

int scrolledTextClass::popdown ( void ) {

  XtPopdown( shell );
  windowIsOpen = 0;

  return 1;

}

void scrolledTextClass::raise ( void ) {

  if ( windowIsOpen && autoRaiseWindow ) {
    XRaiseWindow( display, XtWindow(shell) );
  }

}

Widget scrolledTextClass::top ( void ) {

  return shell;

}

Widget scrolledTextClass::textWidget ( void ) {

  return topScrolledText;

}

Widget scrolledTextClass::paneWidget ( void ) {

  return pane;

}

Widget scrolledTextClass::formWidget ( void ) {

  return topForm;

}

Widget scrolledTextClass::clearPbWidget ( void ) {

  return clear_pb;

}

Widget scrolledTextClass::HorzScrollWidget ( void ) {

int n;
Arg args[2];
Widget w;

  n = 0;
  XtSetArg( args[n], XmNhorizontalScrollBar, &w ); n++;
  XtGetValues( XtParent(topScrolledText), args, n );

  return w;

}

Widget scrolledTextClass::VertScrollWidget ( void ) {

int n;
Arg args[2];
Widget w;

  n = 0;
  XtSetArg( args[n], XmNverticalScrollBar, &w ); n++;
  XtGetValues( XtParent(topScrolledText), args, n );

  return w;

}

int scrolledTextClass::autoOpen ( void ) {

  return autoOpenWindow;

}

void scrolledTextClass::setAutoOpen (
  int flag
) {

  autoOpenWindow = flag;

}

int scrolledTextClass::autoRaise ( void ) {

  return autoRaiseWindow;

}

void scrolledTextClass::setAutoRaise (
  int flag
) {

  autoRaiseWindow = flag;

}
