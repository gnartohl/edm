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

#ifndef __motifSlider_h
#define __motifSlider_h 1

#include "ulBindings.h"
#include "act_grf.h"
#include "entry_form.h"
#include "utility.h"
#include "keypad.h"

#include "Xm/Scale.h"
#include "Xm/ScrollBar.h"

#include "pv_factory.h"
#include "cvtFast.h"

#define MSLC_MAJOR_VERSION 4
#define MSLC_MINOR_VERSION 2
#define MSLC_RELEASE 0

#define MSLC_STATE_IDLE 1
#define MSLC_STATE_MOVING 2

#define MSLC_K_LITERAL 0
#define MSLC_K_LABEL 1
#define MSLC_K_PV_NAME 2

#define MSLC_K_FORMAT_FLOAT 0
#define MSLC_K_FORMAT_EXPONENTIAL 1

#define MSLC_K_COLORMODE_STATIC 0
#define MSLC_K_COLORMODE_ALARM 1

#define MSLC_K_HORIZONTAL 0
#define MSLC_K_VERTICAL 1

#ifdef __motifSlider_cc

#include "motifSlider.str"

static char *dragName[] = {
  activeMotifSliderClass_str36,
  activeMotifSliderClass_str48
};

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void changeParams (
  Widget w,
  XEvent *e,
  String *params,
  Cardinal numParams );

static void msloValueChangeCB (
  Widget w,
  XtPointer client,
  XtPointer call );

static void msloIndicatorDragCB (
  Widget w,
  XtPointer client,
  XtPointer call );

static void dummy (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

static void drag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

static void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

static void selectActions (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

static void pvInfo (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

static void scrollBarEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch );

static void motifSliderEventHandler(
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch );

static void mslc_updateControl (
  XtPointer client,
  XtIntervalId *id );

static void mslc_value_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mslc_value_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mslc_value_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mslc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mslc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mslc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mslc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mslc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

class activeMotifSliderClass : public activeGraphicClass {

private:

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void changeParams (
  Widget w,
  XEvent *e,
  String *params,
  Cardinal numParams );

friend void msloValueChangeCB (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void msloIndicatorDragCB (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void dummy (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

friend void drag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

friend void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

friend void selectActions (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

friend void pvInfo (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

friend void msloSetIncKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void msloCancelKp (
  Widget w,
  XtPointer client,
  XtPointer call );
#endif

friend void scrollBarEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch );

friend void motifSliderEventHandler(
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch );

friend void mslc_updateControl (
  XtPointer client,
  XtIntervalId *id );

friend void mslc_value_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mslc_value_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mslc_value_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mslc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mslc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mslc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mslc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mslc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

XtIntervalId updateControlTimer;
int updateControlTimerValue;
int updateControlTimerActive;

double oneControlV, savedV, newSavedV, autoSetSavedV;

XtIntervalId incrementTimer;
int incrementTimerActive;
int incrementTimerValue;

int opComplete;

int minW;
int minH;

typedef struct editBufTag {
// edit buffer
  char controlBufPvName[PV_Factory::MAX_PV_NAME+1];
  char controlBufLabelName[PV_Factory::MAX_PV_NAME+1];
  char savedValueBufPvName[PV_Factory::MAX_PV_NAME+1];
} editBufType, *editBufPtr;

editBufPtr eBuf;

entryListBase *labelTypeEntry, *labelEntry;

entryListBase *limitsFromDbEntry, *scalePrecEntry, *scaleMinEntry,
 *scaleMaxEntry;

int bufX, bufY, bufW, bufH;

Widget frameWidget, motifSliderWidget, scaleWidget, scrollBarWidget;

int showLimits, bufShowLimits, showValue, bufShowValue, showLabel,
 bufShowLabel, showSavedValue, bufShowSavedValue;

double bufControlV, bufIncrement;
int valueFormX, valueFormY, valueFormW, valueFormH, valueFormMaxH;

double minFv, maxFv, factor, controlV, oldControlV;
int prevScaleV, dragIndicator;

double curControlV;

double increment;
int compute_initial_increment;

int bgColorMode, bufBgColorMode;
pvColorClass bgColor, fgColor;
int bufBgColor, bufFgColor;
int shadeColor, bufShadeColor, topColor, bufTopColor, botColor, bufBotColor;
colorButtonClass fgCb, bgCb, shadeCb, topCb, botCb;
char controlValue[14+1], savedValue[14+1];
char minValue[14+1], maxValue[14+1];
char incString[31+1];

int minX, minY, maxX, maxY, valX, valY, labelX, labelY, scaleX, scaleY,
 scaleW, scaleH, controlX, limitsH, labelH, midVertScaleY,
 midVertScaleY1, midVertScaleY2;

fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

ProcessVariable *controlPvId, *controlLabelPvId, *savedValuePvId;

expStringClass controlPvName, controlLabelName, savedValuePvName;

char controlLabel[PV_Factory::MAX_PV_NAME+1];
int formatType, bufFormatType;
char controlFormat[15+1];

int controlExists, controlLabelExists, savedValueExists;

int controlLabelType, bufControlLabelType;

int controlPvConnected, savedValuePvConnected, bufInvalid, active,
 activeMode, init;

int positive;

int needCtlConnectInit, needCtlInfoInit, needCtlRefresh;
int needCtlLabelConnectInit, needCtlLabelInfoInit, needCtlUpdate;
int needSavedConnectInit, needSavedRefresh;
int needErase, needDraw;
int needToDrawUnconnected, needToEraseUnconnected;
XtIntervalId unconnectedTimer;

char displayFormat[15+1];
int limitsFromDb;
double scaleMin, scaleMax;
efDouble efScaleMin, efScaleMax;
int precision;
efInt efPrecision;

int bufLimitsFromDb;
efDouble bufEfScaleMin, bufEfScaleMax;
efInt bufEfPrecision;
char bufDisplayFormat[15+1];

int orientation, bufOrientation;

double incArray[7];
int incIndex;

int oldStat, oldSev; // for alarms

int isMapped, buttonPressed, needUnmap;

int keySensitive;

public:

activeMotifSliderClass ( void );

activeMotifSliderClass
 ( const activeMotifSliderClass *source );

~activeMotifSliderClass ( void );

static void controlUpdate (
  ProcessVariable *pv,
  void *userarg );

static void infoUpdate (
  ProcessVariable *pv,
  void *userarg );

static void controlLabelUpdate (
  ProcessVariable *pv,
  void *userarg );

static void savedValueUpdate (
  ProcessVariable *pv,
  void *userarg );

static void monitorControlLabelConnectState (
  ProcessVariable *pv,
  void *userarg );

static void monitorControlConnectState (
  ProcessVariable *pv,
  void *userarg );

static void monitorSavedValueConnectState (
  ProcessVariable *pv,
  void *userarg );

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

int eraseActiveControlText ( void );

int drawActiveControlText ( void );

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

void *create_activeMotifSliderClassPtr ( void );
void *clone_activeMotifSliderClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
