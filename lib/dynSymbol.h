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
#include "undo.h"
#include "entry_form.h"

// the following defines btnActionListType & btnActionListPtr
#include "btnActionListType.h"

#include "pv_factory.h"
#include "cvtFast.h"

#define DSC_MAJOR_VERSION 4
#define DSC_MINOR_VERSION 0
#define DSC_RELEASE 1

#define DYNSYMBOL_K_NUM_STATES 64

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
  ProcessVariable *pv,
  void *userarg );

static void dynSymbol_colorUpdate (
  ProcessVariable *pv,
  void *userarg );

static void dynSymbol_monitor_gateUp_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void dynSymbol_gateUpUpdate (
  ProcessVariable *pv,
  void *userarg );

static void dynSymbol_monitor_gateDown_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void dynSymbol_gateDownUpdate (
  ProcessVariable *pv,
  void *userarg );

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

friend class undoDynSymbolOpClass;

dsObjPlusIndexType argRec;

void *voidHead[DYNSYMBOL_K_NUM_STATES]; // cast to activeGraphicListPtr
                                     // array at runtime

ProcessVariable *gateUpPvId, *gateDownPvId, *colorPvId;
int initialGateUpConnection, initialGateDownConnection,
 initialColorConnection;

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

typedef struct editBufTag {
// edit buffer
  char bufControlPvName[PV_Factory::MAX_PV_NAME+1];
  char bufGateDownPvName[PV_Factory::MAX_PV_NAME+1];
  char bufGateUpPvName[PV_Factory::MAX_PV_NAME+1];
  char bufColorPvName[PV_Factory::MAX_PV_NAME+1];
} editBufType, *editBufPtr;

editBufPtr eBuf;

entryListBase *useGateEntry, *gateOnMouseEntry, *gateUpPvEntry, *gateDnPvEntry, 
*gateUpValEntry, *gateDnValEntry;

entryListBase *contEntry, *rateEntry;

entryListBase *presColorEntry, *fgColorEntry, *bgColorEntry;

int bufX;
int bufY;
int bufW;
int bufH;

double bufStateMinValue[DYNSYMBOL_K_NUM_STATES];
double bufStateMaxValue[DYNSYMBOL_K_NUM_STATES];
char bufDynSymbolFileName[127+1];
int bufNumStates, bufUseOriginalSize, bufUseOriginalColors, bufUseGate,
 bufContinuous;
double bufRate;
int bufGateDownValue, bufGateUpValue;
int initialIndex, bufInitialIndex;

int fgColor, bgColor, bufFgColor, bufBgColor;
colorButtonClass fgCb, bgCb;

entryListBase *pvNamesObj;

int needErase, needDraw, needRefresh;
int needGateUp, needGateUpConnect;
int needGateDown, needGateDownConnect;
int needColorInit, needColorRefresh;

int showOOBState, bufShowOOBState;

int gateOnMouseOver, bufGateOnMouseOver;

public:

undoClass undoObj;

friend void dsc_updateControl (
  XtPointer client,
  XtIntervalId *id );

friend void dynSymbol_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void dynSymbol_colorUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void dynSymbol_monitor_gateUp_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void dynSymbol_gateUpUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void dynSymbol_monitor_gateDown_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void dynSymbol_gateDownUpdate (
  ProcessVariable *pv,
  void *userarg );

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

activeDynSymbolClass ( void );

activeDynSymbolClass
 ( const activeDynSymbolClass *source );

~activeDynSymbolClass ( void );

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int readDynSymbolFile ( void );

//int old_readDynSymbolFile ( void );

int createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int save (
  FILE *f );

int old_save (
  FILE *f );

int createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int old_createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int draw ( void );

int erase ( void );

int drawActive ( void );

int eraseActive ( void );

int activate (
  int pass,
  void *ptr,
  int *numSubObjects );

void removePrevBlink ( void );

int deactivate (
  int pass,
  int *numSubObjects );

int moveSelectBox (
  int _x,
  int _y );

int moveSelectBoxAbs (
  int _x,
  int _y );

int moveSelectBoxMidpointAbs (
  int _x,
  int _y );

int checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int resizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

int resizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

int move (
  int x,
  int y );

int moveAbs (
  int x,
  int y );

int moveMidpointAbs (
  int x,
  int y );

int resize (
  int _x,
  int _y,
  int _w,
  int _h );

int resizeAbs (
  int _x,
  int _y,
  int _w,
  int _h );

void mousePointerIn (
  int _x,
  int _y,
  int buttonState );

void mousePointerOut (
  int x,
  int y,
  int buttonState );

void pointerIn (
  int _x,
  int _y,
  int buttonState );

void pointerOut (
  int x,
  int y,
  int buttonState );

int getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

void btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

void btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

void updateGroup ( void );

int expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] );

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

int setProperty (
  char *prop,
  char *value );

char *firstDragName ( void );

char *nextDragName ( void );

char *dragValue (
  int i );

void changePvNames (
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

void updateColors (
  double colorValue );

int rotate (
  int xOrigin,
  int yOrigin,
  char direction ); // '+'=clockwise, '-'=counter clockwise

int flip (
  int xOrigin,
  int yOrigin,
  char direction );

void flushUndo ( void );

int addUndoCreateNode ( undoClass *_undoObj );

int addUndoMoveNode ( undoClass *_undoObj );

int addUndoResizeNode ( undoClass *_undoObj );

int addUndoCopyNode ( undoClass *_undoObj );

int addUndoCutNode ( undoClass *_undoObj );

int addUndoPasteNode ( undoClass *_undoObj );

int addUndoReorderNode ( undoClass *_undoObj );

int addUndoEditNode ( undoClass *_undoObj );

int addUndoGroupNode ( undoClass *_undoObj );

int addUndoRotateNode ( undoClass *_undoObj );

int addUndoFlipNode ( undoClass *_undoObj );

int undoCreate (
  undoOpClass *opPtr );

int undoMove (
  undoOpClass *opPtr,
  int x,
  int y );

int undoResize (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

int undoCopy (
  undoOpClass *opPtr );

int undoCut (
  undoOpClass *opPtr );

int undoPaste (
  undoOpClass *opPtr );

int undoReorder (
  undoOpClass *opPtr );

int undoEdit (
  undoOpClass *opPtr );

int undoGroup (
  undoOpClass *opPtr );

int undoRotate (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

int undoFlip (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

void getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n );

char *getSearchString (
  int i
);

void replaceString (
  int i,
  int max,
  char *string
);

char *crawlerGetFirstPv ( void );

char *crawlerGetNextPv ( void );

};

#endif
