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

gcClass::gcClass ( void );

gcClass::~gcClass ( void );

void gcClass::setCI (
  colorInfoClass *_ci
);

int gcClass::create (
  Widget top );

void gcClass::clipMinimumArea (
  GC gc,
  XRectangle *xRectArray,
  int num );

int gcClass::addNormXClipRectangle (
  XRectangle xR );

int gcClass::removeNormXClipRectangle ( void );

int gcClass::addEraseXClipRectangle (
  XRectangle xR );

int gcClass::removeEraseXClipRectangle ( void );

int gcClass::addXorXClipRectangle (
  XRectangle xR );

int gcClass::removeXorXClipRectangle ( void );

GC gcClass::normGC ( void );

GC gcClass::xorGC ( void );

GC gcClass::eraseGC ( void );

GC gcClass::invertGC ( void );

void gcClass::saveFg ( void );

void gcClass::saveBg ( void );

void gcClass::saveBaseBg ( void );

void gcClass::restoreFg ( void );

void gcClass::restoreBg ( void );

void gcClass::restoreBaseBg ( void );

int gcClass::setFG (
  unsigned int fg );

int gcClass::setFG (
  int fgIndex,
  int *blink );

void gcClass::updateBlink (
  activeGraphicClass *ago,
  int blink );

int gcClass::setBG (
  unsigned int bg );

int gcClass::setBaseBG (
  unsigned int bg );

unsigned int gcClass::getBaseBG ( void );

int gcClass::setFontTag (
  char *fontTag,
  fontInfoClass *fi );

int gcClass::setNativeFont (
  char *fontName,
  fontInfoClass *fi );

int gcClass::setLineWidth (
  int width );

int gcClass::setLineStyle (
  int style );

int gcClass::setLineCapStyle (
  int style );

int gcClass::setLineJoinStyle (
  int style );

int gcClass::setArcModeChord ( void );

int gcClass::setArcModePieSlice ( void );

int gcClass::setGraphicsExposures (
  int state );

int gcClass::setFGforGivenBG (
  unsigned int fgColor,
  unsigned int bgColor );

};

#endif
