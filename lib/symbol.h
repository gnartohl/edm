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

#ifndef __symbol_h
#define __symbol_h 1

#include "act_grf.h"
#include "undo.h"
#include "entry_form.h"

// the following defines btnActionListType & btnActionListPtr
#include "btnActionListType.h"

#include "cadef.h"

#define ASC_MAJOR_VERSION 1
#define ASC_MINOR_VERSION 4
#define ASC_RELEASE 0

#define SYMBOL_K_NUM_STATES 32
#define SYMBOL_K_MAX_PVS 5

#define OR_ORIG 0
#define OR_CW 1
#define OR_CCW 2
#define OR_V 3
#define OR_H 4

#ifdef __symbol_cc

#include "symbol.str"

static char *dragName[] = {
  activeSymbolClass_str1,
};

static char *dragNameTruthTable[] = {
  activeSymbolClass_str2,
  activeSymbolClass_str3,
  activeSymbolClass_str4,
  activeSymbolClass_str5,
  activeSymbolClass_str6,
};

static void symbol_monitor_control_connect_state (
  struct connection_handler_args arg );

static void symbol_controlUpdate (
  struct event_handler_args ast_args );

static void symbolSetItem (
  Widget w,
  XtPointer client,
  XtPointer call );

static void asc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void asc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void asc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void asc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void asc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

typedef struct objPlusIndexTag {
  void *objPtr;
  int index;
  unsigned int setMask;
  unsigned int clrMask;
} objPlusIndexType, *objPlusIndexPtr;

