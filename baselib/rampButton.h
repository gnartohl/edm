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

#ifndef __rampButton_h
#define __rampButton_h 1

#include "act_grf.h"
#include "entry_form.h"
#include "keypad.h"
#include "pv_factory.h"
#include "cvtFast.h"
#include "sys_types.h"
#include "thread.h"

#define RBTC_MAJOR_VERSION 4
#define RBTC_MINOR_VERSION 0
#define RBTC_RELEASE 0

#define RBTC_IDLE 1
#define RBTC_RAMPING 2
#define RBTC_PAUSED 3

#ifdef __rampButton_cc

#include "rampButton.str"

static void rbtc_doBlink (
  void *ptr
);

static void rbtc_unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void rbtc_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void rbtc_visUpdate (
  ProcessVariable *pv,
  void *userarg );

static void rbtc_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void rbtc_colorUpdate (
  ProcessVariable *pv,
  void *userarg );

static void rbtc_controlUpdate (
  ProcessVariable *pv,
  void *userarg );

static void rbtc_finalUpdate (
  ProcessVariable *pv,
  void *userarg );

static void rbtc_decrement (
  XtPointer client,
  XtIntervalId *id );

static void rbtc_increment (
  XtPointer client,
  XtIntervalId *id );

static char *dragName[] = {
  activeRampButtonClass_str8,
  activeRampButtonClass_str9,
  activeRampButtonClass_str34,
  activeRampButtonClass_str10
};

static void rbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void rbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void rbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void rbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void rbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void rbtc_monitor_dest_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void rbtc_monitor_final_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void rbtc_monitor_rampState_connect_state (
  ProcessVariable *pv,
  void *userarg );

#endif

class activeRampButtonClass : public activeGraphicClass {

private:

friend void rbtc_doBlink (
  void *ptr
);

friend void rbtc_unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void rbtc_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void rbtc_visUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void rbtc_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void rbtc_colorUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void rbtc_controlUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void rbtc_finalUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void rbtc_decrement (
  XtPointer client,
  XtIntervalId *id );

friend void rbtc_increment (
  XtPointer client,
  XtIntervalId *id );

friend void rbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rbtc_monitor_dest_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void rbtc_monitor_final_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void rbtc_monitor_rampState_connect_state (
  ProcessVariable *pv,
  void *userarg );

int opComplete;

int minW;
int minH;

typedef struct editBufTag {
// edit buffer
  int bufX;
  int bufY;
  int bufW;
  int bufH;
  int bufFgColor;
  int bufBgColor;
  int bufTopShadowColor;
  int bufBotShadowColor;
  int buf3D;
  int bufInvisible;
  double bufUpdateRate;
  double bufRampRate;
  int bufLimitsFromDb;
  efDouble bufEfScaleMin;
  efDouble bufEfScaleMax;
  int bufVisInverted;
  colorButtonClass fgCb;
  colorButtonClass bgCb;
  colorButtonClass topShadowCb;
  colorButtonClass botShadowCb;
  char bufLabel[39+1];
  char bufDestPvName[PV_Factory::MAX_PV_NAME+1];
  char bufFinalPvName[PV_Factory::MAX_PV_NAME+1];
  char bufRampStatePvName[PV_Factory::MAX_PV_NAME+1];
  char bufVisPvName[PV_Factory::MAX_PV_NAME+1];
  char bufMinVisString[39+1];
  char bufMaxVisString[39+1];
  char bufColorPvName[PV_Factory::MAX_PV_NAME+1];
} editBufType, *editBufPtr;

editBufPtr eBuf;

entryListBase *limitsFromDbEntry, *minEntry, *maxEntry;

entryListBase *invisPvEntry, *visInvEntry, *minVisEntry, *maxVisEntry;

int destType, finalType, rampStateType;

pvColorClass fgColor, bgColor;
int topShadowColor;
int botShadowColor;
expStringClass label;
int _3D, invisible;

fontMenuClass fm;
char fontTag[63+1];
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

static const int destPvConnection = 1;
static const int finalPvConnection = 2;
static const int visPvConnection = 3;
static const int colorPvConnection = 4;
static const int rampStatePvConnection = 5;

pvConnectionClass connection;

ProcessVariable *destPvId, *finalPvId, *rampStatePvId;

expStringClass destPvExpString;

expStringClass finalPvExpString;

expStringClass rampStatePvExpString;

double rampRate, updateRate, increment;

int destExists, finalExists, rampStateExists, buttonPressed;

int destPvConnected, finalPvConnected, rampStatePvConnected,
 active, activeMode, init;

int incrementTimerActive, incrementTimerValue;
XtIntervalId incrementTimer;
double controlV, curControlV, curFinalV, rampFinalV;

int needConnectInit, needFinalConnectInit, needRampStateConnectInit,
 needCtlInfoInit, needRefresh, needErase, needDraw, needToDrawUnconnected,
 needToEraseUnconnected;
XtIntervalId unconnectedTimer;
int initialConnection, initialFinalValueConnection,
 initialRampStateValueConnection, initialVisConnection,
 initialColorConnection;

int limitsFromDb;
double scaleMin, scaleMax, minDv, maxDv;
efDouble efScaleMin, efScaleMax;

ProcessVariable *visPvId;
expStringClass visPvExpString;
int visExists;
double visValue, curVisValue, minVis, maxVis;
char minVisString[39+1];
char maxVisString[39+1];
int prevVisibility, visibility, visInverted;
int needVisConnectInit, needVisInit, needVisUpdate;

ProcessVariable *colorPvId;
expStringClass colorPvExpString;
int colorExists;
double colorValue, curColorValue;
int needColorConnectInit, needColorInit, needColorUpdate;

int rootX, rootY;

int state;

struct timeval baseTime;

public:

activeRampButtonClass ( void );

activeRampButtonClass
 ( const activeRampButtonClass *source );

~activeRampButtonClass ( void );

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

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int draw ( void );

int erase ( void );

int drawActive ( void );

int eraseActive ( void );

int activate ( int pass, void *ptr );

int deactivate ( int pass );

void updateDimensions ( void );

void btnUp (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void btnDown (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void pointerIn (
  int _x,
  int _y,
  int buttonState );

int getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

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

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeRampButtonClassPtr ( void );
void *clone_activeRampButtonClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
