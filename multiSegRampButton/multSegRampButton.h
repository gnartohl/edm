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

#ifndef __multSegRampButton_h
#define __multSegRampButton_h 1

#include "act_grf.h"
#include "entry_form.h"
#include "keypad.h"
#include "pv_factory.h"
#include "cvtFast.h"
#include "sys_types.h"
#include "thread.h"

#define MSRBTC_MAJOR_VERSION 4
#define MSRBTC_MINOR_VERSION 0
#define MSRBTC_RELEASE 0

#define MSRBTC_IDLE 1
#define MSRBTC_RAMPING 2
#define MSRBTC_PAUSED 3

#define MAXSEGS 5

#ifdef __multSegRampButton_cc

#include "multSegRampButton.str"

static void doBlink (
  void *ptr
);

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void msrbtc_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void msrbtc_visUpdate (
  ProcessVariable *pv,
  void *userarg );

static void msrbtc_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void msrbtc_colorUpdate (
  ProcessVariable *pv,
  void *userarg );

static void msrbtc_controlUpdate (
  ProcessVariable *pv,
  void *userarg );

static void msrbtc_finalUpdate (
  ProcessVariable *pv,
  void *userarg );

static void msrbtc_rampRateUpdate (
  ProcessVariable *pv,
  void *userarg );

static void msrbtc_decrement (
  XtPointer client,
  XtIntervalId *id );

static void msrbtc_increment (
  XtPointer client,
  XtIntervalId *id );

static char *dragName[] = {
  activeMultSegRampButtonClass_str8,
  activeMultSegRampButtonClass_str34,
  activeMultSegRampButtonClass_str10,
  activeMultSegRampButtonClass_str9,
  activeMultSegRampButtonClass_str35,
  activeMultSegRampButtonClass_str36,
  activeMultSegRampButtonClass_str37,
  activeMultSegRampButtonClass_str38
};

static void msrbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void msrbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void msrbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void msrbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void msrbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void msrbtc_monitor_dest_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void msrbtc_monitor_final_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void msrbtc_monitor_rampRate_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void msrbtc_monitor_rampState_connect_state (
  ProcessVariable *pv,
  void *userarg );

#endif

class activeMultSegRampButtonClass : public activeGraphicClass {

private:

friend void doBlink (
  void *ptr
);

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void msrbtc_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void msrbtc_visUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void msrbtc_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void msrbtc_colorUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void msrbtc_controlUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void msrbtc_finalUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void msrbtc_rampRateUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void msrbtc_decrement (
  XtPointer client,
  XtIntervalId *id );

friend void msrbtc_increment (
  XtPointer client,
  XtIntervalId *id );

friend void msrbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void msrbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void msrbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void msrbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void msrbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void msrbtc_monitor_dest_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void msrbtc_monitor_final_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void msrbtc_monitor_rampRate_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void msrbtc_monitor_rampState_connect_state (
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
  int bufLimitsFromDb;
  efDouble bufEfScaleMin;
  efDouble bufEfScaleMax;
  efDouble bufEfRateMax;
  int bufVisInverted;
  colorButtonClass fgCb;
  colorButtonClass bgCb;
  colorButtonClass topShadowCb;
  colorButtonClass botShadowCb;
  char bufLabel[39+1];
  char bufDestPvName[PV_Factory::MAX_PV_NAME+1];
  char bufFinalPvName[MAXSEGS][PV_Factory::MAX_PV_NAME+1];
  char bufRampRatePvName[MAXSEGS][PV_Factory::MAX_PV_NAME+1];
  char bufRampStatePvName[PV_Factory::MAX_PV_NAME+1];
  char bufVisPvName[PV_Factory::MAX_PV_NAME+1];
  char bufMinVisString[39+1];
  char bufMaxVisString[39+1];
  char bufColorPvName[PV_Factory::MAX_PV_NAME+1];
} editBufType, *editBufPtr;

editBufPtr eBuf;

entryListBase *finalEntry[MAXSEGS], *rateEntry[MAXSEGS];

entryListBase *limitsFromDbEntry, *minEntry, *maxEntry;

entryListBase *invisPvEntry, *visInvEntry, *minVisEntry, *maxVisEntry;

int destType, finalType[MAXSEGS], rampRateType[MAXSEGS], rampStateType;

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
static const int rampStatePvConnection = 2;
static const int visPvConnection = 3;
static const int colorPvConnection = 4;
int finalPvConnection[MAXSEGS]; // = { 5, 6, 7, 8, 9 };
int rampRatePvConnection[MAXSEGS]; // = { 10, 11, 12, 13, 14 };

pvConnectionClass connection;

ProcessVariable *destPvId, *finalPvId[MAXSEGS], *rampRatePvId[MAXSEGS], *rampStatePvId;

expStringClass destPvExpString;

expStringClass finalPvExpString[MAXSEGS];

expStringClass rampRatePvExpString[MAXSEGS];

expStringClass rampStatePvExpString;

int rampSegment;

double rampRate[MAXSEGS], updateRate, increment[MAXSEGS];

int destExists, finalExists[MAXSEGS], rampRateExists[MAXSEGS], rampStateExists, buttonPressed;

int destPvConnected, finalPvConnected[MAXSEGS], rampRatePvConnected[MAXSEGS],
 rampStatePvConnected, active, activeMode, init, segExists[MAXSEGS];

int incrementTimerActive, incrementTimerValue;
XtIntervalId incrementTimer;
double controlV, curControlV, curFinalV[MAXSEGS], rampFinalV[MAXSEGS], curRampRateV[MAXSEGS];

int needConnectInit, needFinalConnectInit[MAXSEGS], needRampRateConnectInit[MAXSEGS],
 needRampStateConnectInit, needCtlInfoInit, needRefresh, needErase, needDraw,
 needToDrawUnconnected, needToEraseUnconnected;
XtIntervalId unconnectedTimer;
int initialConnection, initialFinalValueConnection[MAXSEGS],
 initialRampRateConnection[MAXSEGS],
 initialRampStateValueConnection, initialVisConnection,
 initialColorConnection;

int limitsFromDb;
double scaleMin, scaleMax, minDv, maxDv, rateMax;
efDouble efScaleMin, efScaleMax, efRateMax;

ProcessVariable *visPvId;
expStringClass visPvExpString;
int visExists;
double visValue, curVisValue, minVis, maxVis;
char minVisString[39+1];
char maxVisString[39+1];
int prevVisibility, visibility, visInverted;
int needVisConnectInit, needVisInit, needVisUpdate;

int monotonic;

ProcessVariable *colorPvId;
expStringClass colorPvExpString;
int colorExists;
double colorValue, curColorValue;
int needColorConnectInit, needColorInit, needColorUpdate;

int rootX, rootY;

int state;

struct timeval baseTime;

public:

activeMultSegRampButtonClass ( void );

activeMultSegRampButtonClass
 ( const activeMultSegRampButtonClass *source );

~activeMultSegRampButtonClass ( void );

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

void *create_activeMultSegRampButtonClassPtr ( void );
void *clone_activeMultSegRampButtonClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
