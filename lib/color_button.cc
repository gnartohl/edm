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

void destroy_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

colorInfoClass *ci;

//   printf( "In destroy_widget_cb\n" );

  ci = (colorInfoClass *) client;
  ci->setActiveWidget( NULL );
  ci->setCurDestination( NULL );

}

colorButtonClass::colorButtonClass ( void ) {

  form = NULL;
  pb = NULL;
  tf = NULL;
  colorPvName = NULL;

}

colorButtonClass::colorButtonClass (
 const colorButtonClass &source )
{

  form = source.form;
  pb = source.pb;
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

}

colorButtonClass colorButtonClass::operator = (
  const colorButtonClass &source ) {

  form = source.form;
  pb = source.pb;
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

  return *this;

}

colorButtonClass::~colorButtonClass ( void ) {

//   printf( "In colorButtonClass::~colorButtonClass\n" );

  if ( colorPvName ) {
    delete colorPvName;
    colorPvName = NULL;
  }

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

  stat = ci->setActiveWidget( w );

  ci->setCurDestination( cb->destination() );

  ci->openColorWindow();

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

  XtAddCallback( pb, XmNactivateCallback, setActive_cb, (XtPointer) this );
  XtAddCallback( pb, XmNdestroyCallback, destroy_cb, (XtPointer) this->ci );


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

  form = XtCreateManagedWidget( "", xmFormWidgetClass, parent,
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

  tf = XtCreateManagedWidget( "", xmTextWidgetClass, form,
    tArgs, tNum_args );

  destPtr = dest;

  curIndex = *dest;

  XtAddCallback( pb, XmNactivateCallback, setActive_cb, (XtPointer) this );
  XtAddCallback( pb, XmNdestroyCallback, destroy_cb, (XtPointer) this->ci );

  return form;

}

Widget colorButtonClass::widget ( void ) {

  return pb;

}

Widget colorButtonClass::formWidget ( void ) {

  return form;

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

  curIndex = i;

  n = 0;
  XtSetArg( arg[n], XmNbackground, (XtArgVal) ci->pix(i) ); n++;
  XtSetValues( pb, arg, n );

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
