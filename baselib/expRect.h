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

static void bgRuleConnectCb (
  void *ptr,
  int ruleId,
  int op
);

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

friend void bgRuleConnectCb (
  void *ptr,
  int ruleId,
  int op
);

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
int bufLineColor;
colorButtonClass lineCb;

int lineColorMode;
int bufLineColorMode;

int fill;
int bufFill;

pvColorClass fillColor;
int bufFillColor;
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
int needRuleInit, firstRuleInit, needColorUpdate;

ruleClass *bgRule;
int bgRuleColor;
int useBgRule;

public:

expRectClass ( void );

// copy constructor
expRectClass
 ( const expRectClass *source );

~expRectClass ( void ) {

  if ( name ) delete name;
  delete bgRule; bgRule = NULL;

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
