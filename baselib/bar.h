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

#ifndef __bar_h
#define __bar_h 1

#include "act_grf.h"
#include "entry_form.h"

#include "pv_factory.h"
#include "cvtFast.h"

#define BARC_K_COLORMODE_STATIC 0
#define BARC_K_COLORMODE_ALARM 1

#define BARC_MAJOR_VERSION 4
#define BARC_MINOR_VERSION 1
#define BARC_RELEASE 2

#define BARC_K_PV_NAME 0
#define BARC_K_LITERAL 1

#define BARC_K_MAX_GE_MIN 1
#define BARC_K_MAX_LT_MIN 2

#define BARC_K_LIMITS_FROM_DB 1
#define BARC_K_LIMITS_FROM_FORM 0

#ifdef __bar_cc

#include "bar.str"

static char *dragName[] = {
  activeBarClass_str1,
  activeBarClass_str2,
};

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void barc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void barc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void barc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void barc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void barc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void bar_readUpdate (
  ProcessVariable *pv,
  void *userarg );

static void bar_nullUpdate (
  ProcessVariable *pv,
  void *userarg );

static void bar_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void bar_monitor_null_connect_state (
  ProcessVariable *pv,
  void *userarg );

#endif

class activeBarClass : public activeGraphicClass {

private:

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void barc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void barc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void barc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void barc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void barc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void bar_readUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void bar_nullUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void bar_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void bar_monitor_null_connect_state (
  ProcessVariable *pv,
  void *userarg );

int horizontal, bufHorizontal;

typedef struct editBufTag {
// edit buffer
  char bufControlPvName[PV_Factory::MAX_PV_NAME+1];
  char bufReadPvName[PV_Factory::MAX_PV_NAME+1];
  char bufNullPvName[PV_Factory::MAX_PV_NAME+1];
  char bufLabel[PV_Factory::MAX_PV_NAME+1];
} editBufType, *editBufPtr;

editBufPtr eBuf;

entryListBase *labelTypeEntry, *labelEntry;

entryListBase *showScaleEntry, *labelTicksEntry, *majorTicksEntry,
 *minorTicksEntry, *scaleFormatEntry, *scaleOriginEntry;

entryListBase *limitsFromDbEntry, *scalePrecEntry, *scaleMinEntry,
 *scaleMaxEntry;

int bufX, bufY, bufW, bufH;

int opComplete;

int minW, minVertW;
int minH, minVertH;

double controlV, curControlV, readV, curReadV, curNullV;
int originW, originH, mode, barStrLen;

double barOriginVal, factorLt, factorGe;

int barAreaX, barAreaW, barAreaY, barAreaH;
int barTop, barBot, barOriginW, barOriginH;
int barMaxW, barMaxH, aboveBarOrigin, oldAboveBarOrigin, zeroCrossover;
int barY, oldBarY, barH, oldBarH, barW, oldBarW, barX, oldBarX;

int bufInvalid;
int posScale;
double factor;
int barOriginLoc, oldBarOriginLoc, barEdgeLoc, oldBarEdgeLoc;

expStringClass barOriginValExpStr;

fontMenuClass fm;
char fontTag[63+1];
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

ProcessVariable *readPvId, *nullPvId;
int initialReadConnection, initialNullConnection;
int oldStat, oldSev;

expStringClass controlPvExpStr, readPvExpStr, nullPvExpStr, label;

unsigned char pvNotConnectedMask;

int controlExists, readExists, nullExists;

int init, active, activeMode;

int barColorMode, fgColorMode;
pvColorClass barColor, fgColor, bgColor;
colorButtonClass barCb, fgCb, bgCb;
int labelType;
int border;
int showScale;
expStringClass labelTicksExpStr, majorTicksExpStr, minorTicksExpStr;
int labelTicks, majorTicks, minorTicks;
char scaleFormat[15+1];
int limitsFromDb;
expStringClass readMinExpStr, readMaxExpStr;
double readMin, readMax;
int precision;
expStringClass precisionExpStr;

int bufBarColorMode, bufFgColorMode;
int bufBarColor, bufFgColor, bufBgColor;
int bufLabelType;
int bufBorder;
int bufShowScale;
char bufLabelTicks[15+1], bufMajorTicks[15+1], bufMinorTicks[15+1];
char bufFontTag[63+1];
char bufScaleFormat[15+1];
int bufLimitsFromDb;
char bufReadMin[15+1], bufReadMax[15+1];
char bufPrecision[15+1];
char bufBarOriginX[15+1];

int needErase, needDraw, needFullDraw, needDrawCheck, needConnectInit,
 needRefresh, needInfoInit;
int needToDrawUnconnected, needToEraseUnconnected;
XtIntervalId unconnectedTimer;

public:

activeBarClass ( void );

activeBarClass
 ( const activeBarClass *source );

~activeBarClass ( void );

char *objName ( void ) {

  return name;

}

int createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int save (
  FILE *f );

int createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int old_createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int draw ( void );

int erase ( void );

int drawActive ( void );

int eraseActive ( void );

int activate ( int pass, void *ptr );

int deactivate ( int pass );

void updateDimensions ( void );

void bufInvalidate ( void );

int expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] );

int expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int containsMacros ( void );

void btnUp (
  int x,
  int y,
  int barState,
  int barNumber );

void btnDown (
  int x,
  int y,
  int barState,
  int barNumber );

void btnDrag (
  int x,
  int y,
  int barState,
  int barNumber );

int getBarActionRequest (
  int *up,
  int *down,
  int *drag );

int checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

void drawScale (
  Widget widget,
  Drawable dr,
  gcClass *gc );

void drawHorzScale (
  Widget widget,
  Drawable dr,
  gcClass *gc );

void drawVertScale (
  Widget widget,
  Drawable dr,
  gcClass *gc );


int isAbove (
  int posScale,
  double val1,
  double val2
);

int isAboveOrEqual (
  int posScale,
  double val1,
  double val2
);

int isBelow (
  int posScale,
  double val1,
  double val2
);

int isBelowOrEqual (
  int posScale,
  double val1,
  double val2
);

void updateScaleInfo ( void );

void updateHorzScaleInfo ( void );

void updateVertScaleInfo ( void );

void updateHorzBar ( void );

void updateVertBar ( void );

void updateBar ( void );

void executeDeferred ( void );

char *firstDragName ( void );

char *nextDragName ( void );

char *dragValue (
  int i );

void changeDisplayParams (
  unsigned int flag,
  char *fontTag,
  int alignment,
  char *ctlFontTag,
  int ctlAlignment,
  char *btnFontTag,
  int btnAlignment,
  int textFgColor,
  int fg1Color,
  int fg2Color,
  int offsetColor,
  int bgColor,
  int topShadowColor,
  int botShadowColor );

void changePvNames (
  int flag,
  int numCtlPvs,
  char *ctlPvs[],
  int numReadbackPvs,
  char *readbackPvs[],
  int numNullPvs,
  char *nullPvs[],
  int numVisPvs,
  char *visPvs[],
  int numAlarmPvs,
  char *alarmPvs[] );

void getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n );

char *getSearchString (
  int i
);

void replaceString (
  int i,
  int max,
  char *string
);

char *crawlerGetFirstPv ( void );

char *crawlerGetNextPv ( void );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeBarClassPtr ( void );
void *clone_activeBarClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
