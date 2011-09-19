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

#ifndef __slider_h
#define __slider_h 1

#include "ulBindings.h"
#include "act_grf.h"
#include "entry_form.h"
#include "utility.h"
#include "keypad.h"

#include "pv_factory.h"
#include "cvtFast.h"

#define SLC_MAJOR_VERSION 4
#define SLC_MINOR_VERSION 0
#define SLC_RELEASE 0

#define SLC_STATE_IDLE 1
#define SLC_STATE_MOVING 2

#define SLC_K_LITERAL 0
#define SLC_K_LABEL 1
#define SLC_K_PV_NAME 2

#define SLC_K_FORMAT_FLOAT 0
#define SLC_K_FORMAT_EXPONENTIAL 1

#define SLC_K_COLORMODE_STATIC 0
#define SLC_K_COLORMODE_ALARM 1

#ifdef __slider_cc

#include "slider.str"

static char *dragName[] = {
  activeSliderClass_str36,
  activeSliderClass_str42,
  activeSliderClass_str48
};

static void sloSetCtlKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void sloSetIncKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void sloCancelKp (
  Widget w,
  XtPointer client,
  XtPointer call );

static void sl_monitor_saved_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void sl_savedValueUpdate (
  ProcessVariable *pv,
  void *userarg );

static void sliderEventHandler(
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch );

static void slc_updateControl (
  XtPointer client,
  XtIntervalId *id );

static void slc_decrement (
  XtPointer client,
  XtIntervalId *id );

static void slc_increment (
  XtPointer client,
  XtIntervalId *id );

static void slc_value_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void slc_value_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void slc_value_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void slc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void slc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void slc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void slc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void slc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void sl_controlUpdate (
  ProcessVariable *pv,
  void *userarg );

static void sl_monitor_control_label_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void sl_monitor_read_label_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void sl_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void sl_readUpdate (
  ProcessVariable *pv,
  void *userarg );

static void sl_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg );

#endif

