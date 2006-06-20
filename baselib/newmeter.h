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

#ifndef __meter_h
#define __meter_h 1

#include "act_grf.h"
#include "entry_form.h"

#include "pv_factory.h"
#include "cvtFast.h"

#define METERC_K_COLORMODE_STATIC 0
#define METERC_K_COLORMODE_ALARM 1

#define METERC_MAJOR_VERSION 4
#define METERC_MINOR_VERSION 1
#define METERC_RELEASE 0

#define METERC_K_LITERAL 1
#define METERC_K_PV_LABEL 2
#define METERC_K_PV_NAME 3

#define METERC_K_MAX_GE_MIN 1
#define METERC_K_MAX_LT_MIN 2

#ifdef __meter_cc

#include "newmeter.str"

static char *dragName[] = {
  activeMeterClass_str1
};

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void meterc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void meterc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void meterc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void meterc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void meterc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void meter_readUpdate (
  ProcessVariable *pv,
  void *userarg );

static void meter_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg );

#endif

class activeMeterClass : public activeGraphicClass {

private:

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void meterc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void meterc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void meterc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void meterc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void meterc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void meter_readUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void meter_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg );

typedef struct editBufTag {
// edit buffer
  int bufX;
  int bufY;
  int bufW;
  int bufH;
  int bufScaleLimitsFromDb;
  int bufMeterColorMode;
  int bufFgColorMode;
  int bufScaleColorMode;
  char bufLabelIntervals[15+1];
  int bufMajorIntervals;
  int bufMinorIntervals;
  int bufMeterColor;
  int bufFgColor;
  int bufBgColor;
  int bufLabelColor;
  int bufScaleColor;
  int bufTsColor;
  int bufBsColor;
  int bufLabelType;
  double bufMeterAngle;
  char bufScalePrecision[15+1];
  char bufScaleMin[15+1];
  char bufScaleMax[15+1];
  int bufNeedleType;
  int bufShadowMode;
  int bufShowScale;
  int bufUseDisplayBg;
  colorButtonClass meterCb;
  colorButtonClass fgCb;
  colorButtonClass bgCb;
  colorButtonClass tsCb;
  colorButtonClass bsCb;
  colorButtonClass labelCb;
  colorButtonClass scaleCb;
  char bufScaleFormat[15+1];
  char bufLabel[39+1];
  char bufLiteralLabel[39+1];
  char bufControlPvName[PV_Factory::MAX_PV_NAME+1];
  char bufReadPvName[PV_Factory::MAX_PV_NAME+1];
  int bufTrackDelta;
} editBufType, *editBufPtr;

editBufPtr eBuf;

int scaleLimitsFromDb;

int opComplete;

int minW;
int minH;

double controlV, curControlV, readV, curReadV, readMin, readMax, baseV;
int meterW, oldMeterW, bufInvalid, meterX, oldMeterX, originW, mode;
double meterOriginX;

int meterNeedleXorigin, meterNeedleYorigin;
int meterNeedleXend, meterNeedleYend;
int oldMeterNeedleXOrigin, oldMeterNeedleYOrigin;
int oldMeterNeedleXEnd, oldMeterNeedleYEnd;

fontMenuClass scaleFm, labelFm;
char scaleFontTag[63+1], labelFontTag[63+1];
XFontStruct *scaleFs, *labelFs;
int scaleFontAscent, scaleFontDescent, scaleFontHeight;
int labelFontAscent, labelFontDescent, labelFontHeight;

ProcessVariable *readPvId;
int initialReadConnection;
int oldStat, oldSev;

expStringClass controlPvExpStr, readPvExpStr, scaleMinExpStr,
 scaleMaxExpStr, scalePrecExpStr, labIntExpStr;

int controlExists, readExists;

int controlPvConnected, readPvConnected, active, activeMode, activeInitFlag;

int meterColorMode, fgColorMode, scaleColorMode;
pvColorClass meterColor, fgColor, bgColor;
pvColorClass tsColor, bsColor, labelColor, scaleColor;
char label[39+1];
int labelType;
int drawStaticFlag;
int showScale;
int useDisplayBg;
int trackDelta;

int labelIntervals, majorIntervals, minorIntervals;
char literalLabel[39+1];
double meterAngle;
char scaleFormat[15+1];
int scalePrecision;
double scaleMin;
double scaleMax;
int needleType;
int shadowMode;

int needErase, needDraw, needConnectInit, needRefresh, needInfoInit;
int needToDrawUnconnected, needToEraseUnconnected;
int unconnectedTimer;

public:

activeMeterClass ( void );

activeMeterClass
 ( const activeMeterClass *source );

~activeMeterClass ( void );

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

int old_save (
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
  int meterState,
  int meterNumber );

void btnDown (
  int x,
  int y,
  int meterState,
  int meterNumber );

void btnDrag (
  int x,
  int y,
  int meterState,
  int meterNumber );

int getMeterActionRequest (
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

char *crawlerGetFirstPv ( void );

char *crawlerGetNextPv ( void );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeMeterClassPtr ( void );
void *clone_activeMeterClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
