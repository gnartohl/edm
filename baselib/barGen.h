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

#ifndef __barGen_h
#define __barGen_h 1

#include "act_grf.h"
#include "entry_form.h"

// #ifdef __epics__
// #include "cadef.h"
// #endif

#include "pv.h"

#ifndef __epics__
#define MAX_ENUM_STRING_SIZE 16
#endif

#define BARC_K_COLORMODE_STATIC 0
#define BARC_K_COLORMODE_ALARM 1

#define BARC_MAJOR_VERSION 1
#define BARC_MINOR_VERSION 7
#define BARC_RELEASE 0

#define BARC_K_PV_NAME 0
#define BARC_K_LITERAL 1

#define BARC_K_MAX_GE_MIN 1
#define BARC_K_MAX_LT_MIN 2

#define BARC_K_LIMITS_FROM_DB 1
#define BARC_K_LIMITS_FROM_FORM 0

#ifdef __barGen_cc

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

#if 0
static void bar_controlUpdate (
  struct event_handler_args ast_args );
#endif

static void bar_infoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void bar_readUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void bar_alarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void bar_markerInfoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void bar_markerUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void bar_markerAlarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

#if 0
static void bar_monitor_control_connect_state (
  struct connection_handler_args arg );
#endif

static void bar_monitor_read_connect_state (
  pvClass *classPtr, void *clientData,
  void *args );

static void bar_monitor_marker_connect_state (
  pvClass *classPtr, void *clientData,
  void *args );

#endif

class activeBarClass : public activeGraphicClass {

private:

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

friend void bar_controlUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void bar_infoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void bar_readUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void bar_alarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void bar_monitor_control_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void bar_monitor_read_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void bar_markerInfoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void bar_markerUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void bar_markerAlarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void bar_monitor_marker_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );


int horizontal;

int bufX, bufY, bufW, bufH;

int opComplete;

int minW, minVertW;
int minH, minVertH;

double controlV, curControlV, readV, curReadV, markerV, curMarkerV;
int barY, oldBarY, barH, oldBarH, barW, oldBarW, bufInvalid, barX, oldBarX,
 originW, originH, mode, barAreaX, barAreaW, barAreaY, barAreaH, barStrLen, barMaxW, barMaxH,
 aboveBarOrigin, oldAboveBarOrigin, zeroCrossover;
double barOriginX, barOriginY, factorLt, factorGe;
efDouble efBarOriginX;
int markerX1, markerX2, markerY1, markerY2, 
 oldMarkerX1, oldMarkerX2, oldMarkerY1, oldMarkerY2;


fontMenuClass fm;
char fontTag[63+1];
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

// #ifdef __epics__
// chid controlPvId, readPvId;
// evid controlEventId, readEventId, alarmEventId;
// #endif

pvClass *readPvId, *controlPvId, *markerPvId;
pvEventClass *controlEventId, *readEventId, *alarmEventId, *markerEventId, *markerAlarmEventId;

expStringClass controlPvExpStr, readPvExpStr, markerPvExpStr;
char bufControlPvName[39+1];
char bufReadPvName[39+1];
char bufMarkerPvName [39+1];

int controlExists, readExists, markerExists;

int controlPvConnected, readPvConnected, markerPvConnected, init, active, activeMode;

int barColorMode, fgColorMode, markerColorMode;
int markerWidth;
pvColorClass barColor, fgColor, bgColor, markerColor;
colorButtonClass barCb, fgCb, bgCb;
char label[39+1];
int labelType;
int border;
int showScale;
int labelTicks, majorTicks, minorTicks;
char scaleFormat[3+1];
int limitsFromDb;
double readMin, readMax;
efDouble efReadMin, efReadMax;
int precision;
efInt efPrecision;

int bufBarColorMode, bufFgColorMode, bufMarkerColorMode;
int bufMarkerWidth;
unsigned int bufBarColor, bufFgColor, bufBgColor, bufMarkerColor;
char bufLabel[39+1];
int bufLabelType;
int bufBorder;
int bufShowScale;
int bufLabelTicks, bufMajorTicks, bufMinorTicks;
char bufFontTag[63+1];
char bufScaleFormat[3+1];
int bufLimitsFromDb;
efDouble bufEfReadMin, bufEfReadMax;
efInt bufEfPrecision;
efDouble bufEfBarOriginX;

int needErase, needDraw, needFullDraw, needDrawCheck, needConnectInit, needMarkerConnectInit,
 needRefresh, needMarkerInfoInit, needInfoInit;

char pvClassName[15+1], pvUserClassName[15+1], pvOptionList[255+1];
int numPvTypes, pvNameIndex;


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

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int draw ( void );

int erase ( void );

int drawActive ( void );

int drawMarker ( void );

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
  gcClass *gc );

void drawHorzScale (
  Widget widget,
  gcClass *gc );

void drawVertScale (
  Widget widget,
  gcClass *gc );

void updateScaleInfo ( void );

void updateHorzScaleInfo ( void );

void updateVertScaleInfo ( void );

void updateBar ( void );

void updateMarker ( void );

void executeDeferred ( void );

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
