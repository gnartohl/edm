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

#ifndef __myPng_h
#define __myPng_h 1

#include "act_grf.h"
#include "entry_form.h"
#include "avl.h"
#include "png.h"
#include <sys/stat.h>
#include <unistd.h>

// the following defines btnActionListType & btnActionListPtr
#include "btnActionListType.h"

#define APNGC_MAJOR_VERSION 1
#define APNGC_MINOR_VERSION 0
#define APNGC_RELEASE 0

#ifdef __png_cc

#include "png.str"

// from readpng.h

/* future versions of libpng will provide this macro: */
#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr)   ((png_ptr)->jmpbuf)
#endif

#ifndef TRUE
#  define TRUE 1
#  define FALSE 0
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) > (b)? (a) : (b))
#  define MIN(a,b)  ((a) < (b)? (a) : (b))
#endif

typedef unsigned char   uch;
typedef unsigned short  ush;
typedef unsigned long   ulg;

// end of from readpng.h

static void apngc_update (
  XtPointer client,
  XtIntervalId *id );

static void apngc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void apngc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void apngc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void apngc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void apngc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class activePngClass : public activeGraphicClass {

private:

typedef struct colorCacheTag {
  AVL_FIELDS(colorCacheTag)
  unsigned int rgb[3]; // [0]=r, [1]=g, [2]=b
  unsigned long pixel;
} colorCacheType, *colorCachePtr;

AVL_HANDLE allColorsH;
AVL_HANDLE colorCacheByColorH;
AVL_HANDLE colorCacheByPixelH;
unsigned char colorStrippingMask;

FILE *fp;

png_structp png_ptr;
png_infop info_ptr, end_info_ptr;
png_uint_32 width, height;
int depth, bit_depth, color_type, interlace_type, usePixelArray;
uch bg_red, bg_green, bg_blue;
unsigned int *pixels;

ulg image_width, image_height, image_rowbytes;
int image_channels;
jmp_buf jmpbuf;

int init, active, activeMode, opComplete;
char pngFileName[127+1];
XImage *image;
uch *xData, *image_data;
int noFile;

int bufX;
int bufY;
int bufW;
int bufH;

char bufPngFileName[127+1];
time_t fileModTime, prevFileModTime;

XtIntervalId timer;
int timerValue;
int timerActive;

int uniformSize, bufUniformSize;
int refreshRate, bufRefreshRate;
int fastErase, bufFastErase;

int needErase;

public:

friend void apngc_update (
  XtPointer client,
  XtIntervalId *id );

friend void apngc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void apngc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void apngc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void apngc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void apngc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

activePngClass::activePngClass ( void );

activePngClass::activePngClass
 ( const activePngClass *source );

activePngClass::~activePngClass ( void );

int activePngClass::addColorToList (
  int red,
  int green,
  int blue
);

int activePngClass::discardColorList ( void );

int activePngClass::allocColors ( void );

int activePngClass::discardPixels ( void );

int activePngClass::allocOneColor (
  unsigned short r,
  unsigned short g,
  unsigned short b
);

void activePngClass::fillPixelArray ( void );

unsigned int activePngClass::getPixel (
  unsigned char r,
  unsigned char g,
  unsigned char b
);

int activePngClass::genericEdit ( void );

int activePngClass::edit ( void );

int activePngClass::editCreate ( void );

void activePngClass::checkPngFileTime ( void );

int activePngClass::readPngFile ( void );

int activePngClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activePngClass::save (
  FILE *f );

int activePngClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activePngClass::draw ( void );

int activePngClass::draw (
  int _x,
  int _y,
  int _w,
  int _h );

int activePngClass::erase ( void );

int activePngClass::drawActive ( void );

int activePngClass::drawActive (
  int _x,
  int _y,
  int _w,
  int _h );

int activePngClass::eraseActive ( void );

int activePngClass::activate (
  int pass,
  void *ptr );

int activePngClass::deactivate (
  int pass );

int activePngClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int activePngClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

void activePngClass::executeDeferred ( void );

void activePngClass::readpng_version_info ( void );

int activePngClass::readpng_init (
  FILE *infile,
  ulg *pWidth,
  ulg *pHeight
);

int activePngClass::readpng_get_bgcolor (
  uch *bg_red,
  uch *bg_green,
  uch *bg_blue
);

uch *activePngClass::readpng_get_image (
  double display_exponent,
  int *pChannels,
  ulg *pRowbytes
);

void activePngClass::readpng_cleanup (
  int free_image_data
);

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activePngClassPtr ( void );
void *clone_activePngClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
