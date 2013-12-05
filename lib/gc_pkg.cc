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

#include "gc_pkg.h"
#include "color_pkg.h"
#include "act_grf.h"

#include "thread.h"

gcClass::gcClass ( void ) {

  fg = 0x7fffffff;
  bg = 0x7fffffff;
  baseBg = 0x7fffffff;
  curLineWidth = -1;
  curLineStyle = -1;
  curLineCapStyle = -1;
  curLineJoinStyle = -1;
  curArcMode = -1;
  strcpy( curFontTag, "?" );

  norm = NULL;
  x_or = NULL;
  erase = NULL;
  invert = NULL;

  normStackPtr = 0;
  eraseStackPtr = 0;
  xorStackPtr = 0;

  ci = NULL;

}

gcClass::~gcClass ( void ) {

  if ( norm ) XFreeGC( display, norm );
  if ( x_or ) XFreeGC( display, x_or );
  if ( erase ) XFreeGC( display, erase );
  if ( invert ) XFreeGC( display, invert );

}

void gcClass::setCI (
  colorInfoClass *_ci
) {

  ci = _ci;

}

int gcClass::create (
  Widget base )
{

int screen;
XGCValues values;

  this->baseWidget = base;

  display = XtDisplay( base );
  screen = DefaultScreen( display );

  XtVaGetValues( base,
   XtNbackground, &values.background,
   NULL );

  values.foreground = BlackPixel(display,screen);

  fg = values.foreground;
  bg = values.background;
  baseBg = values.background;

  norm = XCreateGC( display, RootWindow(display,screen),
   GCForeground|GCBackground, &values );

  values.foreground = fg ^ baseBg;
  values.background = bg ^ baseBg;
  values.function = GXxor;

  x_or = XCreateGC( display, RootWindow(display,screen),
   GCForeground|GCBackground|GCFunction, &values );

  values.background = bg;
  values.foreground = bg;

  erase = XCreateGC( display, RootWindow(display,screen),
   GCForeground|GCBackground, &values );

  values.foreground = baseBg;
  values.background = fg;

  values.function = GXxor;
  invert = XCreateGC( display, RootWindow(display,screen),
   GCForeground|GCBackground|GCFunction, &values );

  normStackPtr = 0;
  eraseStackPtr = 0;
  xorStackPtr = 0;

  return GC_SUCCESS;

}

void gcClass::clipMinimumArea (
  GC gc,
  XRectangle *xRectArray,
  int num )
{

int i, x1, y1, x2, y2, w, h;
XRectangle xR;

// find minimum area for all x clip rectangles

  if ( num <= 0 ) {
    return;
  }

  xR.x = xRectArray[0].x;
  xR.y = xRectArray[0].y;
  xR.width = xRectArray[0].width;
  x1 = xR.x + xR.width;
  xR.height = xRectArray[0].height;
  y1 = xR.y + xR.height;

  for ( i=1; i<num; i++ ) {

    if ( xRectArray[i].x > xR.x ) xR.x = xRectArray[i].x;
    if ( xRectArray[i].y > xR.y ) xR.y = xRectArray[i].y;
    x2 = xRectArray[i].x + xRectArray[i].width;
    if ( x2 < x1 ) {
      x1 = x2;
    }
    y2 = xRectArray[i].y + xRectArray[i].height;
    if ( y2 < y1 ) {
      y1 = y2;
    }

  }

  w = x1 - xR.x;
  if ( w < 0 )
    xR.width = 0;
  else
    xR.width = w;

  h = y1 - xR.y;
  if ( h < 0 )
    xR.height = 0;
  else
    xR.height = h;

  XSetClipRectangles( display, gc, 0, 0, &xR, 1, Unsorted );

}

int gcClass::addNormXClipRectangle (
  XRectangle xR )
{

  if ( normStackPtr >= GC_STACK_MAX ) {
    fprintf( stderr, gcClass_str1 );
    return GC_STACK_OVFLO;
  }

  normXRecStack[normStackPtr] = xR;
  normStackPtr++;

  clipMinimumArea( norm, normXRecStack, normStackPtr );

  return 1;

}

int gcClass::removeNormXClipRectangle ( void )
{

  if ( normStackPtr <= 0 ) return GC_STACK_UNFLO;

  normStackPtr--;

  if ( normStackPtr ) {
    clipMinimumArea( norm, normXRecStack, normStackPtr );
  }
  else {
    XSetClipMask( display, norm, None );
  }

  return 1;

}

int gcClass::addEraseXClipRectangle (
  XRectangle xR )
{

  if ( eraseStackPtr >= GC_STACK_MAX ) {
    fprintf( stderr, gcClass_str2 );
    return GC_STACK_OVFLO;
  }

  eraseXRecStack[eraseStackPtr] = xR;
  eraseStackPtr++;

  clipMinimumArea( erase, eraseXRecStack, eraseStackPtr );

  return 1;

}

