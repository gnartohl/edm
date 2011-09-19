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

#include "pv_factory.h"
#include "cvtFast.h"

#define MSGBTC_MAJOR_VERSION 4
#define MSGBTC_MINOR_VERSION 1
#define MSGBTC_RELEASE 0

#define MSGBTC_K_PUSH 1
#define MSGBTC_K_TOGGLE 2

#ifdef __message_button_cc

#include "message_button.str"

static char *dragName[] = {
  activeMessageButtonClass_str18,
  activeMessageButtonClass_str32,
  activeMessageButtonClass_str28
};

static void doBlink (
  void *ptr
);

static void pw_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pw_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pw_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

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

static void msgbt_monitor_dest_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void msgbt_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void msgbt_visUpdate (
  ProcessVariable *pv,
  void *userarg );

static void msgbt_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void msgbt_colorUpdate (
  ProcessVariable *pv,
  void *userarg );

#endif

class activeMessageButtonClass : public activeGraphicClass {

private:

friend void doBlink (
  void *ptr
);

friend void pw_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pw_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pw_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

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

friend void msgbt_monitor_dest_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void msgbt_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void msgbt_visUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void msgbt_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void msgbt_colorUpdate (
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
  int bufOnColor;
  int bufOffColor;
  int bufTopShadowColor;
  int bufBotShadowColor;
  int bufButtonType;
  int buf3D;
  int bufInvisible;
  int bufToggle;
  int bufPressAction;
  int bufReleaseAction;
  char bufMinVisString[39+1];
  char bufMaxVisString[39+1];
  int bufVisInverted;
  int bufLock;
  int bufUseEnumNumeric;
  char bufDestPvName[PV_Factory::MAX_PV_NAME+1];
  char bufSourcePressPvName[PV_Factory::MAX_PV_NAME+1];
  char bufSourceReleasePvName[PV_Factory::MAX_PV_NAME+1];
  char bufVisPvName[PV_Factory::MAX_PV_NAME+1];
  char bufColorPvName[PV_Factory::MAX_PV_NAME+1];
  char bufOnLabel[MAX_ENUM_STRING_SIZE+1];
  char bufOffLabel[MAX_ENUM_STRING_SIZE+1];
} editBufType, *editBufPtr;

editBufPtr eBuf;

entryListBase *invisPvEntry, *visInvEntry, *minVisEntry, *maxVisEntry;

char bufPw1[31+1];
char bufPw2[31+1];

int sourcePressType, sourceReleaseType, destType;

pvColorClass fgColor, onColor, offColor;
int topShadowColor;
int botShadowColor;
colorButtonClass fgCb, onCb, offCb, topShadowCb, botShadowCb;
expStringClass onLabel, offLabel;
int buttonType, _3D, invisible, toggle, pressAction, releaseAction;

fontMenuClass fm;
char fontTag[63+1];
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

ProcessVariable *sourcePressPvId, *sourceReleasePvId, *destPvId;

int destIsAckS;

expStringClass destPvExpString;
expStringClass sourcePressPvExpString;
expStringClass sourceReleasePvExpString;

//-------------------------------------------------------
static const int destPvConnection = 1;
static const int visPvConnection = 2;
static const int colorPvConnection = 3;
pvConnectionClass connection;
//-------------------------------------------------------

//-------------------------------------------------------
ProcessVariable *visPvId;
expStringClass visPvExpString;
int visExists;
double visValue, curVisValue, minVis, maxVis;
char minVisString[39+1];
char maxVisString[39+1];
int prevVisibility, visibility, visInverted;
int needVisConnectInit, needVisInit, needVisUpdate;
int initialVisConnection, initialColorConnection;
//-------------------------------------------------------

//-------------------------------------------------------
ProcessVariable *colorPvId;
expStringClass colorPvExpString;
int colorExists;
double colorValue, curColorValue;
int needColorConnectInit, needColorInit, needColorUpdate;
//-------------------------------------------------------

pvValType sourcePressV, sourceReleaseV, destV;

int destExists, sourcePressExists, sourceReleaseExists, buttonPressed;

int sourcePressPvConnected, sourceReleasePvConnected, destPvConnected,
 active, activeMode, init;

int needConnectInit, needErase, needDraw, needToEraseUnconnected,
 needToDrawUnconnected;
XtIntervalId unconnectedTimer;

char pw[31+1];
int usePassword;

int lock;

int useEnumNumeric;

int numStates;

int pwFormX, pwFormY, pwFormW, pwFormH, pwFormMaxH;

int needPerformDownAction, needPerformUpAction, needWarning;

public:

activeMessageButtonClass ( void );

activeMessageButtonClass
 ( const activeMessageButtonClass *source );

~activeMessageButtonClass ( void );

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
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

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

void performBtnUpAction ( void );

void btnUp (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void performBtnDownAction ( void );

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

int getEnumNumeric (
  char *string,
  int *value );

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

void *create_activeMessageButtonClassPtr ( void );
void *clone_activeMessageButtonClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
