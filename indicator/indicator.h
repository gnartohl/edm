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

#ifndef __indicator_h
#define __indicator_h 1

#include "act_grf.h"
#include "entry_form.h"

#include "pv_factory.h"

#define INDICATORC_K_COLORMODE_STATIC 0
#define INDICATORC_K_COLORMODE_ALARM 1

#define INDICATORC_MAJOR_VERSION 4
#define INDICATORC_MINOR_VERSION 2
#define INDICATORC_RELEASE 0

#define INDICATORC_K_PV_NAME 0
#define INDICATORC_K_LITERAL 1

#define INDICATORC_K_MAX_GE_MIN 1
#define INDICATORC_K_MAX_LT_MIN 2

#define INDICATORC_K_LIMITS_FROM_DB 1
#define INDICATORC_K_LIMITS_FROM_FORM 0

#define INDICATORC_K_SHAPE_UNKNOWN 0
#define INDICATORC_K_SHAPE_PTR 1
#define INDICATORC_K_SHAPE_GT 2
#define INDICATORC_K_SHAPE_LT 3

#ifdef __indicator_cc

#include "indicator.str"

static char *dragName[] = {
  activeIndicatorClass_str1,
  activeIndicatorClass_str2,
};

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void indicatorc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void indicatorc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void indicatorc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void indicatorc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void indicatorc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void indicator_readUpdate (
  ProcessVariable *pv,
  void *userarg );

static void indicator_nullUpdate (
  ProcessVariable *pv,
  void *userarg );

static void indicator_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void indicator_monitor_null_connect_state (
  ProcessVariable *pv,
  void *userarg );

#endif

class activeIndicatorClass : public activeGraphicClass {

private:

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void indicatorc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void indicatorc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void indicatorc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void indicatorc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void indicatorc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void indicator_readUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void indicator_nullUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void indicator_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void indicator_monitor_null_connect_state (
  ProcessVariable *pv,
  void *userarg );

int horizontal, bufHorizontal;

int halfW, bufHalfW;

int pointerOpposite, bufPointerOpposite;

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
 *minorTicksEntry, *scaleFormatEntry;

entryListBase *limitsFromDbEntry, *scalePrecEntry, *scaleMinEntry,
 *scaleMaxEntry;

int bufX, bufY, bufW, bufH;

int opComplete;

int minW, minVertW;
int minH, minVertH;

double controlV, curControlV, readV, curReadV, curNullV;
int indicatorY, oldIndicatorY, indicatorH, oldIndicatorH, indicatorW, oldIndicatorW,
 bufInvalid, indicatorX, oldIndicatorX, indX, oldIndX, indY, oldIndY, mode,
 indicatorAreaX, indicatorAreaW, indicatorAreaY, indicatorAreaH,
 indicatorStrLen, indicatorMaxW, indicatorMaxH, shape, oldShape;
double range, factor, offset;

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

int indicatorColorMode, fgColorMode;
pvColorClass indicatorColor, fgColor, bgColor;
colorButtonClass indicatorCb, fgCb, bgCb;
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

int bufIndicatorColorMode, bufFgColorMode;
int bufIndicatorColor, bufFgColor, bufBgColor;
int bufLabelType;
int bufBorder;
int bufShowScale;
char bufLabelTicks[15+1], bufMajorTicks[15+1], bufMinorTicks[15+1];
char bufFontTag[63+1];
char bufScaleFormat[15+1];
int bufLimitsFromDb;
char bufReadMin[15+1], bufReadMax[15+1];
char bufPrecision[15+1];

int needErase, needDraw, needFullDraw, needDrawCheck, needConnectInit,
 needRefresh, needInfoInit;
int needToDrawUnconnected, needToEraseUnconnected;
XtIntervalId unconnectedTimer;

public:

activeIndicatorClass ( void );

activeIndicatorClass
 ( const activeIndicatorClass *source );

~activeIndicatorClass ( void );

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
  int indicatorState,
  int indicatorNumber );

void btnDown (
  int x,
  int y,
  int indicatorState,
  int indicatorNumber );

void btnDrag (
  int x,
  int y,
  int indicatorState,
  int indicatorNumber );

int getIndicatorActionRequest (
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

void updateIndicator ( void );

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

void *create_activeIndicatorClassPtr ( void );
void *clone_activeIndicatorClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
