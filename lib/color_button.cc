#define __color_button_cc 1

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

#include "color_button.h"

#include "thread.h"

static void doCbBlink (
  void *ptr
) {

colorButtonClass *cb = (colorButtonClass *) ptr;
Arg arg[10];
int n;
unsigned int bg;

  bg = cb->ci->pixWblink(cb->curIndex);

  n = 0;
  XtSetArg( arg[n], XmNbackground, (XtArgVal) bg ); n++;
  XtSetValues( cb->pb, arg, n );

}

static void destroy_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

colorInfoClass *ci;
colorButtonClass *cb = (colorButtonClass *) client;

  // fprintf( stderr, "In destroy_widget_cb\n" );

  if ( cb->blink ) {
    cb->ci->removeFromBlinkList( (void *) cb, (void *) doCbBlink );
    cb->blink = 0;
  }

  ci = cb->ci;
  ci->setActiveWidget( NULL );
  ci->setNameWidget( NULL );
  ci->setCurDestination( NULL );
  ci->setCurCb( NULL );

}

colorButtonClass::colorButtonClass ( void ) {

  form = NULL;
  pb = NULL;
  namePb = NULL;
  tf = NULL;
  colorPvName = NULL;
  blink = 0;

}

colorButtonClass::colorButtonClass (
 const colorButtonClass &source )
{

  form = source.form;
  pb = source.pb;
  namePb = source.namePb;
  tf = source.tf;
  destPtr = source.destPtr;
  ci = source.ci;
  if ( source.colorPvName ) {
    colorPvName = new char[128];
    strncpy( colorPvName, source.colorPvName, 127 );
    colorPvName[127] = 0;
  }
  else {
    colorPvName = NULL;
  }
  blink = 0;

}

colorButtonClass colorButtonClass::operator = (
  const colorButtonClass &source ) {

  form = source.form;
  pb = source.pb;
  namePb = source.namePb;
  tf = source.tf;
  destPtr = source.destPtr;
  ci = source.ci;
  if ( source.colorPvName ) {
    colorPvName = new char[128];
    strncpy( colorPvName, source.colorPvName, 127 );
    colorPvName[127] = 0;
  }
  else {
    colorPvName = NULL;
  }
  blink = 0;

  return *this;

}

colorButtonClass::~colorButtonClass ( void ) {

//   fprintf( stderr, "In colorButtonClass::~colorButtonClass\n" );

  if ( colorPvName ) {
    delete[] colorPvName;
    colorPvName = NULL;
  }

  if ( blink ) {
    ci->removeFromBlinkList( (void *) this, (void *) doCbBlink );
    blink = 0;
  }

}

void colorButtonClass::enable ( void ) {

  if ( pb ) XtSetSensitive( pb, True );
  if ( namePb ) XtSetSensitive( namePb, True );

}

void colorButtonClass::disable ( void ) {

  if ( pb ) XtSetSensitive( pb, False );
  if ( namePb ) XtSetSensitive( namePb, False );

}

static void setActive_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

colorInfoClass *ci;
colorButtonClass *cb;
int stat;
int curIndex;

  cb = (colorButtonClass *) client;

  curIndex = cb->getIndex();

  ci = cb->colorInfo();

  stat = ci->setCurIndex( curIndex );

  stat = ci->setActiveWidget( cb->widget() );

  ci->setNameWidget( cb->nameWidget() );

  ci->setCurDestination( cb->destination() );

  ci->setCurCb( cb );

  if ( ci->menuPosition(curIndex) ) {
    XmListSelectPos( ci->colorList.listWidget(),
     ci->menuPosition(curIndex), FALSE );
    XmListSetBottomPos( ci->colorList.listWidget(),
     ci->menuPosition(curIndex) );
  }
  else {
    XmListDeselectAllItems( ci->colorList.listWidget() );
  }

  ci->openColorWindow();

}

static void nameSetActive_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

colorInfoClass *ci;
colorButtonClass *cb;
int stat;
int curIndex;

  cb = (colorButtonClass *) client;

  curIndex = cb->getIndex();

  ci = cb->colorInfo();

  stat = ci->setCurIndex( curIndex );

  stat = ci->setActiveWidget( cb->widget() );

  ci->setNameWidget( cb->nameWidget() );

  ci->setCurDestination( cb->destination() );

  ci->setCurCb( cb );

  if ( ci->menuPosition(curIndex) ) {
    XmListSelectPos( ci->colorList.listWidget(),
     ci->menuPosition(curIndex), FALSE );
    XmListSetBottomPos( ci->colorList.listWidget(),
     ci->menuPosition(curIndex) );
  }
  else {
    XmListDeselectAllItems( ci->colorList.listWidget() );
  }

  ci->colorList.popup();

}

