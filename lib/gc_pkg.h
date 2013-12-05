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

#ifndef __gc_pkg_h
#define __gc_pkg_h 1

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <Xm/Xm.h>
#include <X11/Xlib.h>

#include "font_pkg.h"
#include "gc_pkg.str"

#define GC_SUCCESS 1
#define GC_FAIL 100
#define GC_NO_FONT 102
#define GC_STACK_OVFLO 104
#define GC_STACK_UNFLO 106

#define GC_STACK_MAX 5

class colorInfoClass;
class activeGraphicClass;

class gcClass {

private:

Display *display;
Widget baseWidget;
GC norm, x_or, erase, invert;
unsigned int fg, bg, baseBg;
unsigned int savFg, savBg, savBaseBg;
char curFontTag[127+1];
int curLineWidth, curLineStyle, curLineCapStyle, curLineJoinStyle, curArcMode;
int normStackPtr;
XRectangle normXRecStack[GC_STACK_MAX];
int eraseStackPtr;
XRectangle eraseXRecStack[GC_STACK_MAX];
int xorStackPtr;
XRectangle xorXRecStack[GC_STACK_MAX];
colorInfoClass *ci;

public:

gcClass ( void );

~gcClass ( void );

void setCI (
  colorInfoClass *_ci
);

int create (
  Widget top );

void clipMinimumArea (
  GC gc,
  XRectangle *xRectArray,
  int num );

int addNormXClipRectangle (
  XRectangle xR );

int removeNormXClipRectangle ( void );

int addEraseXClipRectangle (
  XRectangle xR );

int removeEraseXClipRectangle ( void );

int addXorXClipRectangle (
  XRectangle xR );

int removeXorXClipRectangle ( void );

GC normGC ( void );

GC xorGC ( void );

GC eraseGC ( void );

GC invertGC ( void );

void saveFg ( void );

void saveBg ( void );

void saveBaseBg ( void );

void restoreFg ( void );

void restoreBg ( void );

void restoreBaseBg ( void );

int setFG (
  unsigned int fg );

int setFG (
  int fgIndex,
  int *blink );

void updateBlink (
  activeGraphicClass *ago,
  int blink );

int setBG (
  unsigned int bg );

int setBG (
  int bgIndex,
  int *blink );

int setBaseBG (
  unsigned int bg );

unsigned int getBaseBG ( void );

int setFontTag (
  char *fontTag,
  fontInfoClass *fi );

int setNativeFont (
  char *fontName,
  fontInfoClass *fi );

int setLineWidth (
  int width );

int setLineStyle (
  int style );

int setLineCapStyle (
  int style );

int setLineJoinStyle (
  int style );

int setArcModeChord ( void );

int setArcModePieSlice ( void );

int setGraphicsExposures (
  int state );

int setFGforGivenBG (
  unsigned int fgColor,
  unsigned int bgColor );

};

#endif
