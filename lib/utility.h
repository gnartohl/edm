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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <sys/time.h>

extern "C"
{
#include <regex.h>
}

#if defined(darwin) || defined(HP_UX)
	#include <sys/wait.h>
#else
	#include <wait.h>
#endif

#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>

#include "font_pkg.h"
#include "gc_pkg.h"
#include "expString.h"
#include "remFileOpen.h"

int doReSearchReplace (
  int caseInsensivite,
  char *expression,
  char *newText,
  int max,
  char *oldString,
  char *newString
);

int doSearchReplace (
  int caseInsensivite,
  int useRegExpr,
  char *expression,
  char *newText,
  int max,
  char *oldString,
  char *newString
);

void enableAccumulator ( void );
void disableAccumulator ( void );
int useAccumulator ( void );
void setAccumulator ( int );
int getAccumulator ( void );
void incAccumulator ( void );
void doAccSubs( expStringClass & );
void doAccSubs( char *, int );

int useAppTopParent ( void );

int debugMode ( void );

int diagnosticMode ( void );

int logDiagnostic (
  char *text
);

void disableBadWindowErrors ( int arg );

int badWindowErrorsDisabled ( void );

char *getEnvironmentVar (
  char *name
);

void setServerSocketFd (
  int fd
);

int executeCmd (
  const char *cmdString
);

void executeCommandInThread (
  char *_cmd
);

char *expandEnvVars (
  char *inStr,
  int maxOut,
  char *outStr
);

int blank (
  char *string );

int blankOrComment (
  char *string );

XtIntervalId appAddTimeOut (
  XtAppContext app,
  unsigned long interval,
  XtTimerCallbackProc proc,
  XtPointer client_data );

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

void trimWhiteSpace (
  char *str );

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

int xDrawText (
  Display *d,
  Window win,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value );

int xEraseText (
  Display *d,
  Window win,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value );

int xDrawImageText (
  Display *d,
  Window win,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value );

int xEraseImageText (
  Display *d,
  Window win,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value );

int drawText (
  Widget widget,
  Drawable dr,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value );

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
  Drawable dr,
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
  Drawable dr,
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
  Drawable dr,
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

void getStringBoxSize (
  char *str,
  int len,
  XFontStruct **fs,
  int alignment,
  int *width,
  int *height );

void XDrawStringsAligned (
  Display *d,
  Window w,
  GC gc,
  int x,
  int y,
  int fieldWidth,
  char *str,
  int len,
  XFontStruct **fs,
  int alignment );

void XDrawStrings (
  Display *d,
  Window w,
  GC gc,
  int x,
  int y,
  int h,
  char *str,
  int len );

void XDrawImageStringsAligned (
  Display *d,
  Window w,
  GC gc,
  int x,
  int y,
  int fieldWidth,
  char *str,
  int len,
  XFontStruct **fs,
  int alignment );

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

int get_scale_params1 (
  double min,
  double max,
  double *adj_min,
  double *adj_max,
  int *num_label_ticks,
  int *majors_per_label,
  int *minors_per_major,
  char *format
);

int get_log10_scale_params1 (
  double min,
  double max,
  double *adj_min,
  double *adj_max,
  int *num_label_ticks,
  int *majors_per_label,
  int *minors_per_major,
  char *format
);

int get_scale_params (
  double min,
  double max,
  double *adj_min,
  double *adj_max,
  double *label_tick,
  int *majors_per_label,
  int *minor_per_major,
  char *format
);

int formatString (
  double value,
  char *string,
  int len
);

int formatString (
  double value,
  char *string,
  int len,
  char *fmt
);

void drawXLinearTimeScale (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleLen,
  time_t absolute_min,
  double adj_min,
  double adj_max,
  int time_format,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridHeight,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int xminConstrained,
  int xmaxConstrained,
  int erase
);

void drawXLinearScale (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleLen,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridHeight,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int xminConstrained,
  int xmaxConstrained,
  int erase
);

void drawXLinearScale (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleLen,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridHeight,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int xminConstrained,
  int xmaxConstrained,
  int erase,
  char* fmt
);

void drawXLinearScale2 (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleLen,
  double cur_min,
  double cur_max,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridHeight,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int xminConstrained,
  int xmaxConstrained,
  int erase
);

