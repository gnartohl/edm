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

#include "pv_factory.h"
#include "cvtFast.h"

#define BTC_K_COLORMODE_STATIC 0
#define BTC_K_COLORMODE_ALARM 1

#define BTC_MAJOR_VERSION 4
#define BTC_MINOR_VERSION 1
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
  ProcessVariable *pv,
  void *userarg );

static void bt_readUpdate (
  ProcessVariable *pv,
  void *userarg );

static void bt_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void bt_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void bt_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void bt_visUpdate (
  ProcessVariable *pv,
  void *userarg );

static void bt_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void bt_colorUpdate (
  ProcessVariable *pv,
  void *userarg );

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
  ProcessVariable *pv,
  void *userarg );

friend void bt_readUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void bt_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void bt_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void bt_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void bt_visUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void bt_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void bt_colorUpdate (
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
  int bufFgColorMode;
  int bufFgColor;
  int bufOnColor;
  int bufOffColor;
  int bufTopShadowColor;
  int bufBotShadowColor;
  int bufInconsistentColor;
  efInt bufEfControlBitPos;
  efInt bufEfReadBitPos;
  colorButtonClass fgCb;
  colorButtonClass onCb;
  colorButtonClass offCb;
  colorButtonClass topShadowCb;
  colorButtonClass botShadowCb;
  colorButtonClass inconsistentCb;
  int bufDownCallbackFlag;
  int bufUpCallbackFlag;
  int bufActivateCallbackFlag;
  int bufDeactivateCallbackFlag;
  int bufVisInverted;
  char bufOnLabel[MAX_ENUM_STRING_SIZE+1];
  char bufOffLabel[MAX_ENUM_STRING_SIZE+1];
  char bufFontTag[63+1];
  char controlBufPvName[PV_Factory::MAX_PV_NAME+1];
  char readBufPvName[PV_Factory::MAX_PV_NAME+1];
  char bufVisPvName[PV_Factory::MAX_PV_NAME+1];
  char bufMinVisString[39+1];
  char bufMaxVisString[39+1];
  char bufColorPvName[PV_Factory::MAX_PV_NAME+1];
} editBufType, *editBufPtr;

editBufPtr eBuf;

entryListBase *ctlPvEntry, *ctlPvBitEntry;

entryListBase *rdPvEntry, *rdPvBitEntry;

entryListBase *invisPvEntry, *visInvEntry, *minVisEntry, *maxVisEntry;

//short controlV, curControlV, readV, curReadV;
int controlV, curControlV, readV, curReadV;

int needCtlConnectInit, needCtlInfoInit, needCtlRefresh;
int needReadConnectInit, needReadInfoInit, needReadRefresh;
int needErase, needDraw, needToDrawUnconnected, needToEraseUnconnected;
XtIntervalId unconnectedTimer;

int fgColorMode;
pvColorClass fgColor, inconsistentColor, onColor, offColor;
int topShadowColor;
int botShadowColor;
char onLabel[MAX_ENUM_STRING_SIZE+1];
char offLabel[MAX_ENUM_STRING_SIZE+1];
char _3DString[7+1], invisibleString[7+1];
int labelType, buttonType, _3D, invisible;
char labelTypeString[15+1];
char buttonTypeStr[7+1];

VPFUNC downCallback, upCallback, activateCallback, deactivateCallback;
int downCallbackFlag, upCallbackFlag, activateCallbackFlag,
 deactivateCallbackFlag, anyCallbackFlag;

fontMenuClass fm;
char fontTag[63+1];
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

static const int controlPvConnection = 1;
static const int readPvConnection = 2;
static const int visPvConnection = 3;
static const int colorPvConnection = 4;

pvConnectionClass connection;

ProcessVariable *controlPvId, *readPvId, *stateStringPvId;

expStringClass controlPvName, readPvName;

int controlExists, readExists, toggle;

int controlPvConnected, readPvConnected, init, active, activeMode,
 controlValid, readValid;

ProcessVariable *visPvId;
expStringClass visPvExpString;
int visExists;
double visValue, curVisValue, minVis, maxVis;
char minVisString[39+1];
char maxVisString[39+1];
int prevVisibility, visibility, visInverted;
int needVisConnectInit, needVisInit, needVisUpdate;
int initialConnection, initialReadConnection, initialVisConnection,
 initialColorConnection;

ProcessVariable *colorPvId;
expStringClass colorPvExpString;
int colorExists;
double colorValue, curColorValue;
int needColorConnectInit, needColorInit, needColorUpdate;

int oldStat, oldSev;

int controlIsBit, readIsBit;
efInt efControlBitPos, efReadBitPos;
int controlBitPos, readBitPos; // 0-31
int prevControlBit, prevReadBit;
int controlBit, readBit;
int initControlBit, initReadBit;
int buttonIsDown; // boolean that indicates if we have received the buttonDown event and not yet received the corresponding buttonUp event.

public:

activeButtonClass ( void );

activeButtonClass
 ( const activeButtonClass *source );

~activeButtonClass ( void );

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

int old_save (
  FILE *f );

int createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int old_createFromFile (
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

int setProperty (
  char *prop,
  int *value );

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

void *create_activeButtonClassPtr ( void );
void *clone_activeButtonClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