int gcClass::removeEraseXClipRectangle ( void )
{

  if ( eraseStackPtr <= 0 ) return GC_STACK_UNFLO;

  eraseStackPtr--;

  if ( eraseStackPtr )
    clipMinimumArea( erase, eraseXRecStack, eraseStackPtr );
  else
    XSetClipMask( display, erase, None );

  return 1;

}

int gcClass::addXorXClipRectangle (
  XRectangle xR )
{

  if ( xorStackPtr >= GC_STACK_MAX ) {
    fprintf( stderr, gcClass_str3 );
    return GC_STACK_OVFLO;
  }

  xorXRecStack[xorStackPtr] = xR;
  xorStackPtr++;

  clipMinimumArea( x_or, xorXRecStack, xorStackPtr );

  return 1;

}

int gcClass::removeXorXClipRectangle ( void )
{

  if ( xorStackPtr <= 0 ) return GC_STACK_UNFLO;

  xorStackPtr--;

  if ( xorStackPtr )
    clipMinimumArea( x_or, xorXRecStack, xorStackPtr );
  else
    XSetClipMask( display, x_or, None );

  return 1;

}

GC gcClass::normGC ( void ) {

  return norm;

}


GC gcClass::xorGC ( void ) {

  return x_or;

}

GC gcClass::eraseGC ( void ) {

  return erase;

}

GC gcClass::invertGC ( void ) {

  return invert;

}

void gcClass::saveFg ( void ) {

  savFg = fg;

}

void gcClass::saveBg ( void ) {

  savBg = bg;

}

void gcClass::saveBaseBg ( void ) {

  savBaseBg = baseBg;

}

void gcClass::restoreFg ( void ) {

  setFG( savFg );

}

void gcClass::restoreBg ( void ) {

  setBG( savBg );

}

void gcClass::restoreBaseBg ( void ) {

  setBaseBG( savBaseBg ); 

}

int gcClass::setFG (
  unsigned int color )
{

  XtVaGetValues( baseWidget,
   XtNbackground, &baseBg,
   NULL );

unsigned int newColor;

  if ( fg == color ) return GC_SUCCESS;

  fg = color;

  XSetForeground( display, norm, color );

  newColor = color ^ baseBg;
  XSetForeground( display, x_or, newColor );

  newColor = color;
  XSetBackground( display, invert, newColor );

  return GC_SUCCESS;  

}

int gcClass::setFG (
  int fgIndex,
  int *blink )
{

  *blink = *blink || ci->blinking( fgIndex );

  setFG( ci->pixWblink(fgIndex) );

  return 1;

}

void gcClass::updateBlink (
  activeGraphicClass *ago,
  int blink )
{

int stat;

  if ( blink ) {

    if ( !ago->blink() ) {
      stat = ci->addToBlinkList( (void *) ago, ago->blinkFunction() );
      ago->setBlink();
    }

  }
  else {

    if ( ago->blink() ) {
      stat = ci->removeFromBlinkList( (void *) ago, ago->blinkFunction() );
      ago->setNotBlink();
    }

  }

}

int gcClass::setBG (
  unsigned int color )
{

unsigned int newColor;

  if ( bg == color ) return GC_SUCCESS;

  XtVaGetValues( baseWidget,
   XtNbackground, &baseBg,
   NULL );

  bg = color;

  XSetBackground( display, norm, color );

  newColor = color ^ baseBg;
  XSetBackground( display, x_or, newColor );

  newColor = baseBg;
  XSetForeground( display, invert, newColor );

  return GC_SUCCESS;  

}

int gcClass::setBG (
  int bgIndex,
  int *blink )
{

  *blink = *blink || ci->blinking( bgIndex );

  setBG( ci->pixWblink(bgIndex) );

  return 1;

}

unsigned int gcClass::getBaseBG ( void )
{

  return baseBg;

}

int gcClass::setBaseBG (
  unsigned int color )
{

unsigned int i, fgColor, bgColor;
Widget p1, p;

//   if ( baseBg == color ) return GC_SUCCESS;

  baseBg = color;

  XtVaSetValues( baseWidget,
   XtNbackground, baseBg,
   NULL );

  i = 100;
  p1 = NULL;
  p = XtParent( baseWidget );
  while ( ( p != p1 ) && i ) {
    p1 = p;
    XtVaSetValues( p,
     XtNbackground, baseBg,
     NULL );
    p = XtParent( baseWidget );
    i--;
  }

  XSetBackground( display, erase, baseBg );
  XSetForeground( display, erase, baseBg );

  bgColor = bg ^ color;
  XSetBackground( display, x_or, bgColor );
  fgColor = fg ^ color;
  XSetForeground( display, x_or,  fgColor );

  bgColor = fg;
  XSetBackground( display, invert, bgColor );
  fgColor = baseBg;
  XSetForeground( display, invert,  fgColor );

  return GC_SUCCESS;  

}

