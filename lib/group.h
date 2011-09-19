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

#ifndef __group_h
#define __group_h 1

#include "act_grf.h"
#include "undo.h"
#include "entry_form.h"

#include "pv_factory.h"
#include "cvtFast.h"

#include "group.str"

// the following defines btnActionListType & btnActionListPtr
#include "btnActionListType.h"

#define AGC_MAJOR_VERSION 4
#define AGC_MINOR_VERSION 0
#define AGC_RELEASE 0

#ifdef __group_cc

static void groupUnconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void agc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void agc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void agc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void agc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class activeGroupClass : public activeGraphicClass {

private:

friend void groupUnconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void agc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void agc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void agc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void agc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

 enum CrawlerStates { GETTING_FIRST_CRAWLER_PV, GETTING_NEXT_CRAWLER_PV,
  NO_MORE_CRAWLER_PVS };

void * voidHead; // cast to activeGraphicListPtr at runtime
void * curCrawlerNode; // cast to activeGraphicListPtr at runtime
int curCrawlerState;

void * sarNode; // cast to activeGraphicListPtr at runtime
int sarIndex, sarNeedNextNode, sarItemIndexOffset;

typedef struct relatedDisplayNodeList {
  void *ptr; // cast to activeGraphicListPtr at runtime
  int first;  // index of first related screen
  int last;   // index of last related screen
  struct relatedDisplayNodeList *flink;
  struct relatedDisplayNodeList *blink;
} RelatedDisplayNodeType, *RelatedDisplayNodePtr;
RelatedDisplayNodePtr relatedDisplayNodeHead;

btnActionListPtr btnDownActionHead;
btnActionListPtr btnUpActionHead;
btnActionListPtr btnMotionActionHead;
btnActionListPtr btnFocusActionHead;
int depth;
undoClass undoObj;

typedef struct editBufTag {
// edit buffer
  char bufVisPvName[PV_Factory::MAX_PV_NAME+1];
} editBufType, *editBufPtr;

editBufPtr eBuf;

entryListBase *invisPvEntry, *visInvEntry, *minVisEntry, *maxVisEntry;

int bufX, bufY;
ProcessVariable *visPvId;
expStringClass visPvExpStr;
pvValType minVis, maxVis;
char minVisString[39+1], bufMinVisString[39+1];
char maxVisString[39+1], bufMaxVisString[39+1];
int visInverted  , bufVisInverted;

int visPvExists;
int activeMode, init, opComplete, op2Complete;

int visibility, prevVisibility;
static const int visPvConnection = 1;
pvConnectionClass connection;

int needConnectInit, needVisUpdate, needRefresh, needToDrawUnconnected,
 needToEraseUnconnected;

XtIntervalId unconnectedTimer;

public:

static void visPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
);

static void visPvValueCallback (
  ProcessVariable *pv,
  void *userarg
);

activeGroupClass ( void );

activeGroupClass
 ( const activeGroupClass *source );

~activeGroupClass ( void );

int createGroup (
  activeWindowClass *aw_obj );

int ungroup (
  void *curListNode );
 
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

int edit ( void );

void beginEdit ( void );

int checkEditStatus ( void );

int draw ( void );

int erase ( void );

int drawActive ( void );

int eraseActive ( void );

activeGraphicClass *enclosingObject (
  int x0,
  int y0 );

int doSmartDrawAllActive ( void );

int drawActiveIfIntersects (
  int x0,
  int y0,
  int x1,
  int y1 );

int smartDrawCount ( void );

void resetSmartDrawCount ( void );

int activate (
  int pass,
  void *ptr,
  int *numSubObjects );

int deactivate (
  int pass,
  int *numSubObjects );

int preReactivate (
  int pass,
  int *numSubObjects );

int reactivate (
  int pass,
  void *ptr,
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

int move (
  int x,
  int y );

int moveAbs (
  int x,
  int y );

int moveMidpointAbs (
  int x,
  int y );

int rotate (
  int xOrigin,
  int yOrigin,
  char direction );

int flip (
  int xOrigin,
  int yOrigin,
  char direction );

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

int getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

void btnDown (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void btnUp (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void btnDrag (
  XMotionEvent *me,
  int x,
  int y,
  int buttonState,
  int buttonNumber );

void pointerIn (
  XMotionEvent *me,
  int x,
  int y,
  int buttonState );

void pointerOut (
  XMotionEvent *me,
  int x,
  int y,
  int buttonState );

void checkMouseOver (
  XMotionEvent *me,
  int x,
  int y,
  int buttonState );

activeGraphicClass *getTail ( void );

void updateGroup ( void );

int initDefExeNode (
  void *ptr );

void executeDeferred ( void );

int containsMacros ( void );

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

void bufInvalidate ( void );

void setNextSelectedToEdit (
  activeGraphicClass *ptr );

void clearNextSelectedToEdit ( void );

int activateBeforePreReexecuteComplete ( void );

int activateComplete ( void );

void changeDisplayParams (
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

int showPvInfo (
  XButtonEvent *be,
  int x,
  int y );

int startDrag (
  XButtonEvent *be,
  int x,
  int y
);

int selectDragValue (
  XButtonEvent *be
);

char *firstDragName (
  int x,
  int y
);

char *nextDragName (
  int x,
  int y
);

char *dragValue (
  int x,
  int y,
  int i
);

int atLeastOneDragPv (
  int x,
  int y
);

void initEnable ( void );

void enable ( void );

void disable ( void );

void map ( void );

void unmap ( void );

int getGroupVisInfo (
  expStringClass *visStr,
  int *visInv,
  int maxLen,
  char *minVis,
  char *maxVis
);

int putGroupVisInfo (
  expStringClass *visStr,
  int visInv,
  int maxLen,
  char *minVis,
  char *maxVis
);

char *crawlerGetFirstPv ( void );

char *crawlerGetNextPv ( void );

int isRelatedDisplay ( void );

int getNumRelatedDisplays ( void );

int getRelatedDisplayProperty (
  int index,
  char *name
);

char *getRelatedDisplayName (
  int index
);

char *getRelatedDisplayMacros (
  int index
);

char *getSearchString (
  int i
);

void replaceString (
  int i,
  int max,
  char *string
);

};

#endif
