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

#ifndef __circle_obj_h
#define __circle_obj_h 1

#include "act_grf.h"
#include "entry_form.h"

#include "cadef.h"

#define ACC_K_COLORMODE_STATIC 0
#define ACC_K_COLORMODE_ALARM 1

#define ACC_MAJOR_VERSION 2
#define ACC_MINOR_VERSION 0
#define ACC_RELEASE 0

#ifdef __circle_obj_cc

#include "circle_obj.str"

static char *dragName[] = {
  activeCircleClass_str1,
  activeCircleClass_str2,
  activeCircleClass_str3
};

static void acc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void acc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void acc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void acc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void acc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void acoMonitorAlarmPvConnectState (
  struct connection_handler_args arg );

static void circleAlarmUpdate (
  struct event_handler_args ast_args );

static void acoMonitorVisPvConnectState (
  struct connection_handler_args arg );

static void circleVisUpdate (
  struct event_handler_args ast_args );

#endif

class activeCircleClass : public activeGraphicClass {

private:

friend void acc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void acc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void acc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void acc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void acc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void acoMonitorAlarmPvConnectState (
  struct connection_handler_args arg );

friend void circleAlarmUpdate (
  struct event_handler_args ast_args );

friend void acoMonitorVisPvConnectState (
  struct connection_handler_args arg );

friend void circleVisUpdate (
  struct event_handler_args ast_args );

int bufX, bufY, bufW, bufH;

pvColorClass lineColor;
int bufLineColor;
colorButtonClass lineCb;

int lineColorMode;
int bufLineColorMode;

int fill;
int bufFill;

pvColorClass fillColor;
int bufFillColor;
colorButtonClass fillCb;

int fillColorMode;
int bufFillColorMode;

int pvType;
pvValType pvValue, minVis, maxVis;
char minVisString[39+1], bufMinVisString[39+1];
char maxVisString[39+1], bufMaxVisString[39+1];

int prevVisibility, visibility, visInverted, bufVisInverted;

chid alarmPvId;
evid alarmEventId;
chid visPvId;
evid visEventId;

expStringClass alarmPvExpStr;
char bufAlarmPvName[39+1];

expStringClass visPvExpStr;
char bufVisPvName[39+1];

int alarmPvExists, alarmPvConnected, visPvExists, visPvConnected;
int active, activeMode, init, opComplete;

int lineWidth, bufLineWidth;
int lineStyle, bufLineStyle;

int needVisConnectInit;
int needAlarmConnectInit;
int needDraw, needErase, needRefresh;

public:

activeCircleClass::activeCircleClass ( void );

// copy constructor
activeCircleClass::activeCircleClass
( const activeCircleClass *source );

activeCircleClass::~activeCircleClass ( void ) {

  if ( name ) delete name;

}

char *activeCircleClass::objName ( void ) {

  return name;

}

int activeCircleClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeCircleClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int activeCircleClass::importFromXchFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeCircleClass::save (
  FILE *f );

int activeCircleClass::genericEdit ( void );

int activeCircleClass::edit ( void );

int activeCircleClass::editCreate ( void );

int activeCircleClass::draw ( void );

int activeCircleClass::erase ( void );

int activeCircleClass::drawActive ( void );

int activeCircleClass::eraseActive ( void );

int activeCircleClass::eraseUnconditional ( void );

int activeCircleClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeCircleClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeCircleClass::containsMacros ( void );

int activeCircleClass::activate ( int pass, void *ptr );

int activeCircleClass::deactivate ( int pass );

void activeCircleClass::executeDeferred ( void );

char *activeCircleClass::firstDragName ( void );

char *activeCircleClass::nextDragName ( void );

char *activeCircleClass::dragValue (
  int i );

void activeCircleClass::changeDisplayParams (
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

void activeCircleClass::changePvNames (
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

void *create_activeCircleClassPtr ( void );
void *clone_activeCircleClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
