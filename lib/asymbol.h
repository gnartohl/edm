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

#ifndef __asymbol_h
#define __asymbol_h 1

#include "act_grf.h"
#include "undo.h"
#include "entry_form.h"

// the following defines btnActionListType & btnActionListPtr
#include "btnActionListType.h"

#include "pv_factory.h"
#include "cvtFast.h"

#define ANSC_MAJOR_VERSION 4
#define ANSC_MINOR_VERSION 0
#define ANSC_RELEASE 0

#define ASYMBOL_K_NUM_STATES 64
#define ASYMBOL_K_MAX_PVS 8

#define OR_ORIG 0
#define OR_CW 1
#define OR_CCW 2
#define OR_V 3
#define OR_H 4

#ifdef __asymbol_cc

#include "asymbol.str"

static char *dragName[] = {
  aniSymbolClass_str2,
  aniSymbolClass_str3,
  aniSymbolClass_str4,
  aniSymbolClass_str5,
  aniSymbolClass_str6,
};

static void asymUnconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void asymbol_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void asymbol_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void asymbol_monitor_x_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void asymbol_monitor_y_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void asymbol_monitor_angle_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void asymbol_controlUpdate (
  ProcessVariable *pv,
  void *userarg );

static void asymbol_colorUpdate (
  ProcessVariable *pv,
  void *userarg );

static void asymbol_xUpdate (
  ProcessVariable *pv,
  void *userarg );

static void asymbol_yUpdate (
  ProcessVariable *pv,
  void *userarg );

static void asymbol_angleUpdate (
  ProcessVariable *pv,
  void *userarg );

static void asymbolSetItem (
  Widget w,
  XtPointer client,
  XtPointer call );

static void ansc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void ansc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void ansc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void ansc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void ansc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

typedef struct objPlusIndexTag1 {
  void *objPtr;
  int index;
  unsigned int setMask;
  unsigned int clrMask;
  unsigned int andMask;
  unsigned int xorMask;
  int shiftCount;
} objPlusIndexType1, *objPlusIndexPtr1;

class aniSymbolClass : public activeGraphicClass {

private:

friend class undoAniSymbolOpClass;

objPlusIndexType1 argRec[ASYMBOL_K_MAX_PVS];

void *voidHead[ASYMBOL_K_NUM_STATES]; // cast to activeGraphicListPtr
                                     // array at runtime

int numPvs;

ProcessVariable *controlPvId[ASYMBOL_K_MAX_PVS];
ProcessVariable *colorPvId, *xPvId, *yPvId, *anglePvId;
int initialCtrlConnection[ASYMBOL_K_MAX_PVS], initialColorConnection,
 initialXPvConnection, initialYPvConnection, initialAnglePvConnection;

unsigned int notControlPvConnected;
int init, active, activeMode, opComplete, controlExists, colorExists,
 colorPvConnected, xPvExists, xPvConnected, yPvExists, yPvConnected,
 anglePvExists, anglePvConnected;

int iValue, savedX, savedY;
double axisAngle, rAxisAngle;
double controlVals[ASYMBOL_K_MAX_PVS], controlV, curControlV, curColorV,
 curXV, curYV, curAngleV;
double stateMinValue[ASYMBOL_K_NUM_STATES];
double stateMaxValue[ASYMBOL_K_NUM_STATES];
char symbolFileName[127+1];
expStringClass controlPvExpStr[ASYMBOL_K_MAX_PVS];
expStringClass colorPvExpStr, xPvExpStr, yPvExpStr, anglePvExpStr;

btnActionListPtr btnDownActionHead;
btnActionListPtr btnUpActionHead;
btnActionListPtr btnMotionActionHead;

int useOriginalSize, useOriginalColors;
int numStates;
int index, prevIndex;

entryListBase *elsvMin;
entryListBase *elsvMax;
double *minPtr[ASYMBOL_K_NUM_STATES], *maxPtr[ASYMBOL_K_NUM_STATES];

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
  char bufXorMask[ASYMBOL_K_MAX_PVS][9+1];
  char bufAndMask[ASYMBOL_K_MAX_PVS][9+1];
  int bufShiftCount[ASYMBOL_K_MAX_PVS];
  double bufStateMinValue[ASYMBOL_K_NUM_STATES];
  double bufStateMaxValue[ASYMBOL_K_NUM_STATES];
  char bufSymbolFileName[127+1];
  char bufControlPvName[ASYMBOL_K_MAX_PVS][PV_Factory::MAX_PV_NAME+1];
  char bufColorPvName[PV_Factory::MAX_PV_NAME+1];
  char bufXPvName[PV_Factory::MAX_PV_NAME+1];
  char bufYPvName[PV_Factory::MAX_PV_NAME+1];
  char bufAnglePvName[PV_Factory::MAX_PV_NAME+1];
  int bufNumStates;
  int bufUseOriginalSize;
  int bufUseOriginalColors;
} editBufType, *editBufPtr;

editBufPtr eBuf;

entryListBase *cntlPvEntry[ASYMBOL_K_MAX_PVS],
 *andMaskEntry[ASYMBOL_K_MAX_PVS],
 *xorMaskEntry[ASYMBOL_K_MAX_PVS],
 *shiftCountEntry[ASYMBOL_K_MAX_PVS];

entryListBase *presColorEntry, *fgColorEntry, *bgColorEntry;

int fgColor, bgColor;
colorButtonClass fgCb, bgCb;

int binaryTruthTable;

entryListBase *pvNamesObj;

int needErase, needDraw, needConnectInit, needConnect[ASYMBOL_K_MAX_PVS],
 needRefresh, needColorInit, needXInit, needYInit, needColorRefresh,
 needPosRefresh, needToDrawUnconnected, needToEraseUnconnected,
 needAngleInit;
XtIntervalId unconnectedTimer;

int orientation, prevOr;

unsigned int xorMask[ASYMBOL_K_MAX_PVS];
unsigned int andMask[ASYMBOL_K_MAX_PVS];
char cXorMask[ASYMBOL_K_MAX_PVS][9+1];
char cAndMask[ASYMBOL_K_MAX_PVS][9+1];
int shiftCount[ASYMBOL_K_MAX_PVS];

unsigned int curUiVal[ASYMBOL_K_MAX_PVS];

public:

undoClass undoObj;

friend void asymUnconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void asymbol_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void asymbol_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void asymbol_monitor_x_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void asymbol_monitor_angle_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void asymbol_monitor_y_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void asymbol_controlUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void asymbol_colorUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void asymbol_xUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void asymbol_yUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void asymbol_angleUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void asymbolSetItem (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void ansc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void ansc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void ansc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void ansc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void ansc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

aniSymbolClass ( void );

aniSymbolClass
 ( const aniSymbolClass *source );

~aniSymbolClass ( void );

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

void updateStates( int index );

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
