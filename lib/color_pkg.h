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

#ifndef __color_pkg_h
#define __color_pkg_h 1

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/MainW.h>
#include <Xm/RowColumn.h>
#include <Xm/DrawingA.h>
#include <Xm/Protocols.h>

#include "sys_types.h"
#include "avl.h"
#include "thread.h"
#include "gc_pkg.h"
#include "color_pkg.str"

#define COLORINFO_SUCCESS 1
#define COLORINFO_EMPTY 100
#define COLORINFO_FAIL 102
#define COLORINFO_NO_FILE 104
#define COLORINFO_NO_COLOR 106

#define NUM_SPECIAL_COLORS 7
#define NUM_BLINKING_COLORS 8
#define MAX_COLORS 88
#define NUM_COLOR_COLS 11

#define COLORINFO_K_DISCONNECTED 0
#define COLORINFO_K_INVALID 1
#define COLORINFO_K_MINOR 2
#define COLORINFO_K_MAJOR 3
#define COLORINFO_K_NOALARM 4

typedef struct colorCacheTag {
  AVL_FIELDS(colorCacheTag)
  unsigned int rgb[3]; // [0]=r, [1]=g, [2]=b
  unsigned int pixel;
  int index;
} colorCacheType, *colorCachePtr;

class colorInfoClass {

private:

friend void doColorBlink (
  XtPointer client,
  XtIntervalId *id );

friend void colorFormEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch );

friend class colorButtonClass;

int max_colors, num_blinking_colors, num_color_cols, usingPrivateColorMap;

AVL_HANDLE colorCacheByColorH;
AVL_HANDLE colorCacheByPixelH;
AVL_HANDLE colorCacheByIndexH;

Display *display;
int screen;
int depth;
Visual *visual;
Colormap cmap;
unsigned int fg;

/*  unsigned int *colors[MAX_COLORS+NUM_BLINKING_COLORS]; */
/*  unsigned long *blinkingColorCells[NUM_BLINKING_COLORS]; */
/*  XColor *blinkingXColor[NUM_BLINKING_COLORS], */
/*   *offBlinkingXColor[NUM_BLINKING_COLORS]; */

unsigned int *colors;
unsigned long *blinkingColorCells;
XColor *blinkingXColor, *offBlinkingXColor;

int special[NUM_SPECIAL_COLORS];
int numColors, blink;

int curIndex, curX, curY;

XtAppContext appCtx;
XtIntervalId incrementTimer;
int incrementTimerValue;

Widget activeWidget;
unsigned int *curDestination;

gcClass gc;

Widget shell, rc, mbar, mb1, mb2, form, rc1, rc2, fgpb, bgpb;

public:

int change;

int colorWindowIsOpen;

colorInfoClass::colorInfoClass ( void );   // constructor

colorInfoClass::~colorInfoClass ( void );   // destructor

int colorInfoClass::colorChanged ( void ) {
int i;
  i = change;
  change = 0;
  return i;
}

int colorInfoClass::initFromFile (
  XtAppContext app,
  Display *d,
  Widget top,
  char *fileName );

int colorInfoClass::openColorWindow( void );

int colorInfoClass::closeColorWindow( void );

unsigned int colorInfoClass::getFg( void );

void colorInfoClass::setCurDestination( unsigned int *ptr );

unsigned int *colorInfoClass::getCurDestination( void );

int colorInfoClass::setActiveWidget( Widget w );

Widget colorInfoClass::getActiveWidget( void );

Widget colorInfoClass::createColorButton(
  Widget parent,
  Arg args[],
  int num_args );

int colorInfoClass::getRGB (
  unsigned int pixel,
  int *r,
  int *g,
  int *b );

int colorInfoClass::setRGB (
  int r,
  int g,
  int b,
  unsigned int *pixel );

int colorInfoClass::getIndex (
  unsigned int pixel,
  int *index );

int colorInfoClass::setIndex (
  int index,
  unsigned int *pixel );

int colorInfoClass::getSpecialColor (
  int index );

Colormap colorInfoClass::getColorMap ( void );

int colorInfoClass::setCurIndexByPixel (
  unsigned int pixel );

int colorInfoClass::canDiscardPixel (
  unsigned int pixel );

unsigned int colorInfoClass::getPixelByIndex (
  int index );

};

#endif
