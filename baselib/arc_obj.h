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

#ifndef __arc_obj_h
#define __arc_obj_h 1

#include "act_grf.h"
#include "entry_form.h"

#include "cadef.h"

#define AAC_K_COLORMODE_STATIC 0
#define AAC_K_COLORMODE_ALARM 1

#define AAC_MAJOR_VERSION 2
#define AAC_MINOR_VERSION 0
#define AAC_RELEASE 0

#ifdef __arc_obj_cc

#include "arc_obj.str"

static char *dragName[] = {
  activeArcClass_str1,
  activeArcClass_str2,
  activeArcClass_str3
};

static void aac_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void aac_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void aac_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void aac_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void aac_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void aaoMonitorAlarmPvConnectState (
  struct connection_handler_args arg );

static void arcAlarmUpdate (
  struct event_handler_args ast_args );

static void aaoMonitorVisPvConnectState (
  struct connection_handler_args arg );

static void arcVisUpdate (
  struct event_handler_args ast_args );

#endif

class activeArcClass : public activeGraphicClass {

private:

friend void aac_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void aac_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void aac_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void aac_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void aac_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void aaoMonitorAlarmPvConnectState (
  struct connection_handler_args arg );

friend void arcAlarmUpdate (
  struct event_handler_args ast_args );

friend void aaoMonitorVisPvConnectState (
  struct connection_handler_args arg );

friend void arcVisUpdate (
  struct event_handler_args ast_args );

int bufX, bufY, bufW, bufH;

pvColorClass lineColor;
unsigned int bufLineColor;
colorButtonClass lineCb;

int lineColorMode;
int bufLineColorMode;

int fill;
int bufFill;

pvColorClass fillColor;
unsigned int bufFillColor;
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

int startAngle, totalAngle;
efDouble efStartAngle, efTotalAngle;
efDouble bufEfStartAngle, bufEfTotalAngle;

int fillMode, bufFillMode;

public:

activeArcClass::activeArcClass ( void );

// copy constructor
activeArcClass::activeArcClass
( const activeArcClass *source );

activeArcClass::~activeArcClass ( void ) {

  if ( name ) delete name;

}

char *activeArcClass::objName ( void ) {

  return name;

}

int activeArcClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeArcClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int activeArcClass::importFromXchFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeArcClass::save (
  FILE *f );

int activeArcClass::genericEdit ( void );

int activeArcClass::edit ( void );

int activeArcClass::editCreate ( void );

int activeArcClass::draw ( void );

int activeArcClass::erase ( void );

int activeArcClass::drawActive ( void );

int activeArcClass::eraseActive ( void );

int activeArcClass::eraseUnconditional ( void );

int activeArcClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeArcClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeArcClass::containsMacros ( void );

int activeArcClass::activate ( int pass, void *ptr );

int activeArcClass::deactivate ( int pass );

void activeArcClass::executeDeferred ( void );

char *activeArcClass::firstDragName ( void );

char *activeArcClass::nextDragName ( void );

char *activeArcClass::dragValue (
  int i );

void activeArcClass::changeDisplayParams (
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

void activeArcClass::changePvNames (
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

void *create_activeArcClassPtr ( void );
void *clone_activeArcClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
