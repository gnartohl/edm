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

#ifndef __expRect_h
#define __expRect_h 1

#include "act_grf.h"
#include "entry_form.h"
#include "rules.h"

#include "cadef.h"

#define ARC_K_COLORMODE_STATIC 0
#define ARC_K_COLORMODE_ALARM 1

#define ARC_MAJOR_VERSION 2
#define ARC_MINOR_VERSION 0
#define ARC_RELEASE 0

#define BG_RULE 1

#ifdef __expRect_cc

#include "expRect.str"

static char *dragName[] = {
  expRectClass_str1,
  expRectClass_str2,
  expRectClass_str3
};

static void bgRuleCb (
  void *ptr,
  int ruleId,
  int value );

static void arc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void arc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void arc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void arc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void arc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void aroMonitorAlarmPvConnectState (
  struct connection_handler_args arg );

static void rectangleAlarmUpdate (
  struct event_handler_args ast_args );

static void aroMonitorVisPvConnectState (
  struct connection_handler_args arg );

static void rectangleVisUpdate (
  struct event_handler_args ast_args );

#endif

class expRectClass : public activeGraphicClass {

private:

friend void bgRuleCb (
  void *ptr,
  int ruleId,
  int value );

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
  struct connection_handler_args arg );

friend void rectangleAlarmUpdate (
  struct event_handler_args ast_args );

friend void aroMonitorVisPvConnectState (
  struct connection_handler_args arg );

friend void rectangleVisUpdate (
  struct event_handler_args ast_args );

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

chid alarmPvId;
evid alarmEventId;
chid visPvId;
evid visEventId;

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

ruleClass *bgRule;

public:

expRectClass::expRectClass ( void );

// copy constructor
expRectClass::expRectClass
 ( const expRectClass *source );

expRectClass::~expRectClass ( void ) {

  if ( name ) delete name;
  delete bgRule; bgRule = NULL;

}

char *expRectClass::objName ( void ) {

  return name;

}

int expRectClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int expRectClass::save (
  FILE *f );

int expRectClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int expRectClass::importFromXchFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int expRectClass::genericEdit ( void );

int expRectClass::edit ( void );

int expRectClass::editCreate ( void );

int expRectClass::draw ( void );

int expRectClass::drawActive ( void );

int expRectClass::erase ( void );

int expRectClass::eraseActive ( void );

int expRectClass::eraseUnconditional ( void );

int expRectClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int expRectClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int expRectClass::containsMacros ( void );

int expRectClass::activate ( int pass, void *ptr );

int expRectClass::deactivate ( int pass );

int expRectClass::isInvisible ( void )
{
  return invisible;
}

void expRectClass::executeDeferred ( void );

char *expRectClass::firstDragName ( void );

char *expRectClass::nextDragName ( void );

char *expRectClass::dragValue (
  int i );

void expRectClass::changeDisplayParams (
  unsigned int flag,
  char *fontTag,
  int alignment,
  char *ctlFontTag,
  int ctlAlignment,
  char *btnFontTag,
  int btnAlignment,
  unsigned int textFgColor,
  unsigned int fg1Color,
  unsigned int fg2Color,
  unsigned int offsetColor,
  unsigned int bgColor,
  unsigned int topShadowColor,
  unsigned int botShadowColor );

void expRectClass::changePvNames (
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

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_expRectClassPtr ( void );
void *clone_expRectClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
