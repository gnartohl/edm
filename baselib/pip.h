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

#ifndef __pip_h
#define __pip_h 1

#include "act_grf.h"
#include "entry_form.h"

#include "pv_factory.h"
#include "cvtFast.h"

#define PIPC_MAJOR_VERSION 4
#define PIPC_MINOR_VERSION 1
#define PIPC_RELEASE 0

#ifdef __pip_cc

#include "pip.str"

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void needUpdateTimeout (
  XtPointer client,
  XtIntervalId *id );

static void needMenuUpdateTimeout (
  XtPointer client,
  XtIntervalId *id );

static void needUnmapTimeout (
  XtPointer client,
  XtIntervalId *id );

static void needMapTimeout (
  XtPointer client,
  XtIntervalId *id );

static void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pipc_edit_ok1 (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pipc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pipc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pipc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pipc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pipc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pip_readUpdate (
  ProcessVariable *pv,
  void *userarg );

static void pip_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void pip_monitor_label_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void pip_menuUpdate (
  ProcessVariable *pv,
  void *userarg );

static void pip_monitor_menu_connect_state (
  ProcessVariable *pv,
  void *userarg );

#endif

class activePipClass : public activeGraphicClass {

public:

static const int maxDsps = 100;
static const int displayFromPV = 0;
static const int displayFromForm = 1;
static const int displayFromMenu = 2;
static const int maxSymbolLen = 2550;

private:

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void needUpdateTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void needMenuUpdateTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void needUnmapTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void needMapTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pipc_edit_ok1 (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pipc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pipc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pipc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pipc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pipc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pip_readUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void pip_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void pip_monitor_label_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void pip_menuUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void pip_monitor_menu_connect_state (
  ProcessVariable *pv,
  void *userarg );

typedef struct bufTag {
  int bufX;
  int bufY;
  int bufW;
  int bufH;
  int bufFgColor;
  int bufBgColor;
  int bufTopShadowColor;
  int bufBotShadowColor;
  int bufDisplaySource; // string pv, filename, or menu
  char bufReadPvName[PV_Factory::MAX_PV_NAME+1];
  char bufLabelPvName[PV_Factory::MAX_PV_NAME+1];
  char bufFileName[127+1];
  int bufPropagateMacros[maxDsps];
  char bufDisplayFileName[maxDsps][127+1];
  char bufSymbols[maxDsps][maxSymbolLen+1];
  int bufReplaceSymbols[maxDsps];
  char bufLabel[maxDsps][127+1];
  int bufCenter;
  int bufSetSize;
  int bufSizeOfs;
  int bufNoScroll;
  int bufIgnoreMultiplexors;
} bufType, *bufPtr;

entryListBase *disSrcEntry, *pvNameEntry, *labelPvNameEntry, *fileNameEntry,
 *menuBtnEntry;

entryListBase *setSizeEntry, *sizeOfsEntry;

int numDsps, dspIndex;

bufPtr buf;

int opComplete;

int minW;
int minH;
int center;
int setSize;
int sizeOfs;
int noScroll;
int ignoreMultiplexors;

Widget *frameWidget, clipWidget, hsbWidget, vsbWidget, popUpMenu,
 pullDownMenu, pb[maxDsps];

int propagateMacros[maxDsps];
expStringClass displayFileName[maxDsps];
expStringClass symbolsExpStr[maxDsps];
char symbols[maxDsps][maxSymbolLen+1];
int replaceSymbols[maxDsps]; // else append
expStringClass label[maxDsps];

int curReadIV;
char readV[39+1], curReadV[39+1];
char curFileName[127+1];
int bufInvalid;

int firstEvent;

entryFormClass *ef1;

ProcessVariable *readPvId, *labelPvId;

int displaySource;

expStringClass readPvExpStr, labelPvExpStr, fileNameExpStr;
char bufReadPvName[PV_Factory::MAX_PV_NAME+1];
char bufLabelPvName[PV_Factory::MAX_PV_NAME+1];
char bufFileName[127+1];

int readExists, labelExists, fileExists;

int readPvConnected, init, active, activeMode;

pvColorClass fgColor, bgColor, topShadowColor, botShadowColor;
colorButtonClass fgCb, bgCb, topCb, botCb;
int bufFgColor, bufBgColor, bufTopShadowColor, bufBotShadowColor;

int needConnectInit, needUpdate, needMenuConnectInit, needMenuUpdate,
 needDraw, needFileOpen, needInitMenuFileOpen, needMap, needUnmap,
 needToDrawUnconnected, needToEraseUnconnected, needConnectTimeout;
int initialReadConnection, initialMenuConnection, initialLabelConnection;
XtIntervalId unconnectedTimer;
int consecutiveDeactivateErrors;

XtIntervalId retryTimerNU, retryTimerNMU, retryTimerNUM, retryTimerNM;

activeWindowClass *aw;

int activateIsComplete;

public:

activePipClass ( void );

activePipClass
 ( const activePipClass *source );

~activePipClass ( void );

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

int reactivate (
  int pass,
  void *ptr );

int reactivate (
  int pass,
  void *ptr,
  int *numSubObjects );

int activate ( int pass, void *ptr );

int preReactivate ( int pass );

int preReactivate (
  int pass,
  int *numSubObjects );

int deactivate ( int pass );

int isRelatedDisplay ( void );

void augmentRelatedDisplayMacros (
  char *buf
);

int getNumRelatedDisplays ( void );

int getRelatedDisplayProperty (
  int index,
  char *key
);

char *getRelatedDisplayName (
  int index
);

char *getRelatedDisplayMacros (
  int index
);

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

int createPipWidgets ( void );

int checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

void openEmbeddedByIndex (
  int index );

void executeDeferred ( void );

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

int isWindowContainer ( void );

int activateComplete ( void );

int activateBeforePreReexecuteComplete ( void );

void map ( void );

void unmap ( void );

char *getSearchString (
  int i
);

void replaceString (
  int i,
  int max,
  char *string
);

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activePipClassPtr ( void );
void *clone_activePipClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
