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

#ifndef __utility_h
#define __utility_h 1

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>

#include "font_pkg.h"
#include "gc_pkg.h"

int blank (
  char *string );

void genericProcessAllEvents (
  int sync,
  XtAppContext app,
  Display *d );

void processAllEventsWithSync (
  XtAppContext app,
  Display *d );

void processAllEvents (
  XtAppContext app,
  Display *d );

int isLegalInteger (
  char *str );

int isLegalFloat (
  char *str );

int writeStringToFile (
  FILE *f,
  char *str );

void readStringFromFile (
  char *str,
  int maxChars,
  FILE *f );

int drawText (
  Widget widget,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value );

int eraseText (
  Widget widget,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value );

int drawImageText (
  Widget widget,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value );

int eraseImageText (
  Widget widget,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value );

int textBoundaries (
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value,
  int *x0,
  int *y0,
  int *x1,
  int *y1 );

int lockFile (
  FILE *f );

int unlockFile (
  FILE *f );

int fileIsLocked (
  FILE *f );

/* new new new */

void buildFileName (
  char *expandedName,
  char *inName,
  char *prefix,
  int maxSize );

int getFileName (
  char *name,
  char *fullName,
  int maxSize );

int getFilePrefix (
  char *prefix,
  char *fullName,
  int maxSize );

int getFilePostfix (
  char *postfix,
  char *fullName,
  int maxSize );

/* new new new */

char *getNextDataString (
  char *str,
  int max,
  FILE *f );

void XDrawStrings (
  Display *d,
  Window w,
  GC gc,
  int x,
  int y,
  int h,
  char *str,
  int len );

void XDrawImageStrings (
  Display *d,
  Window w,
  GC gc,
  int x,
  int y,
  int h,
  char *str,
  int len );

int countSymbolsAndValues (
  char *string,
  int *total,
  int *maxLen
);

int parseSymbolsAndValues (
  char *string,
  int max,
  char *symbols[],
  char *values[],
  int *numFound
);

int parseLocalSymbolsAndValues (
  char *string,
  int max,
  int maxLen,
  char *symbols[],
  char *values[],
  int *numFound
);

int get_scale_params (
  double min,
  double max,
  double *adj_min,
  double *adj_max,
  double *label_tick,
  double *major_tick,
  double *minor_tick,
  char *format
);

#endif
