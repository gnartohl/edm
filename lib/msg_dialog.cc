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

#include "msg_dialog.h"

#include "thread.h"

msgDialogClass::msgDialogClass ( void ) {

  shell = (Widget) NULL;
  winOpen = 0;

}

msgDialogClass::~msgDialogClass ( void ) {

}

int msgDialogClass::destroy ( void ) {

  if ( !shell ) return 1;

  XtDestroyWidget( shell );
  shell = (Widget) NULL;

  return 1;

}

int msgDialogClass::create (
  Widget top )
{

XmString str;

  display = XtDisplay( top );

  shell = XtVaCreatePopupShell( "msg", xmDialogShellWidgetClass,
   top,
   XmNmappedWhenManaged, False,
   XmNmwmDecorations, 0,
   XmNoverrideRedirect, True,
   NULL );

  labelForm = XtVaCreateWidget( "labelform", xmFormWidgetClass, shell, NULL );

  str = XmStringCreateLocalized( "  " );

  label = XtVaCreateWidget( "label", xmLabelWidgetClass,
   labelForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   XmNleftAttachment, XmATTACH_FORM,
   XtNbackground, WhitePixel(display,DefaultScreen(display)),
   XtNforeground, BlackPixel(display,DefaultScreen(display)),
   NULL );

  XmStringFree( str );

  XtManageChild( label );
  XtManageChild( labelForm );

  return 1;

}

int msgDialogClass::createWithOffset (
  Widget top )
{

XmString str;

  display = XtDisplay( top );

  shell = XtVaCreatePopupShell( "msg", xmDialogShellWidgetClass,
   top,
   XmNmappedWhenManaged, False,
   XmNmwmDecorations, 0,
   //XmNoverrideRedirect, True,
   NULL );

  labelForm = XtVaCreateWidget( "labelform", xmFormWidgetClass, shell, NULL );

  str = XmStringCreateLocalized( "  " );

  label = XtVaCreateWidget( "label", xmLabelWidgetClass,
   labelForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   XmNleftAttachment, XmATTACH_FORM,
   XmNbottomAttachment, XmATTACH_FORM,
   XmNleftOffset, 20,
   XmNrightOffset, 20,
   XmNtopOffset, 20,
   XmNbottomOffset, 20,
   NULL );

  XmStringFree( str );

  XtManageChild( label );
  XtManageChild( labelForm );

  return 1;

}

int msgDialogClass::popup (
  char *text,
  int _x,
  int _y
) {

Arg args[5];
int n;
XmString str;

  if ( winOpen ) {

    XtPopdown( shell );

  }

  str = XmStringCreateLocalized( text );
  n = 0;
  XtSetArg( args[n], XmNlabelString, (XtArgVal) str ); n++;
  XtSetValues( label, args, n );
  XmStringFree( str );

  XtUnmanageChild( label );
  XtUnmanageChild( labelForm );

  XtManageChild( label );
  XtManageChild( labelForm );

  n = 0;
  XtSetArg( args[n], XmNx, (XtArgVal) _x ); n++;
  XtSetValues( shell, args, n );
  XtSetArg( args[n], XmNy, (XtArgVal) _y ); n++;
  XtSetValues( shell, args, n );

  winOpen = 1;
  XtPopup( shell, XtGrabNone );
  XRaiseWindow( display, XtWindow(shell) );

  return 1;

}

int msgDialogClass::popdown ( void ) {

  winOpen = 0;
  XtPopdown( shell );

  return 1;

}

Widget msgDialogClass::top ( void ) {

  return shell;

}
