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

#ifndef __dynSymbol_h
#define __dynSymbol_h 1

#include "act_grf.h"
#include "entry_form.h"

// the following defines btnActionListType & btnActionListPtr
#include "btnActionListType.h"

#ifdef __epics__
#include "cadef.h"
#endif

#define DSC_MAJOR_VERSION 1
#define DSC_MINOR_VERSION 3
#define DSC_RELEASE 0

#define DYNSYMBOL_K_NUM_STATES 32

#ifdef __dynSymbol_cc

#include "dynSymbol.str"

static char *dragName[] = {
  activeDynSymbolClass_str33,
  activeDynSymbolClass_str1,
  activeDynSymbolClass_str2
};

static void dsc_updateControl (
  XtPointer client,
  XtIntervalId *id );

static void dynSymbol_monitor_color_connect_state (
  struct connection_handler_args arg );

static void dynSymbol_colorUpdate (
  struct event_handler_args ast_args );

static void dynSymbol_monitor_gateUp_connect_state (
  struct connection_handler_args arg );

static void dynSymbol_gateUpUpdate (
  struct event_handler_args ast_args );

static void dynSymbol_monitor_gateDown_connect_state (
  struct connection_handler_args arg );

static void dynSymbol_gateDownUpdate (
  struct event_handler_args ast_args );

static void dsc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void dsc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void dsc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void dsc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void dsc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

typedef struct dsObjPlusIndexTag {
  void *objPtr;
  int index;
  unsigned int setMask;
  unsigned int clrMask;
} dsObjPlusIndexType, *dsObjPlusIndexPtr;

class activeDynSymbolClass : public activeGraphicClass {

private:

dsObjPlusIndexType argRec;

void *voidHead[DYNSYMBOL_K_NUM_STATES]; // cast to activeGraphicListPtr
                                     // array at runtime

#ifdef __epics__
chid controlPvId, gateUpPvId, gateDownPvId, colorPvId;
evid controlEventId, gateUpEventId, gateDownEventId, colorEventId;
#endif

int curCount, timerActive, up, down;
XtIntervalId timer;

int useGate, continuous;
double rate;

unsigned int gateUpPvConnected, gateDownPvConnected, colorPvConnected;
int init, active, activeMode, opComplete, gateUpExists, gateDownExists,
 colorExists;
int gateDownValue, gateUpValue;
double curColorV;

int iValue;
double controlVal, controlV, curControlV;
double stateMinValue[DYNSYMBOL_K_NUM_STATES];
double stateMaxValue[DYNSYMBOL_K_NUM_STATES];
char dynSymbolFileName[127+1];
expStringClass controlPvExpStr, gateUpPvExpStr, gateDownPvExpStr,
 colorPvExpStr;

btnActionListPtr btnDownActionHead;
btnActionListPtr btnUpActionHead;
btnActionListPtr btnMotionActionHead;

int useOriginalSize, useOriginalColors;
int numStates;
int index, prevIndex;

entryListBase *elsvMin;
entryListBase *elsvMax;
double *minPtr[DYNSYMBOL_K_NUM_STATES], *maxPtr[DYNSYMBOL_K_NUM_STATES];

int bufX;
int bufY;
int bufW;
int bufH;

double bufStateMinValue[DYNSYMBOL_K_NUM_STATES];
double bufStateMaxValue[DYNSYMBOL_K_NUM_STATES];
char bufDynSymbolFileName[127+1], bufControlPvName[39+1],
 bufGateDownPvName[39+1], bufGateUpPvName[39+1], bufColorPvName[39+1];
int bufNumStates, bufUseOriginalSize, bufUseOriginalColors, bufUseGate,
 bufContinuous;
double bufRate;
int bufGateDownValue, bufGateUpValue;
int initialIndex, bufInitialIndex;

int bufFgColor, bufBgColor;
colorButtonClass fgCb, bgCb;

entryListBase *pvNamesObj;

int needErase, needDraw, needRefresh;
int needGateUp, needGateUpConnect;
int needGateDown, needGateDownConnect;
int needColorInit, needColorRefresh;

public:

friend void dsc_updateControl (
  XtPointer client,
  XtIntervalId *id );

friend void dynSymbol_monitor_color_connect_state (
  struct connection_handler_args arg );

friend void dynSymbol_colorUpdate (
  struct event_handler_args ast_args );

friend void dynSymbol_monitor_gateUp_connect_state (
  struct connection_handler_args arg );

friend void dynSymbol_gateUpUpdate (
  struct event_handler_args ast_args );

friend void dynSymbol_monitor_gateDown_connect_state (
  struct connection_handler_args arg );

friend void dynSymbol_gateDownUpdate (
  struct event_handler_args ast_args );

friend void dsc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void dsc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void dsc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void dsc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void dsc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

activeDynSymbolClass::activeDynSymbolClass ( void );

activeDynSymbolClass::activeDynSymbolClass
 ( const activeDynSymbolClass *source );

activeDynSymbolClass::~activeDynSymbolClass ( void );

int activeDynSymbolClass::genericEdit ( void );

int activeDynSymbolClass::edit ( void );

int activeDynSymbolClass::editCreate ( void );

int activeDynSymbolClass::readDynSymbolFile ( void );

int activeDynSymbolClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeDynSymbolClass::save (
  FILE *f );

int activeDynSymbolClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeDynSymbolClass::draw ( void );

int activeDynSymbolClass::erase ( void );

int activeDynSymbolClass::drawActive ( void );

int activeDynSymbolClass::eraseActive ( void );

int activeDynSymbolClass::activate (
  int pass,
  void *ptr );

int activeDynSymbolClass::deactivate ( int pass );

int activeDynSymbolClass::moveSelectBox (
  int _x,
  int _y );

int activeDynSymbolClass::moveSelectBoxAbs (
  int _x,
  int _y );

int activeDynSymbolClass::moveSelectBoxMidpointAbs (
  int _x,
  int _y );

int activeDynSymbolClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int activeDynSymbolClass::resizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int activeDynSymbolClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

int activeDynSymbolClass::resizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

int activeDynSymbolClass::move (
  int x,
  int y );

int activeDynSymbolClass::moveAbs (
  int x,
  int y );

int activeDynSymbolClass::moveMidpointAbs (
  int x,
  int y );

int activeDynSymbolClass::resize (
  int _x,
  int _y,
  int _w,
  int _h );

int activeDynSymbolClass::resizeAbs (
  int _x,
  int _y,
  int _w,
  int _h );

int activeDynSymbolClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag );

void activeDynSymbolClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

void activeDynSymbolClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

void activeDynSymbolClass::updateGroup ( void );

int activeDynSymbolClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeDynSymbolClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeDynSymbolClass::containsMacros ( void );

void activeDynSymbolClass::executeDeferred ( void );

int activeDynSymbolClass::setProperty (
  char *prop,
  char *value );

char *activeDynSymbolClass::firstDragName ( void );

char *activeDynSymbolClass::nextDragName ( void );

char *activeDynSymbolClass::dragValue (
  int i );

void activeDynSymbolClass::changePvNames (
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

void activeDynSymbolClass::updateColors (
  double colorValue );

int activeDynSymbolClass::rotate (
  int xOrigin,
  int yOrigin,
  char direction ); // '+'=clockwise, '-'=counter clockwise

int activeDynSymbolClass::flip (
  int xOrigin,
  int yOrigin,
  char direction );

int activeDynSymbolClass::undoRotate (
  void *opPtr,
  int x,
  int y,
  int w,
  int h );

int activeDynSymbolClass::undoFlip (
  void *opPtr,
  int x,
  int y,
  int w,
  int h );

};

#endif