int gcClass::setFontTag (
  char *fontTag,
  fontInfoClass *fi )
{

XFontStruct *fs;

  if ( !fontTag ) return 0;

  if ( fontTag[0] != 0 ) {
    if ( strcmp( fontTag, curFontTag ) == 0 ) {
       return GC_SUCCESS;
    }
  }

  fs = fi->getXFontStruct( fontTag );
  if ( !fs ) return GC_NO_FONT;

  XSetFont( display, norm, fs->fid );
  XSetFont( display, x_or, fs->fid );
  XSetFont( display, erase, fs->fid );
  XSetFont( display, invert, fs->fid );

  strncpy( curFontTag, fontTag, 127 );

  return GC_SUCCESS;

}

int gcClass::setNativeFont (
  char *fontName,
  fontInfoClass *fi )
{

XFontStruct *fs;

  if ( !fontName ) return 0;

  if ( fontName[0] != 0 ) {
    if ( strcmp( fontName, curFontTag ) == 0 ) {
       return GC_SUCCESS;
    }
  }

  fs = fi->getXNativeFontStruct( fontName );
  if ( !fs ) return GC_NO_FONT;

  XSetFont( display, norm, fs->fid );
  XSetFont( display, x_or, fs->fid );
  XSetFont( display, erase, fs->fid );
  XSetFont( display, invert, fs->fid );

  strncpy( curFontTag, fontName, 127 );

  return GC_SUCCESS;

}

int gcClass::setLineWidth (
  int width )
{

XGCValues values;

  if ( curLineWidth == width ) return GC_SUCCESS;

  curLineWidth = width;

  values.line_width = width;

  XChangeGC( display, norm, GCLineWidth, &values );
  XChangeGC( display, x_or, GCLineWidth, &values );
  XChangeGC( display, erase, GCLineWidth, &values );
  XChangeGC( display, invert, GCLineWidth, &values );

  return GC_SUCCESS;

}

int gcClass::setLineStyle (
  int style )
{

XGCValues values;

  if ( curLineStyle == style ) return GC_SUCCESS;

  curLineStyle = style;

  values.line_style = style;

  XChangeGC( display, norm, GCLineStyle, &values );
  XChangeGC( display, x_or, GCLineStyle, &values );
  XChangeGC( display, erase, GCLineStyle, &values );
  XChangeGC( display, invert, GCLineStyle, &values );

  return GC_SUCCESS;

}

int gcClass::setLineCapStyle (
  int style )
{

XGCValues values;

  if ( curLineCapStyle == style ) return GC_SUCCESS;

  curLineCapStyle = style;

  values.cap_style = style;

  XChangeGC( display, norm, GCCapStyle, &values );
  XChangeGC( display, x_or, GCCapStyle, &values );
  XChangeGC( display, erase, GCCapStyle, &values );
  XChangeGC( display, invert, GCCapStyle, &values );

  return GC_SUCCESS;

}

int gcClass::setLineJoinStyle (
  int style )
{

XGCValues values;

  if ( curLineJoinStyle == style ) return GC_SUCCESS;

  curLineJoinStyle = style;

  values.join_style = style;

  XChangeGC( display, norm, GCJoinStyle, &values );
  XChangeGC( display, x_or, GCJoinStyle, &values );
  XChangeGC( display, erase, GCJoinStyle, &values );
  XChangeGC( display, invert, GCJoinStyle, &values );

  return GC_SUCCESS;

}

int gcClass::setArcModeChord ( void )
{

XGCValues values;

  if ( curArcMode == ArcChord ) return GC_SUCCESS;

  curArcMode = ArcChord;

  values.arc_mode = ArcChord;

  XChangeGC( display, norm, GCArcMode, &values );
  XChangeGC( display, x_or, GCArcMode, &values );
  XChangeGC( display, erase, GCArcMode, &values );
  XChangeGC( display, invert, GCArcMode, &values );

  return GC_SUCCESS;

}

int gcClass::setArcModePieSlice ( void )
{

XGCValues values;

  if ( curArcMode == ArcPieSlice ) return GC_SUCCESS;

  curArcMode = ArcPieSlice;

  values.arc_mode = ArcPieSlice;

  XChangeGC( display, norm, GCArcMode, &values );
  XChangeGC( display, x_or, GCArcMode, &values );
  XChangeGC( display, erase, GCArcMode, &values );
  XChangeGC( display, invert, GCArcMode, &values );

  return GC_SUCCESS;

}

int gcClass::setGraphicsExposures (
  int state )
{

XGCValues values;

  values.graphics_exposures = state;

  XChangeGC( display, norm, GCGraphicsExposures, &values );
  XChangeGC( display, x_or, GCGraphicsExposures, &values );
  XChangeGC( display, erase, GCGraphicsExposures, &values );
  XChangeGC( display, invert, GCGraphicsExposures, &values );

  return GC_SUCCESS;

}

int gcClass::setFGforGivenBG (
  unsigned int _fgColor,
  unsigned int _bgColor )
{

unsigned int newColor;

  fg = _fgColor;

  XSetForeground( display, norm, _fgColor );

  newColor = _fgColor ^ _bgColor;
  XSetForeground( display, x_or, newColor );

  newColor = _fgColor;
  XSetBackground( display, invert, newColor );

  return GC_SUCCESS;  

}
