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

#ifndef __rectangle_gen_h
#define __rectangle_gen_h 1

#include "act_grf.h"
#include "entry_form.h"

//#ifdef __epics__
//#include "cadef.h"
//#endif

#include "pv.h"

#define ARC_K_COLORMODE_STATIC 0
#define ARC_K_COLORMODE_ALARM 1

#define ARC_MAJOR_VERSION 1
#define ARC_MINOR_VERSION 4
#define ARC_RELEASE 0

class activeRectangleClass : public activeGraphicClass {

private:

friend void arc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void arc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void arc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void arc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void arc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void aroMonitorAlarmPvConnectState (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void rectangleAlarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void aroMonitorVisPvConnectState (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void rectangleVisUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

int bufX, bufY, bufW, bufH;

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

int invisible, bufInvisible;
int lineWidth, bufLineWidth;
int lineStyle, bufLineStyle;

int needVisConnectInit;
int needAlarmConnectInit;
int needDraw, needErase, needRefresh;

char pvClassName[15+1], pvUserClassName[15+1], pvOptionList[255+1];

int numPvTypes, pvNameIndex;

public:

activeRectangleClass ( void );

// copy constructor
activeRectangleClass
 ( const activeRectangleClass *source );

~activeRectangleClass ( void ) {

/*   printf( "In ~activeRectangleClass\n" ); */

  if ( name ) delete name;

}

char *objName ( void ) {

  return name;

}

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

int importFromXchFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int draw ( void );

int drawActive ( void );

int erase ( void );

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

int isInvisible ( void )
{
  return invisible;
}

void executeDeferred ( void );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeRectangleClassPtr ( void );
void *clone_activeRectangleClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
