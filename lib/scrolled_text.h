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

#ifndef __scrolled_text_h
#define __scrolled_text_h 1

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

#include "font_pkg.h"

#include "scrolled_text.str"

class scrolledTextClass {

private:

friend void stc_dismiss (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void stc_clear (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void stc_toggle_autoOpen (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void stc_toggle_autoRaise (
  Widget w,
  XtPointer client,
  XtPointer call );

Display *display;

Widget shell, pane, topScrolledText, topForm, dismiss_pb, clear_pb,
 autoOpen_tb, autoRaise_tb;

char *textTag;
XmFontList textFontList;

int bufSize, bufExtra, maxSize, totalSize, autoOpenWindow, autoRaiseWindow,
 windowIsOpen;

public:

int x, y;

scrolledTextClass::scrolledTextClass ( void );

scrolledTextClass::~scrolledTextClass ( void );

int scrolledTextClass::destroy ( void );

int scrolledTextClass::destroyEmbedded ( void );

int scrolledTextClass::create (
  Widget top,
  int _x,
  int _y,
  int _bufSize,
  fontInfoClass *fi,
  const char *textFontTag );

int scrolledTextClass::createEmbedded (
  Widget top,
  int _x,
  int _y,
  int nRows,
  int nCols,
  int _bufSize,
  fontInfoClass *fi,
  const char *textFontTag );

int scrolledTextClass::createEmbeddedWH (
  Widget top,
  int _x,
  int _y,
  int _w,
  int _h,
  int _bufSize,
  fontInfoClass *fi,
  const char *textFontTag );

int scrolledTextClass::addTextNoNL (
  char *text );

int scrolledTextClass::addText (
  char *text );

int scrolledTextClass::popup ( void );

int scrolledTextClass::popdown ( void );

Widget scrolledTextClass::top ( void );

Widget scrolledTextClass::textWidget ( void );

Widget scrolledTextClass::paneWidget ( void );

Widget scrolledTextClass::formWidget ( void );

Widget scrolledTextClass::clearPbWidget ( void );

Widget scrolledTextClass::HorzScrollWidget ( void );

Widget scrolledTextClass::VertScrollWidget ( void );

int scrolledTextClass::autoOpen ( void );

int scrolledTextClass::autoRaise ( void );

};

#endif