Widget colorButtonClass::create(
  Widget parent,
  int *dest,
  colorInfoClass *ptr,
  Arg args[],
  int num_args )
{

  ci = ptr;


  pb = XtCreateManagedWidget( "", xmPushButtonWidgetClass, parent, args,
   num_args );

  destPtr = dest;

  curIndex = *dest;

  if ( ci->blinking(curIndex) ) {
    if ( !blink ) {
      ci->addToBlinkList( (void *) this, (void *) doCbBlink );
      blink = 1;
    }
  }

  XtAddCallback( pb, XmNactivateCallback, setActive_cb, (XtPointer) this );
  XtAddCallback( pb, XmNdestroyCallback, destroy_cb, (XtPointer) this );


  return pb;

}

Widget colorButtonClass::createWithText(
  Widget parent,
  int *dest,
  colorInfoClass *ptr,
  char *pvName,
  Arg fArgs[],
  int fNum_args,
  Arg bArgs[],
  int bNum_args,
  Arg tArgs[],
  int tNum_args )
{

  if ( !colorPvName ) colorPvName = new char[128];

  if ( pvName ) {
    strncpy( colorPvName, pvName, 127 );
    colorPvName[127] = 0;
  }
  else {
    strcpy( colorPvName, "" );
  }

  ci = ptr;

  form = XtCreateManagedWidget( "form", xmFormWidgetClass, parent,
   fArgs, fNum_args );

  pb = XtCreateManagedWidget( "", xmPushButtonWidgetClass, form,
   bArgs, bNum_args );

  XtSetArg( tArgs[tNum_args], XmNleftOffset, (XtArgVal) 5 ); tNum_args++;
  XtSetArg( tArgs[tNum_args], XmNtopAttachment,
   (XtArgVal) XmATTACH_OPPOSITE_WIDGET ); tNum_args++;
  XtSetArg( tArgs[tNum_args], XmNtopWidget, (XtArgVal) pb ); tNum_args++;
  XtSetArg( tArgs[tNum_args], XmNleftAttachment,
   (XtArgVal) XmATTACH_WIDGET ); tNum_args++;
  XtSetArg( tArgs[tNum_args], XmNleftWidget, (XtArgVal) pb ); tNum_args++;
  XtSetArg( tArgs[tNum_args], XmNvalue, colorPvName ); tNum_args++;
  XtSetArg( tArgs[tNum_args], XmNmaxLength, (short) PvSize() ); tNum_args++;

  tf = XtCreateManagedWidget( "text", xmTextWidgetClass, form,
   tArgs, tNum_args );

  destPtr = dest;

  curIndex = *dest;

  if ( ci->blinking(curIndex) ) {
    if ( !blink ) {
      ci->addToBlinkList( (void *) this, (void *) doCbBlink );
      blink = 1;
    }
  }

  XtAddCallback( pb, XmNactivateCallback, setActive_cb, (XtPointer) this );
  XtAddCallback( pb, XmNdestroyCallback, destroy_cb, (XtPointer) this );

  return form;

}

Widget colorButtonClass::createWithRule(
  Widget parent,
  int *dest,
  colorInfoClass *ptr,
  char *pvName,
  Arg fArgs[],
  int fNum_args,
  Arg bArgs[],
  int bNum_args,
  Arg nbArgs[],
  int nbNum_args,
  Arg tArgs[],
  int tNum_args )
{

  if ( !colorPvName ) colorPvName = new char[128];

  if ( pvName ) {
    strncpy( colorPvName, pvName, 127 );
    colorPvName[127] = 0;
  }
  else {
    strcpy( colorPvName, "" );
  }

  ci = ptr;

  form = XtCreateManagedWidget( "form", xmFormWidgetClass, parent,
   fArgs, fNum_args );

  pb = XtCreateManagedWidget( "", xmPushButtonWidgetClass, form,
   bArgs, bNum_args );

  XtAddCallback( pb, XmNactivateCallback, setActive_cb, (XtPointer) this );
  XtAddCallback( pb, XmNdestroyCallback, destroy_cb, (XtPointer) this );

  if ( ci->majorVersion() >= 3 ) {

    XtSetArg( nbArgs[nbNum_args], XmNtopAttachment,
     (XtArgVal) XmATTACH_OPPOSITE_WIDGET ); nbNum_args++;
    XtSetArg( nbArgs[nbNum_args], XmNtopWidget, (XtArgVal) pb ); nbNum_args++;
    XtSetArg( nbArgs[nbNum_args], XmNleftOffset, (XtArgVal) 10 ); nbNum_args++;
    XtSetArg( nbArgs[nbNum_args], XmNleftAttachment,
     (XtArgVal) XmATTACH_WIDGET ); nbNum_args++;
    XtSetArg( nbArgs[nbNum_args], XmNleftWidget,
     (XtArgVal) pb ); nbNum_args++;

    namePb = XtCreateManagedWidget( "pb", xmPushButtonWidgetClass, form,
     nbArgs, nbNum_args );

    XtAddCallback( namePb, XmNactivateCallback, nameSetActive_cb,
     (XtPointer) this );

  }

  if ( pvName ) {

    // XtSetArg( tArgs[tNum_args], XmNleftOffset, (XtArgVal) 5 ); tNum_args++;
    XtSetArg( tArgs[tNum_args], XmNtopAttachment,
     (XtArgVal) XmATTACH_WIDGET ); tNum_args++;
    XtSetArg( tArgs[tNum_args], XmNtopWidget, (XtArgVal) pb ); tNum_args++;
    XtSetArg( tArgs[tNum_args], XmNleftAttachment,
     (XtArgVal) XmATTACH_FORM ); tNum_args++;
    XtSetArg( tArgs[tNum_args], XmNvalue, colorPvName ); tNum_args++;
    XtSetArg( tArgs[tNum_args], XmNmaxLength, (short) PvSize() ); tNum_args++;

    tf = XtCreateManagedWidget( "text", xmTextWidgetClass, form,
     tArgs, tNum_args );

  }

  destPtr = dest;

  curIndex = *dest;

  if ( ci->blinking(curIndex) ) {
    if ( !blink ) {
      ci->addToBlinkList( (void *) this, (void *) doCbBlink );
      blink = 1;
    }
  }

  return form;

}

