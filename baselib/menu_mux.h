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

#ifndef __menu_mux_h
#define __menu_mux_h 1

#include "act_grf.h"
#include "entry_form.h"

#include "cadef.h"

#define MMUX_MAX_ENTRIES 4
#define MMUX_MAX_STATES 16
#define MMUX_MAX_STRING_SIZE 32

#define MMUXC_K_COLORMODE_STATIC 0
#define MMUXC_K_COLORMODE_ALARM 1

#define MMUXC_MAJOR_VERSION 2
#define MMUXC_MINOR_VERSION 0
#define MMUXC_RELEASE 0

#define MMUXC_K_LITERAL 1
#define MMUXC_K_PV_STATE 2

#ifdef __menu_mux_cc

#include "menu_mux.str"

static char *dragName[] = {
  menuMuxClass_str1,
};

static void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

static void mmux_putValueNoPv (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mmux_putValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mmuxc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mmuxc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mmuxc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mmuxc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mmuxc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mmux_controlUpdate (
  struct event_handler_args ast_args );

static void mmux_alarmUpdate (
  struct event_handler_args ast_args );

static void mmux_monitor_control_connect_state (
  struct connection_handler_args arg );

static void mmux_infoUpdate (
  struct event_handler_args ast_args );

static void mmuxSetItem (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class menuMuxClass : public activeGraphicClass {

private:

friend void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

friend void mmux_putValueNoPv (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mmux_putValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mmuxc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mmuxc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mmuxc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mmuxc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mmuxc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mmux_controlUpdate (
  struct event_handler_args ast_args );

friend void mmux_alarmUpdate (
  struct event_handler_args ast_args );

friend void mmux_monitor_control_connect_state (
  struct connection_handler_args arg );

friend void mmux_infoUpdate (
  struct event_handler_args ast_args );

friend void mmuxSetItem (
  Widget w,
  XtPointer client,
  XtPointer call );

int opComplete, firstEvent;

int bufX, bufY, bufW, bufH;

int controlV, curControlV;

int topShadowColor, bufTopShadowColor;
int botShadowColor, bufBotShadowColor;
int bufFgColor, bufBgColor;
pvColorClass fgColor, bgColor;
colorButtonClass fgCb, bgCb, topShadowCb, botShadowCb;
int fgColorMode, bgColorMode, bufFgColorMode, bufBgColorMode;

char *stateString[MMUX_MAX_STATES]; // allocated at run-time
int numStates;

entryListBase *elbt, *elbm[MMUX_MAX_ENTRIES], *elbe[MMUX_MAX_ENTRIES];

char tag[MMUX_MAX_STATES][MMUX_MAX_STRING_SIZE+1],
 m[MMUX_MAX_STATES][MMUX_MAX_ENTRIES][MMUX_MAX_STRING_SIZE+1],
 e[MMUX_MAX_STATES][MMUX_MAX_ENTRIES][MMUX_MAX_STRING_SIZE+1];

char bufTag[MMUX_MAX_STATES][MMUX_MAX_STRING_SIZE+1],
 bufM[MMUX_MAX_STATES][MMUX_MAX_ENTRIES][MMUX_MAX_STRING_SIZE+1],
 bufE[MMUX_MAX_STATES][MMUX_MAX_ENTRIES][MMUX_MAX_STRING_SIZE+1];

char *tagPtr[MMUX_MAX_STATES], *mPtr[MMUX_MAX_ENTRIES][MMUX_MAX_STATES],
 *ePtr[MMUX_MAX_ENTRIES][MMUX_MAX_STATES];

char **mac, **exp;
int numItems, numMac;

fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
XmFontList fontList;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

chid controlPvId;
evid alarmEventId, controlEventId;

char bufControlPvName[39+1];
expStringClass controlPvExpStr;

char bufInitialState[15+1];
expStringClass initialStateExpStr;

int controlExists, widgetsCreated, controlPvConnected, active, activeMode;

Widget optionMenu, pulldownMenu, curHistoryWidget,
 pb[MMUX_MAX_STATES];

int needConnectInit, needDisconnect, needInfoInit, needUpdate, needDraw;

public:

menuMuxClass::menuMuxClass ( void );

menuMuxClass::menuMuxClass
 ( const menuMuxClass *source );

menuMuxClass::~menuMuxClass ( void );

char *menuMuxClass::objName ( void ) {

  return name;

}

int menuMuxClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int menuMuxClass::save (
  FILE *f );

int menuMuxClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int menuMuxClass::genericEdit ( void );

int menuMuxClass::edit ( void );

int menuMuxClass::editCreate ( void );

int menuMuxClass::draw ( void );

int menuMuxClass::erase ( void );

int menuMuxClass::drawActive ( void );

int menuMuxClass::eraseActive ( void );

int menuMuxClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int menuMuxClass::getMacros (
  int *numMacros,
  char ***macro,
  char ***expansion );

int menuMuxClass::createWidgets ( void );

int menuMuxClass::activate ( int pass, void *ptr );

int menuMuxClass::deactivate ( int pass );

void menuMuxClass::updateDimensions ( void );

int isMux ( void ) { return 1; }

void menuMuxClass::executeDeferred ( void );

char *menuMuxClass::firstDragName ( void );

char *menuMuxClass::nextDragName ( void );

char *menuMuxClass::dragValue (
  int i );

void menuMuxClass::changeDisplayParams (
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

void menuMuxClass::changePvNames (
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

void *create_menuMuxClassPtr ( void );
void *clone_menuMuxClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
