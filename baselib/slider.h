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

#include "cadef.h"

#define SLC_MAJOR_VERSION 2
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
  struct connection_handler_args arg );

static void sl_savedValueUpdate (
  struct event_handler_args ast_args );

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
  struct event_handler_args ast_args );

static void sl_infoUpdate (
  struct event_handler_args ast_args );

static void sl_readInfoUpdate (
  struct event_handler_args ast_args );

static void sl_controlLabelUpdate (
  struct event_handler_args ast_args );

static void sl_readLabelUpdate (
  struct event_handler_args ast_args );

static void sl_monitor_control_label_connect_state (
  struct connection_handler_args arg );

static void sl_monitor_read_label_connect_state (
  struct connection_handler_args arg );

static void sl_monitor_control_connect_state (
  struct connection_handler_args arg );

static void sl_readUpdate (
  struct event_handler_args ast_args );

static void sl_monitor_read_connect_state (
  struct connection_handler_args arg );

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
  struct event_handler_args ast_args );

friend void sl_monitor_saved_connect_state (
  struct connection_handler_args arg );

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
  struct event_handler_args ast_args );

friend void sl_infoUpdate (
  struct event_handler_args ast_args );

friend void sl_readInfoUpdate (
  struct event_handler_args ast_args );

friend void sl_controlLabelUpdate (
  struct event_handler_args ast_args );

friend void sl_readLabelUpdate (
  struct event_handler_args ast_args );

friend void sl_monitor_control_label_connect_state (
  struct connection_handler_args arg );

friend void sl_monitor_read_label_connect_state (
  struct connection_handler_args arg );

friend void sl_monitor_control_connect_state (
  struct connection_handler_args arg );

friend void sl_readUpdate (
  struct event_handler_args ast_args );

friend void sl_monitor_read_connect_state (
  struct connection_handler_args arg );

XtIntervalId updateControlTimer;
int updateControlTimerValue;
int updateControlTimerActive;
int controlAdjusted;

float oneControlV;

XtIntervalId incrementTimer;
int incrementTimerActive;
int incrementTimerValue;

int opComplete;

int minW;
int minH;

int bufX, bufY, bufW, bufH;

Widget frameWidget, sliderWidget;

double bufControlV, bufIncrement;
int valueFormX, valueFormY, valueFormW, valueFormH, valueFormMaxH;

float minFv, maxFv, factor, controlV, readV, savedV, newSavedV, oldControlV;
int xRef;

float curControlV, curReadV;

float increment;
int controlState, compute_initial_increment, autoSetSavedV;

int fgColorMode, bufFgColorMode;
int controlColorMode, bufControlColorMode;
int readColorMode, bufReadColorMode;
pvColorClass bgColor, fgColor, shadeColor, controlColor,
 readColor;
unsigned int bufBgColor;
unsigned int bufFgColor;
unsigned int bufShadeColor;
unsigned int bufControlColor;
unsigned int bufReadColor;
colorButtonClass fgCb, bgCb, shadeCb, controlCb, readCb;
char controlValue[14+1], readValue[14+1];
char minValue[14+1], maxValue[14+1];
char incString[31+1];
int controlX, bufControlX, controlY, readX, bufReadX, readY,
 arcStart, arcStop, controlW, controlH, readH, valueAreaH, controlAreaW,
 controlAreaH, labelAreaH, savedX;

fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

chid controlPvId, controlLabelPvId, readPvId, readLabelPvId, savedValuePvId;
evid controlEventId, readEventId, controlLabelEventId,
 readLabelEventId, savedEventId;

expStringClass controlPvName, readPvName, savedValuePvName, controlLabelName,
 readLabelName;

char controlBufPvName[39+1];
char readBufPvName[39+1];
char savedValueBufPvName[39+1];
char controlLabel[39+1];
char controlBufLabelName[39+1];
char readLabel[39+1];
char readBufLabelName[39+1];
int formatType, bufFormatType;
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

VPFUNC changeCallback, activateCallback, deactivateCallback;
int changeCallbackFlag, activateCallbackFlag, deactivateCallbackFlag,
 anyCallbackFlag;
int bufChangeCallbackFlag, bufActivateCallbackFlag, bufDeactivateCallbackFlag;

char displayFormat[3+1];
int limitsFromDb;
double scaleMin, scaleMax;
efDouble efScaleMin, efScaleMax;
int precision;
efInt efPrecision;

int bufLimitsFromDb;
efDouble bufEfScaleMin, bufEfScaleMax;
efInt bufEfPrecision;
char bufDisplayFormat[3+1];

keypadClass kp;
double kpCtlDouble, kpIncDouble;

public:

activeSliderClass::activeSliderClass ( void );

activeSliderClass::activeSliderClass
 ( const activeSliderClass *source );

activeSliderClass::~activeSliderClass ( void ) {

  if ( name ) delete name;

}

char *activeSliderClass::objName ( void ) {

  return name;

}

int activeSliderClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeSliderClass::save (
  FILE *f );

int activeSliderClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeSliderClass::genericEdit ( void );

int activeSliderClass::edit ( void );

int activeSliderClass::editCreate ( void );

int activeSliderClass::draw ( void );

int activeSliderClass::erase ( void );

int activeSliderClass::drawActive ( void );

int activeSliderClass::eraseActive ( void );

void activeSliderClass::bufInvalidate ( void );

int activeSliderClass::activate ( int pass, void *ptr );

int activeSliderClass::deactivate ( int pass );

int activeSliderClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int activeSliderClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

void activeSliderClass::updateDimensions ( void );

int activeSliderClass::eraseActivePointers ( void );

int activeSliderClass::drawActivePointers ( void );

int activeSliderClass::eraseActiveControlText ( void );

int activeSliderClass::drawActiveControlText ( void );

int activeSliderClass::eraseActiveReadText ( void );

int activeSliderClass::drawActiveReadText ( void );

int activeSliderClass::createWidgets ( void );

int activeSliderClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeSliderClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeSliderClass::containsMacros ( void );

void activeSliderClass::executeDeferred ( void );

int activeSliderClass::getProperty (
  char *prop,
  double *_value );

char *activeSliderClass::firstDragName ( void );

char *activeSliderClass::nextDragName ( void );

char *activeSliderClass::dragValue (
  int i );

void activeSliderClass::changeDisplayParams (
  unsigned int flag,
  char *fontTag,
  int alignment,
  char *ctlFontTag,
  int ctlAlignment,
  char *btnFontTag,
  int btnAlignment,
  unsigned int textFgColor,
  unsigned int fg1Color,
  unsigned int fg2Color,
  unsigned int offsetColor,
  unsigned int bgColor,
  unsigned int topShadowColor,
  unsigned int botShadowColor );

void activeSliderClass::changePvNames (
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
