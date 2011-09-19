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

#include "pv_factory.h"
#include "cvtFast.h"

#define MMUX_MAX_ENTRIES 8
#define MMUX_MAX_STATES 16
#define MMUX_MAX_STRING_SIZE 32

#define MMUXC_K_COLORMODE_STATIC 0
#define MMUXC_K_COLORMODE_ALARM 1

#define MMUXC_MAJOR_VERSION 4
#define MMUXC_MINOR_VERSION 1
#define MMUXC_RELEASE 0

#define MMUXC_K_LITERAL 1
#define MMUXC_K_PV_STATE 2

#ifdef __menu_mux_cc

#include "menu_mux.str"

static char *dragName[] = {
  menuMuxClass_str1
};

static void doBlink (
  void *ptr
);

static void retryTimeout (
  XtPointer client,
  XtIntervalId *id );

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

#if 0
static void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );
#endif

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
  ProcessVariable *pv,
  void *userarg );

static void mmux_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void mmuxSetItem (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class menuMuxClass : public activeGraphicClass {

private:

friend void doBlink (
  void *ptr
);

friend void retryTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

#if 0
friend void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );
#endif

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
  ProcessVariable *pv,
  void *userarg );

friend void mmux_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void mmuxSetItem (
  Widget w,
  XtPointer client,
  XtPointer call );

int opComplete, firstEvent;

typedef struct editBufTag {
// edit buffer
  int bufX;
  int bufY;
  int bufW;
  int bufH;
  int bufTopShadowColor;
  int bufBotShadowColor;
  int bufFgColor;
  int bufBgColor;
  int bufFgColorMode;
  int bufBgColorMode;
  char bufTag[MMUX_MAX_STATES][MMUX_MAX_STRING_SIZE+1];
  char bufM[MMUX_MAX_STATES][MMUX_MAX_ENTRIES][MMUX_MAX_STRING_SIZE+1];
  char bufE[MMUX_MAX_STATES][MMUX_MAX_ENTRIES][MMUX_MAX_STRING_SIZE+1];
  char bufInitialState[30+1];
  char bufControlPvName[PV_Factory::MAX_PV_NAME+1];
  colorButtonClass fgCb;
  colorButtonClass bgCb;
  colorButtonClass topShadowCb;
  colorButtonClass botShadowCb;
} editBufType, *editBufPtr;

editBufPtr eBuf;

entryListBase *pvNameEntry, *iniStateEntry;

int controlV, curControlV;

int topShadowColor;
int botShadowColor;
pvColorClass fgColor, bgColor;
int fgColorMode, bgColorMode;

char *stateString[MMUX_MAX_STATES]; // allocated at run-time
int numStates;

entryListBase *elbt, *elbm[MMUX_MAX_ENTRIES], *elbe[MMUX_MAX_ENTRIES];

char tag[MMUX_MAX_STATES][MMUX_MAX_STRING_SIZE+1],
 m[MMUX_MAX_STATES][MMUX_MAX_ENTRIES][MMUX_MAX_STRING_SIZE+1],
 e[MMUX_MAX_STATES][MMUX_MAX_ENTRIES][MMUX_MAX_STRING_SIZE+1];

char *tagPtr[MMUX_MAX_STATES], *mPtr[MMUX_MAX_ENTRIES][MMUX_MAX_STATES],
 *ePtr[MMUX_MAX_ENTRIES][MMUX_MAX_STATES];

char **mac, **exp;
int numItems, numMac;

fontMenuClass fm;
char fontTag[63+1];
XmFontList fontList;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

ProcessVariable *controlPvId;

expStringClass controlPvExpStr;

expStringClass initialStateExpStr;

int controlExists, widgetsCreated, controlPvConnected, active, activeMode;

#if 0
Widget optionMenu, pulldownMenu, curHistoryWidget,
 pb[MMUX_MAX_STATES];
#endif

Widget popUpMenu, pullDownMenu, pb[MAX_ENUM_STATES];

int needConnectInit, needDisconnect, needInfoInit, needUpdate, needDraw,
 needToDrawUnconnected, needToEraseUnconnected;
XtIntervalId unconnectedTimer;
int initialConnection;

XtIntervalId retryTimer;

int buttonPressed;

int oldStat, oldSev;

public:

menuMuxClass ( void );

menuMuxClass
 ( const menuMuxClass *source );

~menuMuxClass ( void );

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

int getNumMacroSets ( void );

int getMacrosSet (
  int *numMacros,
  char ***macro,
  char ***expansion,
  int n
);

int getMacros (
  int *numMacros,
  char ***macro,
  char ***expansion );

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

int isMux ( void ) { return 1; }

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

void *create_menuMuxClassPtr ( void );
void *clone_menuMuxClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
