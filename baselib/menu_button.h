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

#include "pv_factory.h"
#include "cvtFast.h"

#define MAX_ENUM_STATES 16

#define MBTC_K_COLORMODE_STATIC 0
#define MBTC_K_COLORMODE_ALARM 1

#define MBTC_MAJOR_VERSION 4
#define MBTC_MINOR_VERSION 0
#define MBTC_RELEASE 0

#define MBTC_K_LITERAL 1
#define MBTC_K_PV_STATE 2

#ifdef __menu_button_cc

#include "menu_button.str"

static void doBlink (
  void *ptr
);

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static char *dragName[] = {
  activeMenuButtonClass_str1,
  activeMenuButtonClass_str28,
  activeMenuButtonClass_str34,
  activeMenuButtonClass_str30
};

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
  ProcessVariable *pv,
  void *userarg );

static void mbt_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void mbt_readUpdate (
  ProcessVariable *pv,
  void *userarg );

static void mbt_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void mbt_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void mbt_visUpdate (
  ProcessVariable *pv,
  void *userarg );

static void mbt_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void mbt_colorUpdate (
  ProcessVariable *pv,
  void *userarg );

#endif

class activeMenuButtonClass : public activeGraphicClass {

private:

friend void doBlink (
  void *ptr
);

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void menu_cb (
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
  ProcessVariable *pv,
  void *userarg );

friend void mbt_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void mbt_readUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void mbt_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void mbt_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void mbt_visUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void mbt_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void mbt_colorUpdate (
  ProcessVariable *pv,
  void *userarg );

static const int controlPvConnection = 1;
static const int readPvConnection = 2;
static const int visPvConnection = 3;
static const int colorPvConnection = 4;

pvConnectionClass connection;

int init, opComplete, pvCheckExists, controlValid, readValid;

typedef struct editBufTag {
// edit buffer
  char bufControlPvName[PV_Factory::MAX_PV_NAME+1];
  char bufReadPvName[PV_Factory::MAX_PV_NAME+1];
  char bufVisPvName[PV_Factory::MAX_PV_NAME+1];
  char bufColorPvName[PV_Factory::MAX_PV_NAME+1];
} editBufType, *editBufPtr;

editBufPtr eBuf;

entryListBase *invisPvEntry, *visInvEntry, *minVisEntry, *maxVisEntry;

int bufX, bufY, bufW, bufH;

short curValue, value, curReadValue, readValue;

int topShadowColor, bufTopShadowColor;
int botShadowColor, bufBotShadowColor;
int bufFgColor, bufBgColor, bufInconsistentColor;
pvColorClass fgColor, bgColor, inconsistentColor;
colorButtonClass fgCb, bgCb, inconsistentCb, topShadowCb, botShadowCb;
int fgColorMode, bgColorMode, bufFgColorMode, bufBgColorMode;

fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
XmFontList fontList;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

int buttonPressed;

int usePvId;
ProcessVariable *controlPvId, *readPvId, *stateStringPvId;

expStringClass controlPvExpStr;

expStringClass readPvExpStr;

int controlExists, readExists, widgetsCreated, active, activeMode, enumCount;

Widget popUpMenu, pullDownMenu, pb[MAX_ENUM_STATES];

int needConnectInit, needReadConnectInit, needInfoInit,
 needReadInfoInit, needDraw, needRefresh, needToDrawUnconnected,
 needToEraseUnconnected;
XtIntervalId unconnectedTimer;
int initialConnection, initialReadConnection, initialVisConnection,
 initialColorConnection;

ProcessVariable *visPvId;
expStringClass visPvExpStr;
int visExists;
double visValue, curVisValue, minVis, maxVis;
char minVisString[39+1], bufMinVisString[39+1];
char maxVisString[39+1], bufMaxVisString[39+1];
int prevVisibility, visibility, visInverted, bufVisInverted;
int needVisConnectInit, needVisInit, needVisUpdate;

ProcessVariable *colorPvId;
expStringClass colorPvExpStr;
int colorExists;
double colorValue, curColorValue;
int needColorConnectInit, needColorInit, needColorUpdate;

int oldStat, oldSev;

public:

static const int useReadPvId = 1;
static const int useControlPvId = 2;

activeMenuButtonClass ( void );

~activeMenuButtonClass ( void );

activeMenuButtonClass
 ( const activeMenuButtonClass *source );

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

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int draw ( void );

int erase ( void );

int drawActive ( void );

int eraseActive ( void );

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

int activate ( int pass, void *ptr );

int deactivate ( int pass );

void updateDimensions ( void );

void btnUp (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action );

void btnDown (
  XButtonEvent *be,
  int _x,
  int _y,
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

void *create_activeMenuButtonClassPtr ( void );
void *clone_activeMenuButtonClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