class activeSliderClass : public activeGraphicClass {

private:

friend void sloSetCtlKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void sloSetIncKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void sloCancelKp (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void sl_savedValueUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void sl_monitor_saved_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void sliderEventHandler(
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch );

friend void slc_updateControl (
  XtPointer client,
  XtIntervalId *id );

friend void slc_decrement (
  XtPointer client,
  XtIntervalId *id );

friend void slc_increment (
  XtPointer client,
  XtIntervalId *id );

friend void slc_value_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void slc_value_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void slc_value_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void slc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void slc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void slc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void slc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void slc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void sl_controlUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void sl_monitor_control_label_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void sl_monitor_read_label_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void sl_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void sl_readUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void sl_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg );

XtIntervalId updateControlTimer;
int updateControlTimerValue;
int updateControlTimerActive;
int controlAdjusted;

double oneControlV;

XtIntervalId incrementTimer;
int incrementTimerActive;
int incrementTimerValue;

int opComplete;

int minW;
int minH;

typedef struct editBufTag {
// edit buffer
  int bufX;
  int bufY;
  int bufW;
  int bufH;
  double bufControlV;
  double bufIncrement;
  double bufAccelMultiplier;
  int bufBgColorMode;
  int bufControlColorMode;
  int bufReadColorMode;
  int bufBgColor;
  int bufFgColor;
  int bufShadeColor;
  int bufControlColor;
  int bufReadColor;
  colorButtonClass fgCb;
  colorButtonClass bgCb;
  colorButtonClass shadeCb;
  colorButtonClass controlCb;
  colorButtonClass readCb;
  int bufControlX;
  int bufReadX;
  char bufFontTag[63+1];
  char controlBufPvName[PV_Factory::MAX_PV_NAME+1];
  char readBufPvName[PV_Factory::MAX_PV_NAME+1];
  char savedValueBufPvName[PV_Factory::MAX_PV_NAME+1];
  char controlBufLabelName[PV_Factory::MAX_PV_NAME+1];
  char readBufLabelName[PV_Factory::MAX_PV_NAME+1];
  int bufFormatType;
  int bufChangeCallbackFlag;
  int bufActivateCallbackFlag;
  int bufDeactivateCallbackFlag;
  int bufLimitsFromDb;
  efDouble bufEfScaleMin;
  efDouble bufEfScaleMax;
  efInt bufEfPrecision;
  char bufDisplayFormat[15+1];
} editBufType, *editBufPtr;

editBufPtr eBuf;

entryListBase *ctlLabelTypeEntry, *ctlLabelEntry;

entryListBase *rdLabelTypeEntry, *rdLabelEntry;

entryListBase *rdPvEntry, *rdPvColorEntry, *rdPvAlarmSensEntry;

entryListBase *limitsFromDbEntry, *precEntry, *scaleMinEntry,
 *scaleMaxEntry;

Widget frameWidget, sliderWidget;

int valueFormX, valueFormY, valueFormW, valueFormH, valueFormMaxH;

double minFv, maxFv, factor, controlV, readV, savedV, newSavedV, oldControlV;
int xRef;

double curControlV, curReadV;

double increment, accelMultiplier;
int controlState, compute_initial_increment, autoSetSavedV;

int bgColorMode;
int controlColorMode;
int readColorMode;
pvColorClass bgColor, fgColor, shadeColor, controlColor,
 readColor;
char controlValue[14+1], readValue[14+1];
char minValue[14+1], maxValue[14+1];
char incString[31+1];
int controlX, controlY, readX, readY,
 arcStart, arcStop, controlW, controlH, readH, valueAreaH, controlAreaW,
 controlAreaH, labelAreaH, savedX;

fontMenuClass fm;
char fontTag[63+1];
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

ProcessVariable *controlPvId, *controlLabelPvId, *readPvId, *readLabelPvId,
 *savedValuePvId;

expStringClass controlPvName, readPvName, savedValuePvName, controlLabelName,
 readLabelName;

char controlLabel[PV_Factory::MAX_PV_NAME+1];
char readLabel[PV_Factory::MAX_PV_NAME+1];
int formatType;
char controlFormat[15+1], readFormat[15+1];

int controlExists, controlLabelExists, readExists, readLabelExists,
 savedValueExists;

char controlLabelTypeStr[15+1], readLabelTypeStr[15+1];
int controlLabelType, readLabelType;

int controlPvConnected, readPvConnected, savedValuePvConnected,
 bufInvalid, active, activeMode, init;

int markX0, markX1, markY0, markY1;
int restoreX0, restoreX1, restoreY0, restoreY1;

int positive;

int needCtlConnectInit, needCtlInfoInit, needCtlRefresh;
int needReadConnectInit, needReadInfoInit, needReadRefresh;
int needCtlLabelConnectInit, needCtlLabelInfoInit;
int needReadLabelConnectInit, needReadLabelInfoInit;
int needSavedConnectInit, needSavedRefresh;
int needErase, needDraw;
int initialConnection, initialReadConnection, initialSavedValueConnection;

VPFUNC changeCallback, activateCallback, deactivateCallback;
int changeCallbackFlag, activateCallbackFlag, deactivateCallbackFlag,
 anyCallbackFlag;

char displayFormat[15+1];
int limitsFromDb;
double scaleMin, scaleMax;
efDouble efScaleMin, efScaleMax;
int precision;
efInt efPrecision;

keypadClass kp;
double kpCtlDouble, kpIncDouble;

int overSave, overRestore, overInc, overControl, overRead;

int oldCtrlStat, oldCtrlSev, oldReadStat, oldReadSev;

public:

activeSliderClass ( void );

activeSliderClass
 ( const activeSliderClass *source );

~activeSliderClass ( void ) {

  if ( name ) delete[] name;
  if ( eBuf ) delete eBuf;

}

char *objName ( void ) {

  return name;

}

void doIncrement ( void );

void doDecrement ( void );

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

void bufInvalidate ( void );

int activate ( int pass, void *ptr );

int deactivate ( int pass );

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

void updateDimensions ( void );

int eraseActivePointers ( void );

int drawActivePointers ( void );

int eraseActiveControlText ( void );

int drawActiveControlText ( void );

int eraseActiveReadText ( void );

int drawActiveReadText ( void );

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

void executeDeferred ( void );

int getProperty (
  char *prop,
  double *_value );

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

void map ( void );
 
void unmap ( void );

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

void *create_activeSliderClassPtr ( void );
void *clone_activeSliderClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
