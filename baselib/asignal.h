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

#ifndef __asignal_h
#define __asignal_h 1

#include "act_grf.h"
#include "entry_form.h"
#include "keypad.h"
#include "pv_factory.h"
#include "pv_callback.h"
#include "sys_types.h"
#include "thread.h"

#define SIGC_MAJOR_VERSION 4
#define SIGC_MINOR_VERSION 0
#define SIGC_RELEASE 0

#define SIGC_IDLE 1
#define SIGC_GENERATING 2
#define SIGC_PAUSED 3

#define SIGC_K_SINE 0
#define SIGC_K_SQUARE 1
#define SIGC_K_TRIANGLE 2
#define SIGC_K_SAWTOOTH 3
#define SIGC_K_IMPULSE 4

#ifdef __asignal_cc

#include "asignal.str"

static void sigc_doBlink (
  void *ptr
);

static void sigc_unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void sigc_monitor_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void sigc_update (
  ProcessVariable *pv,
  void *userarg );

static void sigc_increment (
  XtPointer client,
  XtIntervalId *id );

static char *dragName[] = {
  activeSignalClass_str8,
  activeSignalClass_str34
};

static void sigc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void sigc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void sigc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void sigc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void sigc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class activeSignalClass : public activeGraphicClass {

private:

friend void sigc_doBlink (
  void *ptr
);

friend void sigc_unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void sigc_monitor_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void sigc_update (
  ProcessVariable *pv,
  void *userarg );

friend void sigc_increment (
  XtPointer client,
  XtIntervalId *id );

friend void sigc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void sigc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void sigc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void sigc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void sigc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

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
  int bufSignalType;
  char bufFreqPvName[PV_Factory::MAX_PV_NAME+1];
  double bufSignalFrequency;
  char bufAmplPvName[PV_Factory::MAX_PV_NAME+1];
  double bufSignalAmplitude;
  char bufPhasePvName[PV_Factory::MAX_PV_NAME+1];
  double bufSignalPhase;
  char bufOffsetPvName[PV_Factory::MAX_PV_NAME+1];
  double bufSignalOffset;
  double bufUpdateRate;
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
  char bufSignalStatePvName[PV_Factory::MAX_PV_NAME+1];
  char bufVisPvName[PV_Factory::MAX_PV_NAME+1];
  char bufMinVisString[39+1];
  char bufMaxVisString[39+1];
  char bufColorPvName[PV_Factory::MAX_PV_NAME+1];
} editBufType, *editBufPtr;

editBufPtr eBuf;

entryListBase *limitsFromDbEntry, *minEntry, *maxEntry;

entryListBase *invisPvEntry, *visInvEntry, *minVisEntry, *maxVisEntry;

int destType, signalStateType;

pvColorClass fgColor, bgColor;
int topShadowColor;
int botShadowColor;
expStringClass label;
int _3D, invisible;

fontMenuClass fm;
char fontTag[63+1];
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

pvConnectionClass connection;

ProcessVariable *destPvId, *signalStatePvId, *amplPvId, *offsetPvId,
 *freqPvId, *phasePvId;
PvCallbackClass *destPvCb, *signalStatePvCb, *amplPvCb, *offsetPvCb,
 *freqPvCb, *phasePvCb;
expStringClass destPvExpString, signalStatePvExpString, amplPvExpString,
 offsetPvExpString, freqPvExpString, phasePvExpString;
int destExists, signalStateExists, amplExists, offsetExists,
 freqExists, phaseExists;

int signalType, firstImpulse;

double signalFrequency, signalAmplitude, signalPhase,
 signalPhaseRads, signalOffset, updateRate, elapsedTime,
 halfPeriod, wfVal, wfInc;

int buttonPressed;

int destPvConnected, signalStatePvConnected,
 active, activeMode, init;

int incrementTimerActive, incrementTimerValue;
XtIntervalId incrementTimer;
double controlV, curControlV;

int needConnectInit, needSignalStateConnectInit, needAmplConnectInit,
 needOffsetConnectInit, needFreqConnectInit, needPhaseConnectInit,
 needCtlInfoInit, needRefresh, needErase, needDraw,
 needToDrawUnconnected, needToEraseUnconnected;
XtIntervalId unconnectedTimer;
int initialSignalStateValueConnection;

int limitsFromDb;
double scaleMin, scaleMax, minDv, maxDv;
efDouble efScaleMin, efScaleMax;

ProcessVariable *visPvId;
PvCallbackClass *visPvCb;
expStringClass visPvExpString;
int visExists;
double visValue, curVisValue, minVis, maxVis;
char minVisString[39+1];
char maxVisString[39+1];
int prevVisibility, visibility, visInverted;
int needVisConnectInit, needVisInit, needVisUpdate;

ProcessVariable *colorPvId;
PvCallbackClass *colorPvCb;
expStringClass colorPvExpString;
int colorExists;
double colorValue, curColorValue;
int needColorConnectInit, needColorInit, needColorUpdate;

int rootX, rootY;

int state;

struct timeval baseTime;

public:

static const int destPvConnectionId = 1;
static const int signalStatePvConnectionId = 2;
static const int visPvConnectionId = 3;
static const int colorPvConnectionId = 4;
static const int amplPvConnectionId = 5;
static const int offsetPvConnectionId = 6;
static const int freqPvConnectionId = 7;
static const int phasePvConnectionId = 8;

activeSignalClass ( void );

activeSignalClass
 ( const activeSignalClass *source );

~activeSignalClass ( void );

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

void *create_activeSignalClassPtr ( void );
void *clone_activeSignalClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
