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

#ifndef __rectangle_obj_h
#define __rectangle_obj_h 1

#include "act_grf.h"
#include "entry_form.h"
#include "app_pkg.h"
#include "act_win.h"

#include "pv_factory.h"
#include "epics_pv_factory.h"
#include "cvtFast.h"

#define ARC_K_COLORMODE_STATIC 0
#define ARC_K_COLORMODE_ALARM 1

#define ARC_MAJOR_VERSION 2
#define ARC_MINOR_VERSION 0
#define ARC_RELEASE 0

#ifdef __rectangle_obj_cc

#include "rectangle_obj.str"

static char *dragName[] = {
  activeRectangleClass_str1,
  activeRectangleClass_str2,
  activeRectangleClass_str3
};

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void arc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void arc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void arc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void arc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void arc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

class activeRectangleClass : public activeGraphicClass {

#endif

private:

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void arc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void arc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void arc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void arc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void arc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

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

int visibility, prevVisibility, visInverted, bufVisInverted;
int lineVisibility, prevLineVisibility;
int fillVisibility, prevFillVisibility;

ProcessVariable *alarmPvId;
ProcessVariable *visPvId;

expStringClass alarmPvExpStr;
char bufAlarmPvName[PV_Factory::MAX_PV_NAME+1];

expStringClass visPvExpStr;
char bufVisPvName[PV_Factory::MAX_PV_NAME+1];

int alarmPvExists, visPvExists;
int activeMode, init, opComplete;

int invisible, bufInvisible;
int lineWidth, bufLineWidth;
int lineStyle, bufLineStyle;

int needConnectInit, needAlarmUpdate, needVisUpdate, needRefresh;
int needToDrawUnconnected, needToEraseUnconnected;
int unconnectedTimer;

int curLineColorIndex, curFillColorIndex, curStatus, curSeverity;

static const int alarmPvConnection = 1;
static const int visPvConnection = 2;
pvConnectionClass connection;

public:

static void activeRectangleClass::alarmPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
);

static void activeRectangleClass::alarmPvValueCallback (
  ProcessVariable *pv,
  void *userarg
);

static void activeRectangleClass::visPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
);

static void activeRectangleClass::visPvValueCallback (
  ProcessVariable *pv,
  void *userarg
);

activeRectangleClass::activeRectangleClass ( void );

// copy constructor
activeRectangleClass::activeRectangleClass
 ( const activeRectangleClass *source );

activeRectangleClass::~activeRectangleClass ( void );

char *activeRectangleClass::objName ( void ) {

  return name;

}

int activeRectangleClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeRectangleClass::save (
  FILE *f );

int activeRectangleClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeRectangleClass::importFromXchFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeRectangleClass::genericEdit ( void );

int activeRectangleClass::edit ( void );

int activeRectangleClass::editCreate ( void );

int activeRectangleClass::draw ( void );

int activeRectangleClass::drawActive ( void );

int activeRectangleClass::erase ( void );

int activeRectangleClass::eraseActive ( void );

int activeRectangleClass::eraseUnconditional ( void );

int activeRectangleClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeRectangleClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeRectangleClass::containsMacros ( void );

int activeRectangleClass::activate ( int pass, void *ptr );

int activeRectangleClass::deactivate ( int pass );

int activeRectangleClass::isInvisible ( void )
{
  return invisible;
}

void activeRectangleClass::executeDeferred ( void );

char *activeRectangleClass::firstDragName ( void );

char *activeRectangleClass::nextDragName ( void );

char *activeRectangleClass::dragValue (
  int i );

void activeRectangleClass::changeDisplayParams (
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

void activeRectangleClass::changePvNames (
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

void activeRectangleClass::updateColors (
  double colorValue );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeRectangleClassPtr ( void );
void *clone_activeRectangleClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
