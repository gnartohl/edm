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

#ifndef __button_h
#define __button_h 1

#include "ulBindings.h"
#include "act_grf.h"
#include "entry_form.h"

#include "cadef.h"

#define BTC_K_COLORMODE_STATIC 0
#define BTC_K_COLORMODE_ALARM 1

#define BTC_MAJOR_VERSION 2
#define BTC_MINOR_VERSION 4
#define BTC_RELEASE 0

#define BTC_K_LITERAL 1
#define BTC_K_PV_STATE 2
#define BTC_K_PUSH 3
#define BTC_K_TOGGLE 4

#ifdef __button_cc

#include "button.str"

static char *dragName[] = {
  activeButtonClass_str1,
  activeButtonClass_str2,
  activeButtonClass_str62,
  activeButtonClass_str58
};

static void doBlink (
  void *ptr
);

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void btc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void btc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void btc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void btc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void btc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void bt_controlUpdate (
  struct event_handler_args ast_args );

static void bt_controlInfoUpdate (
  struct event_handler_args ast_args );

static void bt_readInfoUpdate (
  struct event_handler_args ast_args );

static void bt_readUpdate (
  struct event_handler_args ast_args );

static void bt_alarmUpdate (
  struct event_handler_args ast_args );

static void bt_monitor_control_connect_state (
  struct connection_handler_args arg );

static void bt_monitor_read_connect_state (
  struct connection_handler_args arg );

static void bt_monitor_vis_connect_state (
  struct connection_handler_args arg );

static void bt_visInfoUpdate (
  struct event_handler_args ast_args );

static void bt_visUpdate (
  struct event_handler_args ast_args );

static void bt_monitor_color_connect_state (
  struct connection_handler_args arg );

static void bt_colorInfoUpdate (
  struct event_handler_args ast_args );

static void bt_colorUpdate (
  struct event_handler_args ast_args );

#endif

class activeButtonClass : public activeGraphicClass {

private:

friend void doBlink (
  void *ptr
);

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void btc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void btc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void btc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void btc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void btc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void bt_controlUpdate (
  struct event_handler_args ast_args );

friend void bt_controlInfoUpdate (
  struct event_handler_args ast_args );

friend void bt_readInfoUpdate (
  struct event_handler_args ast_args );

friend void bt_readUpdate (
  struct event_handler_args ast_args );

friend void bt_alarmUpdate (
  struct event_handler_args ast_args );

friend void bt_monitor_control_connect_state (
  struct connection_handler_args arg );

friend void bt_monitor_read_connect_state (
  struct connection_handler_args arg );

friend void bt_monitor_vis_connect_state (
  struct connection_handler_args arg );

friend void bt_visInfoUpdate (
  struct event_handler_args ast_args );

friend void bt_visUpdate (
  struct event_handler_args ast_args );

friend void bt_monitor_color_connect_state (
  struct connection_handler_args arg );

friend void bt_colorInfoUpdate (
  struct event_handler_args ast_args );

friend void bt_colorUpdate (
  struct event_handler_args ast_args );

int opComplete;

int minW;
int minH;

int bufX, bufY, bufW, bufH;

short controlV, curControlV, readV, curReadV;

int needCtlConnectInit, needCtlInfoInit, needCtlRefresh;
int needReadConnectInit, needReadInfoInit, needReadRefresh;
int needErase, needDraw, needToDrawUnconnected, needToEraseUnconnected;
int unconnectedTimer;

int fgColorMode, bufFgColorMode;
int bufFgColor, bufOnColor, bufOffColor;
pvColorClass fgColor, inconsistentColor, onColor, offColor;
int topShadowColor, bufTopShadowColor;
int botShadowColor, bufBotShadowColor;
int bufInconsistentColor;
colorButtonClass fgCb, onCb, offCb, topShadowCb, botShadowCb, inconsistentCb;
char onLabel[MAX_ENUM_STRING_SIZE+1], bufOnLabel[MAX_ENUM_STRING_SIZE+1];
char offLabel[MAX_ENUM_STRING_SIZE+1], bufOffLabel[MAX_ENUM_STRING_SIZE+1];
char stateString[2][MAX_ENUM_STRING_SIZE+1];
char _3DString[7+1], invisibleString[7+1];
int no_str, labelType, buttonType, _3D, invisible;
char labelTypeString[15+1];
char buttonTypeStr[7+1];

VPFUNC downCallback, upCallback, activateCallback, deactivateCallback;
int downCallbackFlag, upCallbackFlag, activateCallbackFlag,
 deactivateCallbackFlag, anyCallbackFlag;
int bufDownCallbackFlag, bufUpCallbackFlag, bufActivateCallbackFlag,
 bufDeactivateCallbackFlag;

fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

static const int controlPvConnection = 1;
static const int readPvConnection = 2;
static const int visPvConnection = 3;
static const int colorPvConnection = 4;

pvConnectionClass connection;

chid controlPvId, readPvId;
evid infoEventId, controlEventId, readEventId, alarmEventId;

char controlBufPvName[activeGraphicClass::MAX_PV_NAME+1];
char readBufPvName[activeGraphicClass::MAX_PV_NAME+1];
expStringClass controlPvName, readPvName;

int controlExists, readExists, toggle;

int controlPvConnected, readPvConnected, init, active, activeMode,
 controlValid, readValid;

chid visPvId;
evid visEventId;
expStringClass visPvExpString;
char bufVisPvName[activeGraphicClass::MAX_PV_NAME+1];
int visExists;
double visValue, curVisValue, minVis, maxVis;
char minVisString[39+1], bufMinVisString[39+1];
char maxVisString[39+1], bufMaxVisString[39+1];
int prevVisibility, visibility, visInverted, bufVisInverted;
int needVisConnectInit, needVisInit, needVisUpdate;

chid colorPvId;
evid colorEventId;
expStringClass colorPvExpString;
char bufColorPvName[activeGraphicClass::MAX_PV_NAME+1];
int colorExists;
double colorValue, curColorValue;
int needColorConnectInit, needColorInit, needColorUpdate;

public:

activeButtonClass::activeButtonClass ( void );

activeButtonClass::activeButtonClass
 ( const activeButtonClass *source );

activeButtonClass::~activeButtonClass ( void );

char *activeButtonClass::objName ( void ) {

  return name;

}

int activeButtonClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeButtonClass::save (
  FILE *f );

int activeButtonClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeButtonClass::importFromXchFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeButtonClass::genericEdit ( void );

int activeButtonClass::edit ( void );

int activeButtonClass::editCreate ( void );

int activeButtonClass::draw ( void );

int activeButtonClass::erase ( void );

int activeButtonClass::drawActive ( void );

int activeButtonClass::eraseActive ( void );

int activeButtonClass::activate ( int pass, void *ptr );

int activeButtonClass::deactivate ( int pass );

void activeButtonClass::updateDimensions ( void );

void activeButtonClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

void activeButtonClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

void activeButtonClass::pointerIn (
  int _x,
  int _y,
  int buttonState );

int activeButtonClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

int activeButtonClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeButtonClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeButtonClass::containsMacros ( void );

void activeButtonClass::executeDeferred ( void );

int activeButtonClass::setProperty (
  char *prop,
  int *value );

char *activeButtonClass::firstDragName ( void );

char *activeButtonClass::nextDragName ( void );

char *activeButtonClass::dragValue (
  int i );

void activeButtonClass::changeDisplayParams (
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

void activeButtonClass::changePvNames (
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

void *create_activeButtonClassPtr ( void );
void *clone_activeButtonClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
