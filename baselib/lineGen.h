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

#ifndef __line_gen_h
#define __line_gen_h 1

#include "act_grf.h"
#include "entry_form.h"

// #ifdef __epics__
// #include "cadef.h"
// #endif

#include "pv.h"

#define ALC_K_COLORMODE_STATIC 0
#define ALC_K_COLORMODE_ALARM 1

#define ALC_MAJOR_VERSION 1
#define ALC_MINOR_VERSION 3
#define ALC_RELEASE 0

#ifdef __line_gen_cc

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

static void aloMonitorAlarmPvConnectState (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void lineAlarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void aloMonitorVisPvConnectState (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void lineVisUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

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

friend void aloMonitorAlarmPvConnectState (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void lineAlarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void aloMonitorVisPvConnectState (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void lineVisUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

int wasSelected;

int bufX, bufY, bufW, bufH;
int oldX, oldY, oldW, oldH;

pointPtr head;
int numPoints;
XPoint *xpoints;

pvColorClass lineColor;
unsigned int bufLineColor;
colorButtonClass lineCb;

int lineColorMode;
int bufLineColorMode;

int fill;
int bufFill;

pvColorClass fillColor;
unsigned int bufFillColor;
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

// #ifdef __epics__
// chid alarmPvId;
// evid alarmEventId;
// chid visPvId;
// evid visEventId;
// #endif

pvClass *visPvId, *alarmPvId;
pvEventClass *alarmEventId, *visEventId;

expStringClass alarmPvExpStr;
char bufAlarmPvName[39+1];

expStringClass visPvExpStr;
char bufVisPvName[39+1];

int alarmPvExists, alarmPvConnected, visPvExists, visPvConnected;
int active, activeMode, init, opComplete;

int needVisConnectInit;
int needAlarmConnectInit;
int needDraw, needErase, needRefresh;

char pvClassName[15+1], pvUserClassName[15+1], pvOptionList[255+1];

int numPvTypes, pvNameIndex;

public:

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

int save (
  FILE *f );

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int editLineSegments ( void );

virtual int addPoint (
  int x,
  int y );

virtual int removeLastPoint ( void );

virtual pointPtr selectPoint (
  int x,
  int y );

virtual int movePoint (
  pointPtr curPoint,
  int x,
  int y );

int select (
  int x,
  int y );

int lineEditDone ( void );

int lineEditComplete ( void );

int lineEditCancel ( void );

int draw ( void );

int erase ( void );

int drawActive ( void );

int eraseActive ( void );

int eraseUnconditional ( void );

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

int isMultiPointObject ( void );

void executeDeferred ( void );

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
