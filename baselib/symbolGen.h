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

#ifndef __symbolGen_h
#define __symbolGen_h 1

#include "act_grf.h"
#include "entry_form.h"

// the following defines btnActionListType & btnActionListPtr
#include "btnActionListType.h"

// #ifdef __epics__
// #include "cadef.h"
// #endif

#include "pv.h"

#define ASC_MAJOR_VERSION 1
#define ASC_MINOR_VERSION 4
#define ASC_RELEASE 0

#define SYMBOL_K_NUM_STATES 32
#define SYMBOL_K_MAX_PVS 5

#ifdef __symbolGen_cc

static void symbol_monitor_control_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void symbol_controlUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

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

// #ifdef __epics__
// chid controlPvId[SYMBOL_K_MAX_PVS];
// evid controlEventId[SYMBOL_K_MAX_PVS];
// #endif

pvClass *controlPvId[SYMBOL_K_MAX_PVS];
pvEventClass *controlEventId[SYMBOL_K_MAX_PVS];

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

char pvClassName[15+1], pvUserClassName[15+1], pvOptionList[255+1];

int numPvTypes, pvNameIndex;

public:

friend void symbol_monitor_control_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void symbol_controlUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

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

int createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int save (
  FILE *f );

int createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int draw ( void );

int erase ( void );

int drawActive ( void );

int eraseActive ( void );

int activate (
  int pass,
  void *ptr );

int deactivate ( int pass );

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

};

#endif
