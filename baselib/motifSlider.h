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

#include "cadef.h"

#define MSLC_MAJOR_VERSION 1
#define MSLC_MINOR_VERSION 0
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
};

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

static void msloSetCtlKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void msloSetIncKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void msloCancelKp (
  Widget w,
  XtPointer client,
  XtPointer call );

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

static void sl_controlUpdate (
  struct event_handler_args ast_args );

static void sl_infoUpdate (
  struct event_handler_args ast_args );

static void sl_controlLabelUpdate (
  struct event_handler_args ast_args );

static void sl_monitor_control_label_connect_state (
  struct connection_handler_args arg );

static void sl_monitor_control_connect_state (
  struct connection_handler_args arg );

#endif

class activeMotifSliderClass : public activeGraphicClass {

private:

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

friend void msloSetCtlKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void msloSetIncKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void msloCancelKp (
  Widget w,
  XtPointer client,
  XtPointer call );

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

friend void sl_controlUpdate (
  struct event_handler_args ast_args );

friend void sl_infoUpdate (
  struct event_handler_args ast_args );

friend void sl_controlLabelUpdate (
  struct event_handler_args ast_args );

friend void sl_monitor_control_label_connect_state (
  struct connection_handler_args arg );

friend void sl_monitor_control_connect_state (
  struct connection_handler_args arg );

XtIntervalId updateControlTimer;
int updateControlTimerValue;
int updateControlTimerActive;
int controlAdjusted;

double oneControlV, savedV;

XtIntervalId incrementTimer;
int incrementTimerActive;
int incrementTimerValue;

int opComplete;

int minW;
int minH;

int bufX, bufY, bufW, bufH;

Widget frameWidget, motifSliderWidget, scaleWidget, scrollBarWidget;

int showLimits, bufShowLimits, showValue, bufShowValue, showLabel,
 bufShowLabel;

double bufControlV, bufIncrement;
int valueFormX, valueFormY, valueFormW, valueFormH, valueFormMaxH;

double minFv, maxFv, factor, controlV, oldControlV;
int prevScaleV, dragIndicator;

double curControlV;

double increment;
int compute_initial_increment;

int fgColorMode, bufFgColorMode;
pvColorClass bgColor, fgColor;
int bufBgColor, bufFgColor;
int shadeColor, bufShadeColor, topColor, bufTopColor, botColor, bufBotColor;
colorButtonClass fgCb, bgCb, shadeCb, topCb, botCb;
char controlValue[14+1];
char minValue[14+1], maxValue[14+1];
char incString[31+1];

int minX, minY, maxX, maxY, valX, valY, labelX, labelY, scaleX, scaleY,
 scaleW, scaleH, controlX, savedX, limitsH, labelH, midVertScaleY;

fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

chid controlPvId, controlLabelPvId;
evid controlEventId, controlLabelEventId;

expStringClass controlPvName, controlLabelName;

char controlBufPvName[39+1];
char controlLabel[39+1];
char controlBufLabelName[39+1];
int formatType, bufFormatType;
char controlFormat[15+1];

int controlExists, controlLabelExists;

int controlLabelType, bufControlLabelType;

int controlPvConnected, bufInvalid, active, activeMode, init;

int positive;

int needCtlConnectInit, needCtlInfoInit, needCtlRefresh;
int needCtlLabelConnectInit, needCtlLabelInfoInit;
int needErase, needDraw;

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

keypadClass kp;
double kpCtlDouble, kpIncDouble;

public:

activeMotifSliderClass::activeMotifSliderClass ( void );

activeMotifSliderClass::activeMotifSliderClass
 ( const activeMotifSliderClass *source );

activeMotifSliderClass::~activeMotifSliderClass ( void ) {

  if ( name ) delete name;

}

char *activeMotifSliderClass::objName ( void ) {

  return name;

}

int activeMotifSliderClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeMotifSliderClass::save (
  FILE *f );

int activeMotifSliderClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeMotifSliderClass::genericEdit ( void );

int activeMotifSliderClass::edit ( void );

int activeMotifSliderClass::editCreate ( void );

int activeMotifSliderClass::draw ( void );

int activeMotifSliderClass::erase ( void );

int activeMotifSliderClass::drawActive ( void );

int activeMotifSliderClass::eraseActive ( void );

void activeMotifSliderClass::bufInvalidate ( void );

int activeMotifSliderClass::activate ( int pass, void *ptr );

int activeMotifSliderClass::deactivate ( int pass );

int activeMotifSliderClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int activeMotifSliderClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

void activeMotifSliderClass::updateDimensions ( void );

int activeMotifSliderClass::eraseActiveControlText ( void );

int activeMotifSliderClass::drawActiveControlText ( void );

int activeMotifSliderClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeMotifSliderClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeMotifSliderClass::containsMacros ( void );

void activeMotifSliderClass::executeDeferred ( void );

int activeMotifSliderClass::getProperty (
  char *prop,
  double *_value );

char *activeMotifSliderClass::firstDragName ( void );

char *activeMotifSliderClass::nextDragName ( void );

char *activeMotifSliderClass::dragValue (
  int i );

void activeMotifSliderClass::changeDisplayParams (
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

void activeMotifSliderClass::changePvNames (
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

void *create_activeMotifSliderClassPtr ( void );
void *clone_activeMotifSliderClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