class activeSymbolClass : public activeGraphicClass {

private:

objPlusIndexType argRec[SYMBOL_K_MAX_PVS];

void *voidHead[SYMBOL_K_NUM_STATES]; // cast to activeGraphicListPtr
                                     // array at runtime

int numPvs;

chid controlPvId[SYMBOL_K_MAX_PVS];
evid controlEventId[SYMBOL_K_MAX_PVS];

unsigned int notControlPvConnected;
int init, active, activeMode, opComplete, controlExists;

int iValue;
double controlVals[SYMBOL_K_MAX_PVS], controlV, curControlV;
double stateMinValue[SYMBOL_K_NUM_STATES];
double stateMaxValue[SYMBOL_K_NUM_STATES];
char symbolFileName[127+1];
expStringClass controlPvExpStr[SYMBOL_K_MAX_PVS];

btnActionListPtr btnDownActionHead;
btnActionListPtr btnUpActionHead;
btnActionListPtr btnMotionActionHead;

int useOriginalSize;
int numStates;
int index, prevIndex;

entryListBase *elsvMin;
entryListBase *elsvMax;
double *minPtr[SYMBOL_K_NUM_STATES], *maxPtr[SYMBOL_K_NUM_STATES];

int bufX;
int bufY;
int bufW;
int bufH;

double bufStateMinValue[SYMBOL_K_NUM_STATES];
double bufStateMaxValue[SYMBOL_K_NUM_STATES];
char bufSymbolFileName[127+1], bufControlPvName[SYMBOL_K_MAX_PVS][39+1];
int bufNumStates, bufUseOriginalSize;

int binaryTruthTable, bufBinaryTruthTable;

entryListBase *pvNamesObj;

int needErase, needDraw, needConnectInit, needConnect[SYMBOL_K_MAX_PVS],
 needRefresh;

undoClass undoObj;

int orientation, bufOrientation, prevOr;

public:

friend void symbol_monitor_control_connect_state (
  struct connection_handler_args arg );

friend void symbol_controlUpdate (
  struct event_handler_args ast_args );

friend void symbolSetItem (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void asc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void asc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void asc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void asc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void asc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

activeSymbolClass::activeSymbolClass ( void );

activeSymbolClass::activeSymbolClass
 ( const activeSymbolClass *source );

activeSymbolClass::~activeSymbolClass ( void );

int activeSymbolClass::genericEdit ( void );

int activeSymbolClass::edit ( void );

int activeSymbolClass::editCreate ( void );

int activeSymbolClass::readSymbolFile ( void );

int activeSymbolClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeSymbolClass::save (
  FILE *f );

int activeSymbolClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeSymbolClass::draw ( void );

int activeSymbolClass::erase ( void );

int activeSymbolClass::drawActive ( void );

int activeSymbolClass::eraseActive ( void );

int activeSymbolClass::activate (
  int pass,
  void *ptr );

int activeSymbolClass::deactivate ( int pass );

int activeSymbolClass::moveSelectBox (
  int _x,
  int _y );

int activeSymbolClass::moveSelectBoxAbs (
  int _x,
  int _y );

int activeSymbolClass::moveSelectBoxMidpointAbs (
  int _x,
  int _y );

int activeSymbolClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int activeSymbolClass::resizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int activeSymbolClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

int activeSymbolClass::resizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

int activeSymbolClass::resizeSelectBoxAbsFromUndo (
  int _x,
  int _y,
  int _w,
  int _h );

int activeSymbolClass::move (
  int x,
  int y );

int activeSymbolClass::moveAbs (
  int x,
  int y );

int activeSymbolClass::moveMidpointAbs (
  int x,
  int y );

int activeSymbolClass::resize (
  int _x,
  int _y,
  int _w,
  int _h );

int activeSymbolClass::resizeAbs (
  int _x,
  int _y,
  int _w,
  int _h );

int activeSymbolClass::resizeAbsFromUndo (
  int _x,
  int _y,
  int _w,
  int _h );

int activeSymbolClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag );

void activeSymbolClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

void activeSymbolClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

void activeSymbolClass::updateGroup ( void );

int activeSymbolClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeSymbolClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeSymbolClass::containsMacros ( void );

void activeSymbolClass::executeDeferred ( void );

int activeSymbolClass::setProperty (
  char *prop,
  int *value );

char *activeSymbolClass::firstDragName ( void );

char *activeSymbolClass::nextDragName ( void );

char *activeSymbolClass::dragValue (
  int i );

void activeSymbolClass::changePvNames (
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

int activeSymbolClass::rotate (
  int xOrigin,
  int yOrigin,
  char direction ); // '+'=clockwise, '-'=counter clockwise

int activeSymbolClass::flip (
  int xOrigin,
  int yOrigin,
  char direction );

void activeSymbolClass::flushUndo ( void );

int activeSymbolClass::addUndoCreateNode ( undoClass *_undoObj );

int activeSymbolClass::addUndoMoveNode ( undoClass *_undoObj );

int activeSymbolClass::addUndoResizeNode ( undoClass *_undoObj );

int activeSymbolClass::addUndoCopyNode ( undoClass *_undoObj );

int activeSymbolClass::addUndoCutNode ( undoClass *_undoObj );

int activeSymbolClass::addUndoPasteNode ( undoClass *_undoObj );

int activeSymbolClass::addUndoReorderNode ( undoClass *_undoObj );

int activeSymbolClass::addUndoEditNode ( undoClass *_undoObj );

int activeSymbolClass::addUndoGroupNode ( undoClass *_undoObj );

int activeSymbolClass::addUndoRotateNode ( undoClass *_undoObj );

int activeSymbolClass::addUndoFlipNode ( undoClass *_undoObj );

int activeSymbolClass::undoCreate (
  undoOpClass *opPtr );

int activeSymbolClass::undoMove (
  undoOpClass *opPtr,
  int x,
  int y );

int activeSymbolClass::undoResize (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

int activeSymbolClass::undoCopy (
  undoOpClass *opPtr );

int activeSymbolClass::undoCut (
  undoOpClass *opPtr );

int activeSymbolClass::undoPaste (
  undoOpClass *opPtr );

int activeSymbolClass::undoReorder (
  undoOpClass *opPtr );

int activeSymbolClass::undoEdit (
  undoOpClass *opPtr );

int activeSymbolClass::undoGroup (
  undoOpClass *opPtr );

int activeSymbolClass::undoRotate (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

int activeSymbolClass::undoFlip (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

};

#endif
