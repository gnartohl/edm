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

#ifndef __menu_button_h
#define __menu_button_h 1

#include "act_grf.h"
#include "entry_form.h"

#ifdef __epics__
#include "cadef.h"
#endif

#ifndef __epics__
#define MAX_ENUM_STATES 4
#define MAX_ENUM_STRING_SIZE 16
#endif

#define MBTC_K_COLORMODE_STATIC 0
#define MBTC_K_COLORMODE_ALARM 1

#define MBTC_MAJOR_VERSION 2
#define MBTC_MINOR_VERSION 0
#define MBTC_RELEASE 0

#define MBTC_K_LITERAL 1
#define MBTC_K_PV_STATE 2

#ifdef __menu_button_cc

#include "menu_button.str"

static char *dragName[] = {
  activeMenuButtonClass_str1,
};

static void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

static void mbt_infoUpdate (
  struct event_handler_args ast_args );

static void mbt_readInfoUpdate (
  struct event_handler_args ast_args );

static void putValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mbt_controlUpdate (
  struct event_handler_args ast_args );

static void mbt_alarmUpdate (
  struct event_handler_args ast_args );

static void mbt_monitor_control_connect_state (
  struct connection_handler_args arg );

static void mbt_readUpdate (
  struct event_handler_args ast_args );

static void mbt_readAlarmUpdate (
  struct event_handler_args ast_args );

static void mbt_monitor_read_connect_state (
  struct connection_handler_args arg );

#endif

class activeMenuButtonClass : public activeGraphicClass {

private:

friend void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

friend void mbt_infoUpdate (
  struct event_handler_args ast_args );

friend void mbt_readInfoUpdate (
  struct event_handler_args ast_args );

friend void putValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mbt_controlUpdate (
  struct event_handler_args ast_args );

friend void mbt_alarmUpdate (
  struct event_handler_args ast_args );

friend void mbt_monitor_control_connect_state (
  struct connection_handler_args arg );

friend void mbt_readUpdate (
  struct event_handler_args ast_args );

friend void mbt_readAlarmUpdate (
  struct event_handler_args ast_args );

friend void mbt_monitor_read_connect_state (
  struct connection_handler_args arg );

pvConnectionClass connection;

int opComplete, pvCheckExists;

int bufX, bufY, bufW, bufH;

short curValue, value;

unsigned int topShadowColor, bufTopShadowColor;
unsigned int botShadowColor, bufBotShadowColor;
unsigned int bufFgColor, bufBgColor;
pvColorClass fgColor, bgColor;
colorButtonClass fgCb, bgCb, topShadowCb, botShadowCb;
int fgColorMode, bgColorMode, bufFgColorMode, bufBgColorMode;

char *stateString[MAX_ENUM_STATES]; // allocated at run-time
int numStates;

fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
XmFontList fontList;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

#ifdef __epics__
chid controlPvId, readPvId;
evid alarmEventId, controlEventId, readAlarmEventId, readEventId;
#endif

char bufControlPvName[39+1];
expStringClass controlPvExpStr;

char bufReadPvName[39+1];
expStringClass readPvExpStr;

int controlExists, readExists, widgetsCreated, active, activeMode;

Widget optionMenu, pulldownMenu, curHistoryWidget,
 pb[MAX_ENUM_STATES];

int needConnectInit, needReadConnectInit, needInfoInit,
 needReadInfoInit, needDraw, needRefresh;

public:

activeMenuButtonClass::activeMenuButtonClass ( void );

activeMenuButtonClass::~activeMenuButtonClass ( void );

activeMenuButtonClass::activeMenuButtonClass
 ( const activeMenuButtonClass *source );

char *activeMenuButtonClass::objName ( void ) {

  return name;

}

int activeMenuButtonClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeMenuButtonClass::save (
  FILE *f );

int activeMenuButtonClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeMenuButtonClass::genericEdit ( void );

int activeMenuButtonClass::edit ( void );

int activeMenuButtonClass::editCreate ( void );

int activeMenuButtonClass::draw ( void );

int activeMenuButtonClass::erase ( void );

int activeMenuButtonClass::drawActive ( void );

int activeMenuButtonClass::eraseActive ( void );

int activeMenuButtonClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeMenuButtonClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeMenuButtonClass::containsMacros ( void );

int activeMenuButtonClass::createWidgets ( void );

int activeMenuButtonClass::activate ( int pass, void *ptr );

int activeMenuButtonClass::deactivate ( int pass );

void activeMenuButtonClass::updateDimensions ( void );

void activeMenuButtonClass::executeDeferred ( void );

char *activeMenuButtonClass::firstDragName ( void );

char *activeMenuButtonClass::nextDragName ( void );

char *activeMenuButtonClass::dragValue (
  int i );

void activeMenuButtonClass::changeDisplayParams (
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

void activeMenuButtonClass::changePvNames (
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

void *create_activeMenuButtonClassPtr ( void );
void *clone_activeMenuButtonClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
