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

#ifndef __cursor_h
#define __cursor_h 1

#include <X11/Xlib.h>

#define CURSOR_K_DEFAULT 1
#define CURSOR_K_CROSSHAIR 2
#define CURSOR_K_TINYCROSSHAIR 3
#define CURSOR_K_WAIT 4
#define CURSOR_K_NO 5
#define CURSOR_K_WILL_SET 6
#define CURSOR_K_WILL_OPEN 7
#define CURSOR_K_WILL_OPEN_WITH_HELP 8
#define CURSOR_K_RUN 9
#define CURSOR_K_RUN_WITH_HELP 10
#define CURSOR_K_UPDOWN 11
#define CURSOR_K_PNTR_WITH_HELP 12

class cursorClass {

private:

Display *display;
Colormap colormap;

Pixmap crossHairShape, crossHairMask;
XColor shapeColor, maskColor;

Pixmap tinyCrossHairShape, tinyCrossHairMask;
XColor tinyCrossHairShapeColor, tinyCrossHairMaskColor;

Pixmap waitShape, waitMask;
XColor waitShapeColor, waitMaskColor;

Pixmap noShape, noMask;
XColor noShapeColor, noMaskColor;

Pixmap willSetShape, willSetMask;
XColor willSetShapeColor, willSetMaskColor;

Pixmap willOpenShape, willOpenMask;
XColor willOpenShapeColor, willOpenMaskColor;

Pixmap willOpenWithHelpShape, willOpenWithHelpMask;
XColor willOpenWithHelpShapeColor, willOpenWithHelpMaskColor;

Pixmap runShape, runMask;
XColor runShapeColor, runMaskColor;

Pixmap runWithHelpShape, runWithHelpMask;
XColor runWithHelpShapeColor, runWithHelpMaskColor;

Pixmap upDownShape, upDownMask;
XColor upDownShapeColor, upDownMaskColor;

Pixmap pntrWithHelpShape, pntrWithHelpMask;
XColor pntrWithHelpShapeColor, pntrWithHelpMaskColor;

Cursor curCursor, crossHair, tinyCrossHair, wait, no, willSet,
 willOpen, willOpenWithHelp, run, runWithHelp, upDown, pntrWithHelp;

public:

cursorClass ( void );

~cursorClass ( void );

void create (
  Display *dsp,
  Window rootWin,
  Colormap cmap );

int set (
  Window win,
  int cursorId );

int setColor (
  unsigned int fg,
  unsigned int bg );

};

#endif
