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

#ifndef __updownButton_h
#define __updownButton_h 1

#include "act_grf.h"
#include "entry_form.h"
#include "keypad.h"

#include "cadef.h"

#define UDBTC_MAJOR_VERSION 1
#define UDBTC_MINOR_VERSION 2
#define UDBTC_RELEASE 0

#ifdef __updownButton_cc

#include "updownButton.str"

static void udbtoCancelKp (
  Widget w,
  XtPointer client,
  XtPointer call );

static void udbtoSetKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void udbt_infoUpdate (
 struct event_handler_args ast_args );

static void udbtc_controlUpdate (
  struct event_handler_args ast_args );

static void udbtc_saveUpdate (
  struct event_handler_args ast_args );

static void udbtc_decrement (
  XtPointer client,
  XtIntervalId *id );

static void udbtc_increment (
  XtPointer client,
  XtIntervalId *id );

static char *dragName[] = {
  activeUpdownButtonClass_str18,
};

static void udbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void udbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void udbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void udbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void udbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void udbtc_monitor_dest_connect_state (
  struct connection_handler_args arg );

static void udbtc_monitor_save_connect_state (
  struct connection_handler_args arg );

#endif

class activeUpdownButtonClass : public activeGraphicClass {

private:

friend void udbtoCancelKp (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void udbtoSetKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void udbt_infoUpdate (
 struct event_handler_args ast_args );

friend void udbtc_controlUpdate (
  struct event_handler_args ast_args );

friend void udbtc_saveUpdate (
  struct event_handler_args ast_args );

friend void udbtc_decrement (
  XtPointer client,
  XtIntervalId *id );

friend void udbtc_increment (
  XtPointer client,
  XtIntervalId *id );

friend void udbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void udbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void udbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void udbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void udbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void udbtc_monitor_dest_connect_state (
  struct connection_handler_args arg );

friend void udbtc_monitor_save_connect_state (
  struct connection_handler_args arg );

int opComplete;

int minW;
int minH;

int bufX, bufY, bufW, bufH;

int destType, saveType;

int bufFgColor, bufBgColor;
pvColorClass fgColor, bgColor;
int topShadowColor, bufTopShadowColor;
int botShadowColor, bufBotShadowColor;
colorButtonClass fgCb, bgCb, topShadowCb, botShadowCb;
expStringClass label;
char bufLabel[39+1];
int _3D, buf3D, invisible, bufInvisible;

fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

chid destPvId, savePvId;
evid destEventId, saveEventId;

expStringClass destPvExpString;
char bufDestPvName[activeGraphicClass::MAX_PV_NAME+1];

expStringClass savePvExpString;
char bufSavePvName[activeGraphicClass::MAX_PV_NAME+1];

expStringClass fineExpString;
char bufFine[39+1];

expStringClass coarseExpString;
char bufCoarse[39+1];

double rate, bufRate;

int destExists, saveExists, buttonPressed;

int destPvConnected, savePvConnected, active, activeMode, init;

int incrementTimerActive, incrementTimerValue;
XtIntervalId incrementTimer;
double controlV, curControlV, curSaveV, coarse, fine;

int needConnectInit, needSaveConnectInit, needCtlInfoInit, needRefresh,
 needErase, needDraw;

int widgetsCreated;
Widget popUpMenu, pullDownMenu, pbCoarse, pbFine, pbRate, pbValue,
 pbSave, pbRestore;

double kpDouble;
keypadClass kp;
int keyPadOpen, kpDest;

int isSaved;

static const int kpCoarseDest = 1;
static const int kpFineDest = 2;
static const int kpRateDest = 3;
static const int kpValueDest = 4;

int limitsFromDb, bufLimitsFromDb;
double scaleMin, scaleMax, minDv, maxDv;
efDouble efScaleMin, efScaleMax, bufEfScaleMin, bufEfScaleMax;;

public:

activeUpdownButtonClass::activeUpdownButtonClass ( void );

activeUpdownButtonClass::activeUpdownButtonClass
 ( const activeUpdownButtonClass *source );

activeUpdownButtonClass::~activeUpdownButtonClass ( void ) {

  if ( name ) delete name;

}

char *activeUpdownButtonClass::objName ( void ) {

  return name;

}

int activeUpdownButtonClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeUpdownButtonClass::save (
  FILE *f );

int activeUpdownButtonClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeUpdownButtonClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int activeUpdownButtonClass::genericEdit ( void );

int activeUpdownButtonClass::edit ( void );

int activeUpdownButtonClass::editCreate ( void );

int activeUpdownButtonClass::draw ( void );

int activeUpdownButtonClass::erase ( void );

int activeUpdownButtonClass::drawActive ( void );

int activeUpdownButtonClass::eraseActive ( void );

int activeUpdownButtonClass::activate ( int pass, void *ptr );

int activeUpdownButtonClass::deactivate ( int pass );

void activeUpdownButtonClass::updateDimensions ( void );

void activeUpdownButtonClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void activeUpdownButtonClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void activeUpdownButtonClass::pointerIn (
  int _x,
  int _y,
  int buttonState );

int activeUpdownButtonClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

int activeUpdownButtonClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeUpdownButtonClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeUpdownButtonClass::containsMacros ( void );

void activeUpdownButtonClass::executeDeferred ( void );

char *activeUpdownButtonClass::firstDragName ( void );

char *activeUpdownButtonClass::nextDragName ( void );

char *activeUpdownButtonClass::dragValue (
  int i );

void activeUpdownButtonClass::changeDisplayParams (
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

void activeUpdownButtonClass::changePvNames (
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

void *create_activeUpdownButtonClassPtr ( void );
void *clone_activeUpdownButtonClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
