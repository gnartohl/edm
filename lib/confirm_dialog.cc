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

#include "confirm_dialog.h"

#include "thread.h"

confirmDialogClass::confirmDialogClass ( void ) {

  maxButtons = 0;
  numButtons = 0;
  actionTag = (char *) NULL;
  actionFontList = NULL;
  pb = (Widget *) NULL;

}

confirmDialogClass::~confirmDialogClass ( void ) {

  if ( actionTag ) delete actionTag;
  if ( pb ) delete pb;

}

int confirmDialogClass::destroy ( void ) {

  XtDestroyWidget( shell );

  if ( actionTag ) delete actionTag;
  actionTag = (char *) NULL;

  if ( actionFontList ) XmFontListFree( actionFontList );
  actionFontList = NULL;

  if ( pb ) delete pb;
  pb = (Widget *) NULL;

  maxButtons = 0;
  numButtons = 0;

  return 1;

}

int confirmDialogClass::create (
  Widget top,
  char *widgetName,
  int _x,
  int _y,
  int _maxButtons,
  char *text,
  fontInfoClass *fi,
  const char *actionFontTag )
{

XmString str;

   x = _x;
   y = _y;
   maxButtons = _maxButtons;
   numButtons = 0;

  pb = (Widget *) new Widget[maxButtons];
  if ( !pb ) maxButtons = 0;

  display = XtDisplay( top );

  if ( fi ) {

    if ( actionFontTag ) {
      actionTag = new char[strlen(actionFontTag)+1];
      strcpy( actionTag, actionFontTag );
      fi->getTextFontList( actionTag, &actionFontList );
    }

  }

  shell = XtVaCreatePopupShell( widgetName, xmDialogShellWidgetClass,
   top,
   XmNmappedWhenManaged, False,
   NULL );

  pane = XtVaCreateWidget( "pane", xmPanedWindowWidgetClass, shell,
   XmNsashWidth, 1,
   XmNsashHeight, 1,
   NULL );

  labelForm = XtVaCreateWidget( "labelform", xmFormWidgetClass, pane, NULL );

  if ( actionTag )
    str = XmStringCreate( text, actionTag );
  else
    str = XmStringCreateLocalized( text );

  if ( actionFontList ) {
    mainLabel = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
     labelForm,
     XmNlabelString, str,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     XmNleftAttachment, XmATTACH_FORM,
     XmNfontList, actionFontList,
     NULL );
  }
  else {
    mainLabel = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
     labelForm,
     XmNlabelString, str,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     XmNleftAttachment, XmATTACH_FORM,
     NULL );
  }

  XmStringFree( str );

  bottomForm = XtVaCreateWidget( "botform", xmFormWidgetClass, pane,
   NULL );

  return 1;

}

int confirmDialogClass::addButton (
  char *label,
  XtCallbackProc cb,
  void *ptr )
{

XmString str;

  if ( numButtons >= maxButtons ) return 0;

  if ( actionTag )
    str = XmStringCreate( label, actionTag );
  else
    str = XmStringCreateLocalized( label );

  if ( numButtons == 0 ) {

    if ( actionFontList ) {
      pb[numButtons] = XtVaCreateManagedWidget( "pb", xmPushButtonGadgetClass,
       bottomForm,
       XmNtopAttachment, XmATTACH_FORM,
       XmNbottomAttachment, XmATTACH_FORM,
       XmNrightAttachment, XmATTACH_FORM,
       XmNdefaultButtonShadowThickness, 1,
       XmNlabelString, str,
       XmNfontList, actionFontList,
       NULL );
    }
    else {
      pb[numButtons] = XtVaCreateManagedWidget( "pb", xmPushButtonGadgetClass,
       bottomForm,
       XmNtopAttachment, XmATTACH_FORM,
       XmNbottomAttachment, XmATTACH_FORM,
       XmNrightAttachment, XmATTACH_FORM,
       XmNdefaultButtonShadowThickness, 1,
       XmNlabelString, str,
       NULL );
    }

  }
  else {

    if ( actionFontList ) {
      pb[numButtons] = XtVaCreateManagedWidget( "pb", xmPushButtonGadgetClass,
       bottomForm,
       XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
       XmNtopWidget, pb[numButtons-1],
       XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
       XmNbottomWidget, pb[numButtons-1],
       XmNrightAttachment, XmATTACH_WIDGET,
       XmNrightWidget, pb[numButtons-1],
       XmNdefaultButtonShadowThickness, 1,
       XmNlabelString, str,
       XmNfontList, actionFontList,
       NULL );
    }
    else {
      pb[numButtons] = XtVaCreateManagedWidget( "pb", xmPushButtonGadgetClass,
       bottomForm,
       XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
       XmNtopWidget, pb[numButtons-1],
       XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
       XmNbottomWidget, pb[numButtons-1],
       XmNrightAttachment, XmATTACH_WIDGET,
       XmNrightWidget, pb[numButtons-1],
       XmNdefaultButtonShadowThickness, 1,
       XmNlabelString, str,
       NULL );
    }

  }

  XmStringFree( str );

  XtAddCallback( pb[numButtons], XmNactivateCallback, cb, ptr );

  numButtons++;

  return 1;

}

int confirmDialogClass::finished ( void )
{

  XtManageChild( labelForm );
  XtManageChild( bottomForm );
  XtManageChild( pane );

  return 1;

}

int confirmDialogClass::popup ( void ) {

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

  return 1;

}

int confirmDialogClass::popdown ( void ) {

  XtPopdown( shell );

  this->destroy();

  return 1;

}

Widget confirmDialogClass::top ( void ) {

  return shell;

}