Widget colorButtonClass::widget ( void ) {

  return pb;

}

Widget colorButtonClass::formWidget ( void ) {

  return form;

}

Widget colorButtonClass::nameWidget ( void ) {

  return namePb;

}

Widget colorButtonClass::textWidget ( void ) {

  return tf;

}

colorInfoClass *colorButtonClass::colorInfo( void ) {

  return ci;

}

int *colorButtonClass::destination( void ) {

  return destPtr;

}

unsigned int colorButtonClass::getPixel ( void ) {
Arg arg[10];
int n;
unsigned int fg;

  n = 0;
  XtSetArg( arg[n], XmNbackground, (XtArgVal) &fg ); n++;
  XtGetValues( pb, arg, n );

  return fg;

}

int colorButtonClass::setPixel (
  unsigned int p )
{

Arg arg[10];
int n;

  n = 0;
  XtSetArg( arg[n], XmNbackground, (XtArgVal) p ); n++;
  XtSetValues( pb, arg, n );

  return COLORBUTTON_SUCCESS;

}


int colorButtonClass::getIndex ( void ) {

  return curIndex;

}

int colorButtonClass::setIndex (
  int i )
{

Arg arg[10];
int n;
unsigned int fg, bg;
XmString str;

  curIndex = i;
  bg = ci->pixWblink(i);

  if ( ci->blinking(i) ) {
    if ( !blink ) {
      ci->addToBlinkList( (void *) this, (void *) doCbBlink );
      blink = 1;
    }
  }
  else {
    if ( blink ) {
      ci->removeFromBlinkList( (void *) this, (void *) doCbBlink );
      blink = 0;
    }
  }

  fg = ci->labelPix(i);

  if ( ci->isRule(i) ) {
    str = XmStringCreateLocalized( "*" );
  }
  else {
    str = XmStringCreateLocalized( " " );
  }

  n = 0;
  XtSetArg( arg[n], XmNbackground, (XtArgVal) bg ); n++;
  XtSetArg( arg[n], XmNforeground, (XtArgVal) fg ); n++;
  XtSetArg( arg[n], XmNlabelString, (XtArgVal) str ); n++;
  XtSetArg( arg[n], XmNwidth, (XtArgVal) 25 ); n++;
  XtSetArg( arg[n], XmNheight, (XtArgVal) 25 ); n++;
  XtSetArg( arg[n], XmNrecomputeSize, (XtArgVal) 0 ); n++;
  XtSetValues( pb, arg, n );

  XmStringFree( str );

  if ( namePb ) {
    str = XmStringCreateLocalized( ci->colorName(i) );
    n = 0;
    XtSetArg( arg[n], XmNlabelString, (XtArgVal) str ); n++;
    XtSetValues( namePb, arg, n );
    XmStringFree( str );
  }

  return COLORBUTTON_SUCCESS;

}

void colorButtonClass::setPv (
  char *name )
{

  strncpy( colorPvName, name, 127 );
  colorPvName[127] = 0;

}

char *colorButtonClass::getPv ( void )
{

  return colorPvName;

}

int colorButtonClass::PvSize ( void )
{

  return 127;

}
