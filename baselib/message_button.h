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

#ifndef __message_button_h
#define __message_button_h 1

#include "act_grf.h"
#include "entry_form.h"

#include "cadef.h"

#define MSGBTC_MAJOR_VERSION 2
#define MSGBTC_MINOR_VERSION 0
#define MSGBTC_RELEASE 0

#define MSGBTC_K_PUSH 1
#define MSGBTC_K_TOGGLE 2

#ifdef __message_button_cc

#include "message_button.str"

static char *dragName[] = {
  activeMessageButtonClass_str18,
};

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void msgbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void msgbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void msgbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void msgbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void msgbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#if 0
static void msgbt_sourcePressUpdate (
  struct event_handler_args ast_args );

static void msgbt_monitor_sourcePress_connect_state (
  struct connection_handler_args arg );

static void msgbt_sourceReleaseUpdate (
  struct event_handler_args ast_args );

static void msgbt_monitor_sourceRelease_connect_state (
  struct connection_handler_args arg );
#endif

static void msgbt_monitor_dest_connect_state (
  struct connection_handler_args arg );

#endif

class activeMessageButtonClass : public activeGraphicClass {

private:

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void msgbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void msgbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void msgbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void msgbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void msgbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#if 0
friend void msgbt_sourcePressUpdate (
  struct event_handler_args ast_args );

friend void msgbt_monitor_sourcePress_connect_state (
  struct connection_handler_args arg );

friend void msgbt_sourceReleaseUpdate (
  struct event_handler_args ast_args );

friend void msgbt_monitor_sourceRelease_connect_state (
  struct connection_handler_args arg );
#endif

friend void msgbt_monitor_dest_connect_state (
  struct connection_handler_args arg );

int opComplete;

int minW;
int minH;

int bufX, bufY, bufW, bufH;

int sourcePressType, sourceReleaseType, destType;

int bufFgColor, bufOnColor, bufOffColor;
pvColorClass fgColor, onColor, offColor;
int topShadowColor, bufTopShadowColor;
int botShadowColor, bufBotShadowColor;
colorButtonClass fgCb, onCb, offCb, topShadowCb, botShadowCb;
expStringClass onLabel, offLabel;
char bufOnLabel[MAX_ENUM_STRING_SIZE+1];
char bufOffLabel[MAX_ENUM_STRING_SIZE+1];
// char onLabel[MAX_ENUM_STRING_SIZE+1], bufOnLabel[MAX_ENUM_STRING_SIZE+1];
// char offLabel[MAX_ENUM_STRING_SIZE+1], bufOffLabel[MAX_ENUM_STRING_SIZE+1];
int buttonType, bufButtonType, _3D, buf3D, invisible, bufInvisible,
 toggle, bufToggle, pressAction, bufPressAction,
 releaseAction, bufReleaseAction;

fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

chid sourcePressPvId, sourceReleasePvId, destPvId;
evid sourcePressEventId, sourceReleaseEventId;

expStringClass destPvExpString;
char bufDestPvName[activeGraphicClass::MAX_PV_NAME+1];
expStringClass sourcePressPvExpString;
char bufSourcePressPvName[39+1];
expStringClass sourceReleasePvExpString;
char bufSourceReleasePvName[39+1];

pvValType sourcePressV, sourceReleaseV, destV;

int destExists, sourcePressExists, sourceReleaseExists, buttonPressed;

int sourcePressPvConnected, sourceReleasePvConnected, destPvConnected,
 active, activeMode, init;

int needConnectInit, needErase, needDraw, needToEraseUnconnected,
 needToDrawUnconnected;
int unconnectedTimer;

public:

activeMessageButtonClass::activeMessageButtonClass ( void );

activeMessageButtonClass::activeMessageButtonClass
 ( const activeMessageButtonClass *source );

activeMessageButtonClass::~activeMessageButtonClass ( void );

char *activeMessageButtonClass::objName ( void ) {

  return name;

}

int activeMessageButtonClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeMessageButtonClass::save (
  FILE *f );

int activeMessageButtonClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeMessageButtonClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int activeMessageButtonClass::genericEdit ( void );

int activeMessageButtonClass::edit ( void );

int activeMessageButtonClass::editCreate ( void );

int activeMessageButtonClass::draw ( void );

int activeMessageButtonClass::erase ( void );

int activeMessageButtonClass::drawActive ( void );

int activeMessageButtonClass::eraseActive ( void );

int activeMessageButtonClass::activate ( int pass, void *ptr );

int activeMessageButtonClass::deactivate ( int pass );

void activeMessageButtonClass::updateDimensions ( void );

void activeMessageButtonClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void activeMessageButtonClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void activeMessageButtonClass::pointerIn (
  int _x,
  int _y,
  int buttonState );

int activeMessageButtonClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

int activeMessageButtonClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeMessageButtonClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeMessageButtonClass::containsMacros ( void );

void activeMessageButtonClass::executeDeferred ( void );

char *activeMessageButtonClass::firstDragName ( void );

char *activeMessageButtonClass::nextDragName ( void );

char *activeMessageButtonClass::dragValue (
  int i );

void activeMessageButtonClass::changeDisplayParams (
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

void activeMessageButtonClass::changePvNames (
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

void *create_activeMessageButtonClassPtr ( void );
void *clone_activeMessageButtonClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
