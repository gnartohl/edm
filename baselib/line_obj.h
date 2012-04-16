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
#include "cvtFast.h"

#define ALC_K_COLORMODE_STATIC 0
#define ALC_K_COLORMODE_ALARM 1

#define ALC_MAJOR_VERSION 4
#define ALC_MINOR_VERSION 0
#define ALC_RELEASE 1

#ifdef __line_obj_cc

#include "line_obj.str"

static char *dragName[] = {
  activeLineClass_str2,
  activeLineClass_str3
};

static void doBlink (
  void *ptr
);

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

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

friend void doBlink (
  void *ptr
);

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

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

typedef struct editBufTag {
// edit buffer
  int bufX;
  int bufY;
  int bufW;
  int bufH;
  int bufLineColor;
  colorButtonClass lineCb;
  int bufLineColorMode;
  int bufFill;
  int bufFillColor;
  colorButtonClass fillCb;
  int bufFillColorMode;
  int bufCapStyle;
  int bufJoinStyle;
  int bufLineStyle;
  int bufLineWidth;
  char bufMinVisString[39+1];
  char bufMaxVisString[39+1];
  int bufVisInverted;
  char bufAlarmPvName[PV_Factory::MAX_PV_NAME+1];
  char bufVisPvName[PV_Factory::MAX_PV_NAME+1];
  int bufClosePolygon;
  int bufArrows;
} editBufType, *editBufPtr;

editBufPtr eBuf;

entryListBase *invisPvEntry, *visInvEntry, *minVisEntry, *maxVisEntry;

entryListBase *fillEntry, *fillColorEntry, *fillAlarmSensEntry;

int oldX, oldY, oldW, oldH;

pointPtr head;
int numPoints;
XPoint *xpoints;

pvColorClass lineColor;

int lineColorMode;

int fill;

pvColorClass fillColor;

int fillColorMode;

int capStyle, joinStyle, lineStyle, lineWidth;

int pvType;
pvValType pvValue, minVis, maxVis;
char minVisString[39+1];
char maxVisString[39+1];

int prevVisibility, visibility, visInverted;
int lineVisibility, prevLineVisibility;
int fillVisibility, prevFillVisibility;

ProcessVariable *alarmPvId;
ProcessVariable *visPvId;

expStringClass alarmPvExpStr;

expStringClass visPvExpStr;

int alarmPvExists, visPvExists;
int activeMode, init, opComplete;

int needConnectInit, needAlarmUpdate, needVisUpdate, needRefresh;
int needToDrawUnconnected, needToEraseUnconnected;
XtIntervalId unconnectedTimer;

int curLineColorIndex, curFillColorIndex, curStatus, curSeverity;
static const int alarmPvConnection = 1;
static const int visPvConnection = 2;
pvConnectionClass connection;

int closePolygon;
int arrows;

static const int ARROW_NONE = 0;
static const int ARROW_FROM = 1;
static const int ARROW_TO   = 2;
static const int ARROW_BOTH = 3;

public:

static void alarmPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
);

static void alarmPvValueCallback (
  ProcessVariable *pv,
  void *userarg
);

static void visPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
);

static void visPvValueCallback (
  ProcessVariable *pv,
  void *userarg
);

activeLineClass ( void );

~activeLineClass ( void );

// copy constructor
activeLineClass
( const activeLineClass *source );

char *objName ( void ) {

  return name;

}

int createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int save (
  FILE *f );

int old_save (
  FILE *f );

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int editLineSegments ( void );

virtual int insertPoint (
  int x,
  int y );

virtual int removePoint (
  int x,
  int y );

virtual int addPoint (
  int x,
  int y );

virtual int removeLastPoint ( void );

virtual pointPtr selectPoint (
  int x,
  int y );

virtual void deselectAllPoints ( void );

virtual int movePoint (
  pointPtr curPoint,
  int x,
  int y );

virtual int movePointRel (
  pointPtr curPoint,
  int xofs,
  int yofs );

int select (
  int x,
  int y );

int lineEditDone ( void );

int lineEditComplete ( void );

int lineEditCancel ( void );

int draw ( void );

int erase ( void );

int drawActiveIfIntersects (
  int x0,
  int y0,
  int x1,
  int y1 );

int drawActive ( void );

int eraseActive ( void );

int eraseUnconditional ( void );

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

int activate ( int pass, void *ptr );

int deactivate ( int pass );

void updateDimensions ( void );

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

int rotate (
  int xOrigin,
  int yOrigin,
  char direction );

int flip (
  int xOrigin,
  int yOrigin,
  char direction );

int isMultiPointObject ( void );

void executeDeferred ( void );

char *firstDragName ( void );

char *nextDragName ( void );

char *dragValue (
  int i );

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

void updateColors (
  double colorValue );

int addUndoEditNode ( undoClass *undoObj );

int addUndoRotateNode ( undoClass *undoObj );

int addUndoFlipNode ( undoClass *undoObj );

int undoEdit (
  undoOpClass *_opPtr );

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

void getArrowCoords (
  int arrows,
  XPoint *points
);

int ctlBoxLen ( void );

void getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n );

char *getSearchString (
  int index );

void replaceString (
  int i,
  int max,
  char *string
);

char *crawlerGetFirstPv ( void );

char *crawlerGetNextPv ( void );

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
