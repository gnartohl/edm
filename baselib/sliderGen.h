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

#ifndef __sliderGen_h
#define __sliderGen_h 1

#include "ulBindings.h"
#include "act_grf.h"
#include "entry_form.h"
#include "utility.h"

// #ifdef __epics__
// #include "cadef.h"
// #endif

#include "pv.h"

#define SLC_MAJOR_VERSION 1
#define SLC_MINOR_VERSION 5
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

#ifdef __sliderGen_cc

static void sl_monitor_saved_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void sl_savedValueUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

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
  pvClass *classPtr,
  void *clientData,
  void *args );

static void sl_infoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void sl_readInfoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void sl_controlLabelUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void sl_readLabelUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void sl_monitor_control_label_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void sl_monitor_read_label_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void sl_monitor_control_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void sl_readUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void sl_monitor_read_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

#endif

class activeSliderClass : public activeGraphicClass {

private:

friend void sl_savedValueUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void sl_monitor_saved_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

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
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void sl_infoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void sl_readInfoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void sl_controlLabelUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void sl_readLabelUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void sl_monitor_control_label_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void sl_monitor_read_label_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void sl_monitor_control_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void sl_readUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void sl_monitor_read_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

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

pvClass *controlPvId, *controlLabelPvId, *readPvId, *readLabelPvId;
pvClass *savedValuePvId;
pvEventClass *controlEventId, *readEventId, *controlLabelEventId;
pvEventClass *readLabelEventId, *savedEventId;

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

char pvClassName[15+1], pvUserClassName[15+1], pvOptionList[255+1];
int numPvTypes, pvNameIndex;

public:

activeSliderClass ( void );

activeSliderClass
 ( const activeSliderClass *source );

~activeSliderClass ( void ) {

/*   printf( "In ~activeSliderClass\n" ); */

  if ( name ) delete name;

}

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

int createWidgets ( void );

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
