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

#include "pv_factory.h"
#include "cvtFast.h"

#define ASC_MAJOR_VERSION 4
#define ASC_MINOR_VERSION 0
#define ASC_RELEASE 0

#define SYMBOL_K_NUM_STATES 64
#define SYMBOL_K_MAX_PVS 5

#define OR_ORIG 0
#define OR_CW 1
#define OR_CCW 2
#define OR_V 3
#define OR_H 4

#ifdef __symbol_cc

#include "symbol.str"

static char *dragName[] = {
  activeSymbolClass_str2,
  activeSymbolClass_str3,
  activeSymbolClass_str4,
  activeSymbolClass_str5,
  activeSymbolClass_str6,
};

static void symUnconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void symbol_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void symbol_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void symbol_controlUpdate (
  ProcessVariable *pv,
  void *userarg );

static void symbol_colorUpdate (
  ProcessVariable *pv,
  void *userarg );

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
  unsigned int andMask;
  unsigned int xorMask;
  int shiftCount;
} objPlusIndexType, *objPlusIndexPtr;

class activeSymbolClass : public activeGraphicClass {

private:

friend class undoSymbolOpClass;

objPlusIndexType argRec[SYMBOL_K_MAX_PVS];

void *voidHead[SYMBOL_K_NUM_STATES]; // cast to activeGraphicListPtr
                                     // array at runtime

int numPvs;

ProcessVariable *controlPvId[SYMBOL_K_MAX_PVS];
ProcessVariable *colorPvId;
int initialCtrlConnection[SYMBOL_K_MAX_PVS], initialColorConnection;

unsigned int notControlPvConnected;
int init, active, activeMode, opComplete, controlExists, colorExists,
 colorPvConnected;

int iValue;
double controlVals[SYMBOL_K_MAX_PVS], controlV, curControlV, curColorV;
double stateMinValue[SYMBOL_K_NUM_STATES];
double stateMaxValue[SYMBOL_K_NUM_STATES];
char symbolFileName[127+1];
expStringClass controlPvExpStr[SYMBOL_K_MAX_PVS];
expStringClass colorPvExpStr;

btnActionListPtr btnDownActionHead;
btnActionListPtr btnUpActionHead;
btnActionListPtr btnMotionActionHead;

int useOriginalSize, useOriginalColors;
int numStates;
int index, prevIndex;

entryListBase *elsvMin;
entryListBase *elsvMax;
double *minPtr[SYMBOL_K_NUM_STATES], *maxPtr[SYMBOL_K_NUM_STATES];

typedef struct editBufTag {
// edit buffer
  int bufX;
  int bufY;
  int bufW;
  int bufH;
  int bufFgColor;
  int bufBgColor;
  int bufBinaryTruthTable;
  int bufOrientation;
  char bufXorMask[SYMBOL_K_MAX_PVS][9+1];
  char bufAndMask[SYMBOL_K_MAX_PVS][9+1];
  int bufShiftCount[SYMBOL_K_MAX_PVS];
  double bufStateMinValue[SYMBOL_K_NUM_STATES];
  double bufStateMaxValue[SYMBOL_K_NUM_STATES];
  char bufSymbolFileName[127+1];
  char bufControlPvName[SYMBOL_K_MAX_PVS][PV_Factory::MAX_PV_NAME+1];
  char bufColorPvName[PV_Factory::MAX_PV_NAME+1];
  int bufNumStates;
  int bufUseOriginalSize;
  int bufUseOriginalColors;
} editBufType, *editBufPtr;

editBufPtr eBuf;

entryListBase *cntlPvEntry[SYMBOL_K_MAX_PVS],
 *andMaskEntry[SYMBOL_K_MAX_PVS],
 *xorMaskEntry[SYMBOL_K_MAX_PVS],
 *shiftCountEntry[SYMBOL_K_MAX_PVS];

entryListBase *presColorEntry, *fgColorEntry, *bgColorEntry;

int fgColor, bgColor;
colorButtonClass fgCb, bgCb;

int binaryTruthTable;

entryListBase *pvNamesObj;

int needErase, needDraw, needConnectInit, needConnect[SYMBOL_K_MAX_PVS],
 needRefresh, needColorInit, needColorRefresh, needToDrawUnconnected,
 needToEraseUnconnected;
XtIntervalId unconnectedTimer;

int orientation, prevOr;

unsigned int xorMask[SYMBOL_K_MAX_PVS];
unsigned int andMask[SYMBOL_K_MAX_PVS];
char cXorMask[SYMBOL_K_MAX_PVS][9+1];
char cAndMask[SYMBOL_K_MAX_PVS][9+1];
int shiftCount[SYMBOL_K_MAX_PVS];

unsigned int curUiVal[SYMBOL_K_MAX_PVS];

public:

undoClass undoObj;

friend void symUnconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void symbol_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void symbol_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void symbol_controlUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void symbol_colorUpdate (
  ProcessVariable *pv,
  void *userarg );

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

activeSymbolClass ( void );

activeSymbolClass
 ( const activeSymbolClass *source );

~activeSymbolClass ( void );

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int readSymbolFile ( void );

//int old_readSymbolFile ( void );

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

void removePrevBlink ( void );

int activate (
  int pass,
  void *ptr,
  int *numSubObjects );

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

int resizeSelectBoxAbsFromUndo (
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

int resizeAbsFromUndo (
  int _x,
  int _y,
  int _w,
  int _h );

int getButtonActionRequest (
  int *up,
  int *down,
  int *drag );

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
  int *value );

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

int rotate (
  int xOrigin,
  int yOrigin,
  char direction ); // '+'=clockwise, '-'=counter clockwise

int flip (
  int xOrigin,
  int yOrigin,
  char direction );

int rotateInternal (
  int xOrigin,
  int yOrigin,
  char direction ); // '+'=clockwise, '-'=counter clockwise

int flipInternal (
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

void updateColors (
  double colorValue );

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
