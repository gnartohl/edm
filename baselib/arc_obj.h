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

#include "pv_factory.h"
#include "epics_pv_factory.h"
#include "cvtFast.h"

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

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

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

#endif

class activeArcClass : public activeGraphicClass {

private:

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

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

int lineWidth, bufLineWidth;
int lineStyle, bufLineStyle;

int needConnectInit, needAlarmUpdate, needVisUpdate, needRefresh;
int needToDrawUnconnected, needToEraseUnconnected;
int unconnectedTimer;

int curLineColorIndex, curFillColorIndex, curStatus, curSeverity;
static const int alarmPvConnection = 1;
static const int visPvConnection = 2;
pvConnectionClass connection;

int startAngle, totalAngle;
efDouble efStartAngle, efTotalAngle;
efDouble bufEfStartAngle, bufEfTotalAngle;

int fillMode, bufFillMode;

public:

static void activeArcClass::alarmPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
);

static void activeArcClass::alarmPvValueCallback (
  ProcessVariable *pv,
  void *userarg
);

static void activeArcClass::visPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
);

static void activeArcClass::visPvValueCallback (
  ProcessVariable *pv,
  void *userarg
);

activeArcClass::activeArcClass ( void );

// copy constructor
activeArcClass::activeArcClass
( const activeArcClass *source );

activeArcClass::~activeArcClass ( void );

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
  int textFgColor,
  int fg1Color,
  int fg2Color,
  int offsetColor,
  int bgColor,
  int topShadowColor,
  int botShadowColor );

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

void activeArcClass::updateColors (
  double colorValue );

int activeArcClass::rotate (
  int xOrigin,
  int yOrigin,
  char direction ); // '+'=clockwise, '-'=counter clockwise

int activeArcClass::flip (
  int xOrigin,
  int yOrigin,
  char direction ); // 'H' or 'V'

int activeArcClass::addUndoRotateNode ( 
  undoClass *undoObj );

int activeArcClass::addUndoFlipNode (
  undoClass *undoObj );

int activeArcClass::undoRotate (
  undoOpClass *_opPtr,
  int _x,
  int _y,
  int _w,
  int _h );

int activeArcClass::undoFlip (
  undoOpClass *_opPtr,
  int x,
  int y,
  int w,
  int h );

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
