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

#ifndef __buttonGen_h
#define __buttonGen_h 1

#include "ulBindings.h"
#include "act_grf.h"
#include "entry_form.h"

// #ifdef __epics__
// #include "cadef.h"
// #endif

#include "pv.h"

// #ifndef __epics__
#define MAX_ENUM_STRING_SIZE 63
// #endif

#define BTC_K_COLORMODE_STATIC 0
#define BTC_K_COLORMODE_ALARM 1

#define BTC_MAJOR_VERSION 1
#define BTC_MINOR_VERSION 4
#define BTC_RELEASE 0

#define BTC_K_LITERAL 1
#define BTC_K_PV_STATE 2
#define BTC_K_PUSH 3
#define BTC_K_TOGGLE 4

#ifdef __buttonGen_cc

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
  pvClass *classPtr,
  void *clientData,
  void *args );

static void bt_controlInfoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void bt_readInfoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void bt_readUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void bt_alarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void bt_monitor_control_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void bt_monitor_read_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

#endif

class activeButtonClass : public activeGraphicClass {

private:

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
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void bt_controlInfoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void bt_readInfoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void bt_readUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void bt_alarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void bt_monitor_control_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void bt_monitor_read_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

int opComplete;

int minW;
int minH;

int bufX, bufY, bufW, bufH;

short controlV, curControlV, readV, curReadV;

int needCtlConnectInit, needCtlInfoInit, needCtlRefresh;
int needReadConnectInit, needReadInfoInit, needReadRefresh;
int needErase, needDraw;

int fgColorMode, bufFgColorMode;
unsigned int bufFgColor, bufOnColor, bufOffColor;
pvColorClass fgColor, inconsistentColor, onColor, offColor;
unsigned int topShadowColor, bufTopShadowColor;
unsigned int botShadowColor, bufBotShadowColor;
unsigned int bufInconsistentColor;
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

// #ifdef __epics__
// chid controlPvId, readPvId;
// evid infoEventId, controlEventId, readEventId, alarmEventId;
// #endif

pvClass *controlPvId;
pvClass *readPvId;
pvEventClass *infoEventId, *controlEventId;
pvEventClass *readEventId, *alarmEventId;

char controlBufPvName[39+1];
char readBufPvName[39+1];
expStringClass controlPvName, readPvName;

int controlExists, readExists, toggle;

int controlPvConnected, readPvConnected, init, active, activeMode,
 controlValid, readValid;

char pvClassName[15+1], pvUserClassName[15+1], pvOptionList[255+1];
int numPvTypes, pvNameIndex;

public:

activeButtonClass ( void );

activeButtonClass
 ( const activeButtonClass *source );

~activeButtonClass ( void ) {

/*   printf( "In ~activeButtonClass\n" ); */

  if ( name ) delete name;

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

int erase ( void );

int drawActive ( void );

int eraseActive ( void );

int activate ( int pass, void *ptr );

int deactivate ( int pass );

void updateDimensions ( void );

void btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

void btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

int getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

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

int setProperty (
  char *prop,
  int *value );

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
