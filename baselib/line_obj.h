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

#ifndef __line_obj_h
#define __line_obj_h 1

#include "act_grf.h"
#include "entry_form.h"
#include "pv_factory.h"
#include "epics_pv_factory.h"
#include "cvtFast.h"

#define ALC_K_COLORMODE_STATIC 0
#define ALC_K_COLORMODE_ALARM 1

#define ALC_MAJOR_VERSION 2
#define ALC_MINOR_VERSION 0
#define ALC_RELEASE 0

#ifdef __line_obj_cc

#include "line_obj.str"

static char *dragName[] = {
  activeLineClass_str1,
  activeLineClass_str2,
  activeLineClass_str3
};

static void alc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void alc_edit_prop_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void alc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void alc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void alc_edit_prop_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void alc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class activeLineClass : public activeGraphicClass {

private:

friend void alc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void alc_edit_prop_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void alc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void alc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void alc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void alc_edit_prop_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void alc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

int wasSelected;

int bufX, bufY, bufW, bufH;
int oldX, oldY, oldW, oldH;

pointPtr head;
int numPoints;
XPoint *xpoints;

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

int closedPolygon;
int bufClosedPolygon;

int capStyle, joinStyle, lineStyle, lineWidth;
int bufCapStyle, bufJoinStyle, bufLineStyle, bufLineWidth;

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
char bufAlarmPvName[39+1];

expStringClass visPvExpStr;
char bufVisPvName[39+1];

int alarmPvExists, visPvExists;
int activeMode, init, opComplete;

int needConnectInit, needAlarmUpdate, needVisUpdate, needRefresh;

int curLineColorIndex, curFillColorIndex, curStatus, curSeverity;
static const int alarmPvConnection = 1;
static const int visPvConnection = 2;
pvConnectionClass connection;

public:

static void activeLineClass::alarmPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
);

static void activeLineClass::alarmPvValueCallback (
  ProcessVariable *pv,
  void *userarg
);

static void activeLineClass::visPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
);

static void activeLineClass::visPvValueCallback (
  ProcessVariable *pv,
  void *userarg
);

activeLineClass::activeLineClass ( void );

activeLineClass::~activeLineClass ( void );

// copy constructor
activeLineClass::activeLineClass
( const activeLineClass *source );

char *activeLineClass::objName ( void ) {

  return name;

}

int activeLineClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeLineClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int activeLineClass::save (
  FILE *f );

int activeLineClass::genericEdit ( void );

int activeLineClass::edit ( void );

int activeLineClass::editCreate ( void );

int activeLineClass::editLineSegments ( void );

virtual int activeLineClass::addPoint (
  int x,
  int y );

virtual int activeLineClass::removeLastPoint ( void );

virtual pointPtr activeLineClass::selectPoint (
  int x,
  int y );

virtual int activeLineClass::movePoint (
  pointPtr curPoint,
  int x,
  int y );

int activeLineClass::select (
  int x,
  int y );

int activeLineClass::lineEditDone ( void );

int activeLineClass::lineEditComplete ( void );

int activeLineClass::lineEditCancel ( void );

int activeLineClass::draw ( void );

int activeLineClass::erase ( void );

int activeLineClass::drawActive ( void );

int activeLineClass::eraseActive ( void );

int activeLineClass::eraseUnconditional ( void );

int activeLineClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeLineClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeLineClass::containsMacros ( void );

int activeLineClass::activate ( int pass, void *ptr );

int activeLineClass::deactivate ( int pass );

void activeLineClass::updateDimensions ( void );

int activeLineClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int activeLineClass::resizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int activeLineClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

int activeLineClass::resizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

int activeLineClass::rotate (
  int xOrigin,
  int yOrigin,
  char direction );

int activeLineClass::flip (
  int xOrigin,
  int yOrigin,
  char direction );

int activeLineClass::isMultiPointObject ( void );

void activeLineClass::executeDeferred ( void );

char *activeLineClass::firstDragName ( void );

char *activeLineClass::nextDragName ( void );

char *activeLineClass::dragValue (
  int i );

void activeLineClass::changeDisplayParams (
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

void activeLineClass::changePvNames (
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

int activeLineClass::addUndoRotateNode ( undoClass *undoObj );

int activeLineClass::addUndoFlipNode ( undoClass *undoObj );

int activeLineClass::undoRotate (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

int activeLineClass::undoFlip (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeLineClassPtr ( void );
void *clone_activeLineClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
