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

#ifndef __confirm_dialog_h
#define __confirm_dialog_h 1

#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/BulletinB.h>
#include <Xm/DrawingA.h>
#include <Xm/PushBG.h>
#include <Xm/ArrowBG.h>
#include <Xm/Label.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/DialogS.h>
#include <Xm/PanedW.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/Form.h>

#include "font_pkg.h"
#include "confirm_dialog.str"
#include "utility.h"

class confirmDialogClass {

private:

Display *display;

Widget shell, pane, labelForm, mainLabel, bottomForm, *pb;

int numButtons, maxButtons;

char *actionTag;
XmFontList actionFontList;

public:

int x, y;

confirmDialogClass ( void );

~confirmDialogClass ( void );

int destroy ( void );

int create (
  Widget top,
  char *widgetName,
  int _x,
  int _y,
  int _maxButtons,
  char *text,
  fontInfoClass *fi,
  const char *actionFontTag );

int addButton (
  char *label,
  XtCallbackProc cb,
  void *ptr );

int finished ( void );

int popup ( void );

int popdown ( void );

Widget top ( void );

};

#endif