void drawXLog10Scale (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleLen,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridHeight,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int xminConstrained,
  int xmaxConstrained,
  int erase
);

int xScaleHeight (
  char *fontTag,
  XFontStruct *fs
);

int xScaleMargin (
  char *fontTag,
  XFontStruct *fs,
  double adj_min,
  double adj_max
);

int xTimeScaleHeight (
  char *fontTag,
  XFontStruct *fs
);

int xTimeScaleMargin (
  char *fontTag,
  XFontStruct *fs,
  double adj_min,
  double adj_max
);

void getXLimitCoords (
  int x,
  int y,
  int scaleLen,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  char *fontTag,
  XFontStruct *fs,
  int *xMinX0,
  int *xMinX1,
  int *xMinY0,
  int *xMinY1,
  int *xMaxX0,
  int *xMaxX1,
  int *xMaxY0,
  int *xMaxY1
);

void getXLog10LimitCoords (
  int x,
  int y,
  int scaleLen,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  char *fontTag,
  XFontStruct *fs,
  int *xMinX0,
  int *xMinX1,
  int *xMinY0,
  int *xMinY1,
  int *xMaxX0,
  int *xMaxX1,
  int *xMaxY0,
  int *xMaxY1
);

void drawYLinearScale (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleHeight,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridLen,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int xminConstrained,
  int xmaxConstrained,
  int erase
);

void drawYLinearScale (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleHeight,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridLen,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int xminConstrained,
  int xmaxConstrained,
  int erase,
  char *fmt
);

void drawYLinearScale2 (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleHeight,
  double cur_min,
  double cur_max,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridLen,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int xminConstrained,
  int xmaxConstrained,
  int erase
);

void drawY2LinearScale (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleHeight,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridLen,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int xminConstrained,
  int xmaxConstrained,
  int erase
);

void drawY2LinearScale2 (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleHeight,
  double cur_min,
  double cur_max,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridLen,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int xminConstrained,
  int xmaxConstrained,
  int erase
);

void drawYLog10Scale (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleHeight,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridLen,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int xminConstrained,
  int xmaxConstrained,
  int erase
);

void drawY2Log10Scale (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleHeight,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridLen,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int xminConstrained,
  int xmaxConstrained,
  int erase
);

int yScaleWidth (
  char *fontTag,
  XFontStruct *fs,
  double adj_min,
  double adj_max,
  int num_label_ticks
);

int yLog10ScaleWidth (
  char *fontTag,
  XFontStruct *fs,
  double adj_min,
  double adj_max,
  int num_label_ticks
);

int yScaleMargin (
  char *fontTag,
  XFontStruct *fs
);

void getYLimitCoords (
  int x,
  int y,
  int scaleLen,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  char *fontTag,
  XFontStruct *fs,
  int *yMinX0,
  int *yMinX1,
  int *yMinY0,
  int *yMinY1,
  int *yMaxX0,
  int *yMaxX1,
  int *yMaxY0,
  int *yMaxY1
);

void getY2LimitCoords (
  int x,
  int y,
  int scaleLen,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  char *fontTag,
  XFontStruct *fs,
  int *yMinX0,
  int *yMinX1,
  int *yMinY0,
  int *yMinY1,
  int *yMaxX0,
  int *yMaxX1,
  int *yMaxY0,
  int *yMaxY1
);

void getYLog10LimitCoords (
  int x,
  int y,
  int scaleLen,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  char *fontTag,
  XFontStruct *fs,
  int *yMinX0,
  int *yMinX1,
  int *yMinY0,
  int *yMinY1,
  int *yMaxX0,
  int *yMaxX1,
  int *yMaxY0,
  int *yMaxY1
);

void getY2Log10LimitCoords (
  int x,
  int y,
  int scaleLen,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  char *fontTag,
  XFontStruct *fs,
  int *yMinX0,
  int *yMinX1,
  int *yMinY0,
  int *yMinY1,
  int *yMaxX0,
  int *yMaxX1,
  int *yMaxY0,
  int *yMaxY1
);

int intersects (
  int x0,
  int y0,
  int x1,
  int y1,
  int xx0,
  int yy0,
  int xx1,
  int yy1
);

#endif
